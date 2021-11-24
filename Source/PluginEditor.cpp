/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginEditor.h"

#include "PluginProcessor.h"

//==============================================================================
Ap_dynamicsAudioProcessorEditor::Ap_dynamicsAudioProcessorEditor(Ap_dynamicsAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p), stylePicker_(p)
{
  bgText_ = std::make_unique<juce::Image>(
      juce::ImageCache::getFromMemory(BinaryData::logo_clean_png, BinaryData::logo_clean_pngSize));
  textShadow_ =
      std::make_unique<juce::Image>(juce::ImageCache::getFromMemory(BinaryData::shadow_png, BinaryData::shadow_pngSize));
  myFont_ = std::make_unique<juce::Font>(
      juce::Typeface::createSystemTypefaceFor(BinaryData::VarelaRound_ttf, BinaryData::VarelaRound_ttfSize));

  // Slider Setup
  setupSlider(thresholdSlider_, thresholdLabel_, "Threshold", SliderType::Invert, "dB");
  thresholdAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "THR",
                                                                                                thresholdSlider_->slider);
  setupSlider(ratioSlider_, ratioLabel_, "Ratio", SliderType::Normal, ": 1");
  ratioAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "RAT",
                                                                                            ratioSlider_->slider);

  styleLabel_ = std::make_unique<juce::Label>("", "style");
  styleLabel_->setJustificationType(juce::Justification::centred);
  styleLabel_->setText("style", juce::dontSendNotification);
  styleLabel_->setFont(myFont_->withHeight(APConstants::Gui::FONT_HEIGHT));
  styleLabel_->setBorderSize(juce::BorderSize<int>(10, 50, 30, 0));
  styleLabel_->setColour(juce::Label::textColourId, APConstants::Colors::DarkGrey);
  styleLabel_->attachToComponent(&stylePicker_, false);

  addAndMakeVisible(stylePicker_);

  thresholdBounds_ = juce::Rectangle<int>(40, APConstants::Gui::SLIDER_Y, APConstants::Gui::SLIDER_WIDTH, sliderHeight_);
  ratioBounds_     = juce::Rectangle<int>(280, APConstants::Gui::SLIDER_Y, APConstants::Gui::SLIDER_WIDTH, sliderHeight_);
  pickerBounds_    = juce::Rectangle<int>(470, APConstants::Gui::SLIDER_Y, APConstants::Gui::SLIDER_WIDTH, sliderHeight_);

  setSize(APConstants::Gui::M_WIDTH, APConstants::Gui::M_HEIGHT);
  setResizable(false, false);
  startTimerHz(60);

  kernel_ = std::make_unique<juce::ImageConvolutionKernel>(16);
  kernel_->createGaussianBlur(5.6f);
  kernel_->applyToImage(*textShadow_, *textShadow_, textShadow_->getBounds());

  kernel_->clear();
  kernel_->createGaussianBlur(3.2f);

  // Shadow Setup
  thresholdShadow_ = std::make_unique<juce::Image>();
  ratioShadow_     = std::make_unique<juce::Image>();
  styleShadow_     = std::make_unique<juce::Image>();
  tSliderShadow_   = std::make_unique<juce::Image>();
  rSliderShadow_   = std::make_unique<juce::Image>();
  sSliderShadow_   = std::make_unique<juce::Image>();
  setupLabelShadow(*thresholdShadow_, thresholdLabel_->getText());
  setupLabelShadow(*ratioShadow_, ratioLabel_->getText());
  setupLabelShadow(*styleShadow_, styleLabel_->getText());
  setupSliderShadow(*tSliderShadow_);
  setupSliderShadow(*rSliderShadow_);
  setupSliderShadow(*sSliderShadow_);

//  sliderBarGl.setBounds(thresholdBounds_);
//  addAndMakeVisible(sliderBarGl);
}

Ap_dynamicsAudioProcessorEditor::~Ap_dynamicsAudioProcessorEditor() { stopTimer(); }

//==============================================================================
void Ap_dynamicsAudioProcessorEditor::paint(juce::Graphics& g)
{
  g.setGradientFill(juce::ColourGradient(APConstants::Colors::INNER_GRADIENT_BG, getWidth() * 0.5f, getHeight() * 0.93f,
                                         APConstants::Colors::OUTER_GRADIENT_BG, -80, -80, true));
  g.fillAll();

  // Logo
  constexpr int textDeltaX = 80;
  constexpr int textDeltaY = 50;
  const int textHeight     = static_cast<const int>(static_cast<float>(getHeight()) * 0.45f);
  const auto textBounds    = getLocalBounds()
                              .removeFromTop(textHeight)
                              .reduced(textDeltaX, textDeltaY)
                              .withBottomY(APConstants::Gui::SLIDER_Y - 100)
                              .toFloat();

  // Draw Logo Shadow
  g.drawImage(*textShadow_, textBounds.withY(textBounds.getY() - 10).expanded(shadowDeltaXY_ * 1.5f) + offset_,
              juce::RectanglePlacement::fillDestination);

  g.drawImage(*bgText_, textBounds, juce::RectanglePlacement::fillDestination);

  // Draw Label Shadows
  auto shadowHeight = APConstants::Gui::SLIDER_Y - 68;
  g.drawImage(*thresholdShadow_,
              juce::Rectangle<float>(thresholdSlider_->getX() + 6, shadowHeight, thresholdShadow_->getWidth(),
                                     thresholdShadow_->getHeight()),
              juce::RectanglePlacement::fillDestination);
  g.drawImage(
      *ratioShadow_,
      juce::Rectangle<float>(ratioSlider_->getX() + 6, shadowHeight, ratioShadow_->getWidth(), ratioShadow_->getHeight()),
      juce::RectanglePlacement::fillDestination);
  g.drawImage(
      *styleShadow_,
      juce::Rectangle<float>(stylePicker_.getX() + 66, shadowHeight, styleShadow_->getWidth(), styleShadow_->getHeight()),
      juce::RectanglePlacement::fillDestination);

  auto shadowBounds = juce::Rectangle<float>(40, APConstants::Gui::SLIDER_Y, 130, sliderHeight_);

  g.drawImage(*tSliderShadow_, shadowBounds.expanded(shadowDeltaXY_) + offset_, juce::RectanglePlacement::fillDestination);
  g.drawImage(*rSliderShadow_, shadowBounds.withX(280).expanded(shadowDeltaXY_) + offset_,
              juce::RectanglePlacement::fillDestination);
  g.drawImage(*sSliderShadow_, shadowBounds.withX(530).expanded(shadowDeltaXY_) + offset_,
              juce::RectanglePlacement::fillDestination);

  // Version No.
  juce::String version = JUCE_APPLICATION_VERSION_STRING;
  g.setColour(juce::Colours::darkgrey);
  g.drawFittedText("Version #: " + version, getWidth() - 105, getHeight() - 20, 100, 15, juce::Justification::centred, 1);
}

void Ap_dynamicsAudioProcessorEditor::resized()
{
  thresholdSlider_->setBounds(thresholdBounds_);

  ratioSlider_->setBounds(ratioBounds_);

  stylePicker_.setBounds(pickerBounds_);
}

void Ap_dynamicsAudioProcessorEditor::setupLabelShadow(juce::Image& shadow, const juce::String& name)
{
  shadow = juce::Image(juce::Image::PixelFormat::ARGB, 120, (int)APConstants::Gui::SHADOW_FONT_HEIGHT, true);
  juce::Graphics graphics(shadow);
  graphics.setColour(APConstants::Colors::SHADOW_COLOR);
  graphics.setFont(myFont_->withHeight(APConstants::Gui::SHADOW_FONT_HEIGHT));
  graphics.drawText(name, 0, 0, shadow.getWidth(), shadow.getHeight(), juce::Justification::centred, false);
  kernel_->applyToImage(shadow, shadow, shadow.getBounds());
}

void Ap_dynamicsAudioProcessorEditor::setupSliderShadow(juce::Image& shadow)
{
  auto cornerSize = 10.0f;
  auto width      = 130;

  shadow = juce::Image(juce::Image::PixelFormat::ARGB, width, sliderHeight_, true);
  juce::Graphics graphics(shadow);
  graphics.setColour(APConstants::Colors::SHADOW_COLOR);
  graphics.fillRoundedRectangle(juce::Rectangle<float>(0, 0, shadow.getWidth() - 8, shadow.getHeight() - 8)
                                    .withCentre(shadow.getBounds().getCentre().toFloat()),
                                cornerSize);
  kernel_->applyToImage(shadow, shadow, shadow.getBounds());
}

void Ap_dynamicsAudioProcessorEditor::setupSlider(std::unique_ptr<APSlider>& apSlider, std::unique_ptr<juce::Label>& label,
                                                  const juce::String& name, const SliderType sliderType,
                                                  const juce::String& suffix)
{
  apSlider = std::make_unique<APSlider>(audioProcessor, sliderType);
  apSlider->slider.setTextBoxIsEditable(false);
  apSlider->slider.setSliderStyle(juce::Slider::LinearBarVertical);
  apSlider->slider.setTextValueSuffix(" " + suffix);
  apSlider->slider.setColour(juce::Slider::trackColourId, juce::Colour(0xFFFFD479));
  apSlider->slider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::snow);

  label = std::make_unique<juce::Label>("", name);
  label->setJustificationType(juce::Justification::centred);
  label->setText(name.toLowerCase(), juce::dontSendNotification);
  label->setBorderSize(juce::BorderSize<int>(10, 0, 30, 70));
  label->setColour(juce::Label::textColourId, APConstants::Colors::DarkGrey);
  label->setFont(myFont_->withHeight(APConstants::Gui::FONT_HEIGHT));
  label->attachToComponent(apSlider.get(), false);

  addAndMakeVisible(apSlider.get());
}

void Ap_dynamicsAudioProcessorEditor::timerCallback() { repaint(); }
