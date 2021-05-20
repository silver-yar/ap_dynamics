/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginEditor.h"

#include "PluginProcessor.h"

//==============================================================================
Ap_dynamicsAudioProcessorEditor::Ap_dynamicsAudioProcessorEditor(Ap_dynamicsAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p), thresholdLook_(p, Invert), ratioLook_(p, Normal), stylePicker_(p)
{
  setupSlider(thresholdSlider_, thresholdLabel_, lshdwT_, "Threshold", SliderType::Invert, "dB");
  thresholdSlider_->slider.setLookAndFeel(&thresholdLook_);
  thresholdAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "THR",
                                                                                                thresholdSlider_->slider);

  setupSlider(ratioSlider_, ratioLabel_, lshdwR_, "Ratio", SliderType::Normal, ": 1");
  ratioSlider_->slider.setLookAndFeel(&ratioLook_);
  ratioAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "RAT",
                                                                                            ratioSlider_->slider);

  styleLabel_ = std::make_unique<juce::Label>("", "style");
  styleLabel_->setJustificationType(juce::Justification::centred);
  styleLabel_->setText("style", juce::dontSendNotification);
  styleLabel_->setFont(myFont_.withHeight(FONT_HEIGHT));
  styleLabel_->setBorderSize(juce::BorderSize<int>(10, 50, 30, 0));
  styleLabel_->setColour(juce::Label::textColourId, AP::Colors::DarkGrey);
  styleLabel_->attachToComponent(&stylePicker_, false);

  addAndMakeVisible(stylePicker_);

  thresholdBounds_ = juce::Rectangle<int>(40, SLIDER_Y, SLIDER_WIDTH, sliderHeight_);
  ratioBounds_     = juce::Rectangle<int>(280, SLIDER_Y, SLIDER_WIDTH, sliderHeight_);
  pickerBounds_    = juce::Rectangle<int>(470, SLIDER_Y, SLIDER_WIDTH, sliderHeight_);

  setSize(M_WIDTH, M_HEIGHT);
  startTimerHz(60);

  kernel_.createGaussianBlur(5.6f);
  kernel_.applyToImage(textShadow_, textShadow_, textShadow_.getBounds());

  kernel_.clear();
  kernel_.createGaussianBlur(3.2f);
  setupLabelShadow(thresholdShadow_, thresholdLabel_->getText());
  setupLabelShadow(ratioShadow_, ratioLabel_->getText());
  setupLabelShadow(styleShadow_, styleLabel_->getText());
  setupSliderShadow(tSliderShadow_);
  setupSliderShadow(rSliderShadow_);
  setupSliderShadow(sSliderShadow_);
}

Ap_dynamicsAudioProcessorEditor::~Ap_dynamicsAudioProcessorEditor() { stopTimer(); }

//==============================================================================
void Ap_dynamicsAudioProcessorEditor::paint(juce::Graphics& g)
{
  g.setGradientFill(
      juce::ColourGradient(INNER_GRADIENT_BG, getWidth() * 0.5f, getHeight() * 0.93f, OUTER_GRADIENT_BG, -80, -80, true));
  g.fillAll();

  // Logo
  constexpr int textDeltaX = 80;
  constexpr int textDeltaY = 50;
  const int textHeight     = getHeight() * 0.45f;
  const auto textBounds =
      getLocalBounds().removeFromTop(textHeight).reduced(textDeltaX, textDeltaY).withBottomY(SLIDER_Y - 100).toFloat();

  // Draw Logo Shadow
  g.drawImage(textShadow_,
              textBounds.withY(textBounds.getY() - 10).expanded(testBox_.shadowDeltaXY * 1.5f) + testBox_.offset,
              juce::RectanglePlacement::fillDestination);

  g.drawImage(bgText_, textBounds, juce::RectanglePlacement::fillDestination);

  // Draw Label Shadows
  auto shadowHeight = SLIDER_Y - 68;
  g.drawImage(thresholdShadow_,
              juce::Rectangle<float>(thresholdSlider_->getX() + 6, shadowHeight, thresholdShadow_.getWidth(),
                                     thresholdShadow_.getHeight()),
              juce::RectanglePlacement::fillDestination);
  g.drawImage(
      ratioShadow_,
      juce::Rectangle<float>(ratioSlider_->getX() + 6, shadowHeight, ratioShadow_.getWidth(), ratioShadow_.getHeight()),
      juce::RectanglePlacement::fillDestination);
  g.drawImage(
      styleShadow_,
      juce::Rectangle<float>(stylePicker_.getX() + 66, shadowHeight, styleShadow_.getWidth(), styleShadow_.getHeight()),
      juce::RectanglePlacement::fillDestination);

  // Slider Shadows
  //    g.setColour (SHADOW_COLOR);

  auto shadowBounds = juce::Rectangle<float>(40, SLIDER_Y, 130, sliderHeight_);

  g.drawImage(tSliderShadow_, shadowBounds.expanded(shadowDeltaXY_) + offset_, juce::RectanglePlacement::fillDestination);
  g.drawImage(rSliderShadow_, shadowBounds.withX(280).expanded(shadowDeltaXY_) + offset_,
              juce::RectanglePlacement::fillDestination);
  g.drawImage(sSliderShadow_, shadowBounds.withX(530).expanded(shadowDeltaXY_) + offset_,
              juce::RectanglePlacement::fillDestination);

  //    g.fillRoundedRectangle(shadowBounds.expanded(testBox_.shadowDeltaXY) +
  //                                                        testBox_.offset, 10.0f);
  //    g.fillRoundedRectangle(shadowBounds.withX(280).expanded(testBox_.shadowDeltaXY) +
  //                                                        testBox_.offset, 10.0f);
  //    g.fillRoundedRectangle(shadowBounds.withX(530).expanded(testBox_.shadowDeltaXY) +
  //                                                        testBox_.offset, 10.0f);
}

void Ap_dynamicsAudioProcessorEditor::resized()
{
  thresholdSlider_->setBounds(thresholdBounds_);

  ratioSlider_->setBounds(ratioBounds_);

  stylePicker_.setBounds(pickerBounds_);
}

void Ap_dynamicsAudioProcessorEditor::setupLabelShadow(juce::Image& shadow, const juce::String& name)
{
  shadow = juce::Image(juce::Image::PixelFormat::ARGB, 120, (int)SHADOW_FONT_HEIGHT, true);
  juce::Graphics graphics(shadow);
  graphics.setColour(SHADOW_COLOR);
  graphics.setFont(myFont_.withHeight(SHADOW_FONT_HEIGHT));
  graphics.drawText(name, 0, 0, shadow.getWidth(), shadow.getHeight(), juce::Justification::centred, false);
  kernel_.applyToImage(shadow, shadow, shadow.getBounds());
}

void Ap_dynamicsAudioProcessorEditor::setupSliderShadow(juce::Image& shadow)
{
  auto cornerSize = 10.0f;
  auto width      = 130;

  shadow = juce::Image(juce::Image::PixelFormat::ARGB, width, sliderHeight_, true);
  juce::Graphics graphics(shadow);
  graphics.setColour(SHADOW_COLOR);
  graphics.fillRoundedRectangle(juce::Rectangle<float>(0, 0, shadow.getWidth() - 8, shadow.getHeight() - 8)
                                    .withCentre(shadow.getBounds().getCentre().toFloat()),
                                cornerSize);
  kernel_.applyToImage(shadow, shadow, shadow.getBounds());
}

void Ap_dynamicsAudioProcessorEditor::setupSlider(std::unique_ptr<CustomSlider_>& slider,
                                                  std::unique_ptr<juce::Label>& label,
                                                  std::unique_ptr<juce::Label>& labelShadow, const juce::String& name,
                                                  SliderType sliderType, const juce::String& suffix)
{
  slider = std::make_unique<CustomSlider_>(audioProcessor, sliderType);
  slider->slider.setTextBoxIsEditable(false);
  slider->slider.setSliderStyle(juce::Slider::LinearBarVertical);
  slider->slider.setTextValueSuffix(" " + suffix);
  slider->slider.setColour(juce::Slider::trackColourId, juce::Colour(0xFFFFD479));
  slider->slider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::snow);

  label = std::make_unique<juce::Label>("", name);
  label->setJustificationType(juce::Justification::centred);
  label->setText(name.toLowerCase(), juce::dontSendNotification);
  label->setBorderSize(juce::BorderSize<int>(10, 0, 30, 70));
  label->setColour(juce::Label::textColourId, AP::Colors::DarkGrey);
  label->setFont(myFont_.withHeight(FONT_HEIGHT));
  label->attachToComponent(slider.get(), false);

  addAndMakeVisible(slider.get());
}

void Ap_dynamicsAudioProcessorEditor::timerCallback() { repaint(); }
