/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginEditor.h"

#include "PluginProcessor.h"

//==============================================================================
Ap_dynamicsAudioProcessorEditor::Ap_dynamicsAudioProcessorEditor(Ap_dynamicsAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor_(p), stylePicker_(p)
{
  initializeAssets();
  // Slider Setup
  setupSlider(thresholdSlider_, thresholdLabel_, "Threshold", SliderType::Invert, "dB");
  thresholdAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor_.apvts, "THR",
                                                                                                thresholdSlider_->slider);
  setupSlider(ratioSlider_, ratioLabel_, "Ratio", SliderType::Normal, ": 1");
  ratioAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor_.apvts, "RAT",
                                                                                            ratioSlider_->slider);
  auto thresh_look_and_feel = dynamic_cast<MyLookAndFeel*>(&thresholdSlider_->slider.getLookAndFeel());
  jassert(thresh_look_and_feel != nullptr);
  thresh_look_and_feel->getLabelText = [this]()
  {
    auto slider_value        = thresholdSlider_->slider.getValue();
    auto scaled_slider_value = std::round(slider_value * 100.0f);
    slider_value             = scaled_slider_value / 100.0f;
    auto label_text = (static_cast<int>(scaled_slider_value) % 100 != 0)
                          ? juce::String(slider_value) + thresholdSlider_->slider.getTextValueSuffix()
                          : juce::String(slider_value) + ".0" + thresholdSlider_->slider.getTextValueSuffix();
    return label_text;
  };
  
  auto ratio_look_and_feel = dynamic_cast<MyLookAndFeel*>(&ratioSlider_->slider.getLookAndFeel());
  jassert(ratio_look_and_feel != nullptr);
  ratio_look_and_feel->getLabelText = [this]()
  {
    auto slider_value        = ratioSlider_->slider.getValue();
    auto scaled_slider_value = std::round(slider_value * 100.0f);
    slider_value             = scaled_slider_value / 100.0f;
    auto label_text = (static_cast<int>(scaled_slider_value) % 100 != 0)
                                   ? juce::String(slider_value) + ratioSlider_->slider.getTextValueSuffix()
                                   : juce::String(slider_value) + ".0" + ratioSlider_->slider.getTextValueSuffix();
    return label_text;
  };
  
  styleLabel_->setJustificationType(juce::Justification::centred);
  styleLabel_->setText("style", juce::dontSendNotification);
  styleLabel_->setFont(APConstants::Gui::SYS_FONT.withHeight(APConstants::Gui::FONT_HEIGHT));
  styleLabel_->setBorderSize(juce::BorderSize<int>(10, 50, 30, 0));
  styleLabel_->setColour(juce::Label::textColourId, APConstants::Colors::DARK_GREY);
  styleLabel_->attachToComponent(&stylePicker_, false);
  addAndMakeVisible(stylePicker_);

  // Shadow Setup
  kernel_->createGaussianBlur(APConstants::Gui::BLUR_RADIUS_LOGO);
  kernel_->applyToImage(*textShadow_, *textShadow_, textShadow_->getBounds());
  kernel_->clear();
  kernel_->createGaussianBlur(APConstants::Gui::BLUR_RADIUS_LABEL);
  setupLabelShadow(*thresholdShadow_, thresholdLabel_->getText());
  setupLabelShadow(*ratioShadow_, ratioLabel_->getText());
  setupLabelShadow(*styleShadow_, styleLabel_->getText());
  setupSliderShadow(*tSliderShadow_);
  setupSliderShadow(*rSliderShadow_);
  setupSliderShadow(*sSliderShadow_);

  thresholdBounds_ = juce::Rectangle<int>(40, APConstants::Gui::SLIDER_Y, APConstants::Gui::SLIDER_WIDTH, sliderHeight_);
  ratioBounds_     = juce::Rectangle<int>(280, APConstants::Gui::SLIDER_Y, APConstants::Gui::SLIDER_WIDTH, sliderHeight_);
  pickerBounds_    = juce::Rectangle<int>(470, APConstants::Gui::SLIDER_Y, APConstants::Gui::SLIDER_WIDTH, sliderHeight_);

  setSize(APConstants::Gui::M_WIDTH, APConstants::Gui::M_HEIGHT);
  setResizable(false, false);
  startTimerHz(60);
}

Ap_dynamicsAudioProcessorEditor::~Ap_dynamicsAudioProcessorEditor() { stopTimer(); }

//==============================================================================
void Ap_dynamicsAudioProcessorEditor::paint(juce::Graphics& g)
{
  g.setGradientFill(juce::ColourGradient(APConstants::Colors::INNER_GRADIENT_BG, static_cast<float>(getWidth()) * 0.5f,
                                         static_cast<float>(getHeight()) * 0.93f, APConstants::Colors::OUTER_GRADIENT_BG,
                                         -80, -80, true));
  g.fillAll();

  // Logo
  constexpr int textDeltaX = 80;
  constexpr int textDeltaY = 50;
  const int textHeight     = static_cast<int>(static_cast<float>(getHeight()) * 0.45f);
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
  constexpr auto shadowHeight = static_cast<float>(APConstants::Gui::SLIDER_Y - 68);
  g.drawImage(*thresholdShadow_,
              juce::Rectangle<float>(static_cast<float>(thresholdSlider_->getX() + 6), shadowHeight,
                                     static_cast<float>(thresholdShadow_->getWidth()),
                                     static_cast<float>(thresholdShadow_->getHeight())),
              juce::RectanglePlacement::fillDestination);
  g.drawImage(
      *ratioShadow_,
      juce::Rectangle<float>(static_cast<float>(ratioSlider_->getX() + 6), shadowHeight,
                             static_cast<float>(ratioShadow_->getWidth()), static_cast<float>(ratioShadow_->getHeight())),
      juce::RectanglePlacement::fillDestination);
  g.drawImage(
      *styleShadow_,
      juce::Rectangle<float>(static_cast<float>(stylePicker_.getX() + 66), shadowHeight,
                             static_cast<float>(styleShadow_->getWidth()), static_cast<float>(styleShadow_->getHeight())),
      juce::RectanglePlacement::fillDestination);

  const auto shadowBounds = juce::Rectangle<float>(40, APConstants::Gui::SLIDER_Y, 130, static_cast<float>(sliderHeight_));

  g.drawImage(*tSliderShadow_, shadowBounds.expanded(shadowDeltaXY_) + offset_, juce::RectanglePlacement::fillDestination);
  g.drawImage(*rSliderShadow_, shadowBounds.withX(280).expanded(shadowDeltaXY_) + offset_,
              juce::RectanglePlacement::fillDestination);
  g.drawImage(*sSliderShadow_, shadowBounds.withX(530).expanded(shadowDeltaXY_) + offset_,
              juce::RectanglePlacement::fillDestination);

  // Version No.
  const juce::String version = JUCE_APPLICATION_VERSION_STRING;
  g.setColour(juce::Colours::darkgrey);
  g.setFont(APConstants::Gui::SYS_FONT);
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
  graphics.setFont(APConstants::Gui::SYS_FONT.withHeight(APConstants::Gui::SHADOW_FONT_HEIGHT));
  graphics.drawText(name, 0, 0, shadow.getWidth(), shadow.getHeight(), juce::Justification::centred, false);
  kernel_->applyToImage(shadow, shadow, shadow.getBounds());
}

void Ap_dynamicsAudioProcessorEditor::setupSliderShadow(juce::Image& shadow)
{
  constexpr auto cornerSize = 10.0f;
  constexpr auto width      = 130;

  shadow = juce::Image(juce::Image::PixelFormat::ARGB, width, sliderHeight_, true);
  juce::Graphics graphics(shadow);
  graphics.setColour(APConstants::Colors::SHADOW_COLOR);
  graphics.fillRoundedRectangle(
      juce::Rectangle<float>(0, 0, static_cast<float>(shadow.getWidth() - 8), static_cast<float>(shadow.getHeight() - 8))
          .withCentre(shadow.getBounds().getCentre().toFloat()),
      cornerSize);
  kernel_->applyToImage(shadow, shadow, shadow.getBounds());
}

void Ap_dynamicsAudioProcessorEditor::setupSlider(std::unique_ptr<APSlider>& apSlider, std::unique_ptr<juce::Label>& label,
                                                  const juce::String& name, SliderType sliderType, const String& suffix)
{
  apSlider = std::make_unique<APSlider>(audioProcessor_, sliderType);
  apSlider->slider.setSliderStyle(juce::Slider::LinearBarVertical);
  apSlider->slider.setTextValueSuffix(" " + suffix);
  apSlider->slider.setColour(juce::Slider::trackColourId, juce::Colour(0xFFFFD479));
  apSlider->slider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::snow);
  apSlider->slider.setTextBoxIsEditable(true);

  label = std::make_unique<juce::Label>("", name);
  label->setJustificationType(juce::Justification::centred);
  label->setText(name.toLowerCase(), juce::dontSendNotification);
  label->setBorderSize(juce::BorderSize<int>(10, 0, 30, 70));
  label->setColour(juce::Label::textColourId, APConstants::Colors::DARK_GREY);
  label->setFont(APConstants::Gui::SYS_FONT.withHeight(APConstants::Gui::FONT_HEIGHT));
  label->attachToComponent(apSlider.get(), false);

  addAndMakeVisible(apSlider.get());
}

void Ap_dynamicsAudioProcessorEditor::timerCallback() { repaint(); }
void Ap_dynamicsAudioProcessorEditor::initializeAssets()
{
  bgText_ = std::make_unique<juce::Image>(
      juce::ImageCache::getFromMemory(BinaryData::logo_clean_png, BinaryData::logo_clean_pngSize));
  textShadow_ =
      std::make_unique<juce::Image>(juce::ImageCache::getFromMemory(BinaryData::shadow_png, BinaryData::shadow_pngSize));
  styleLabel_      = std::make_unique<juce::Label>("", "style");
  kernel_          = std::make_unique<juce::ImageConvolutionKernel>(16);
  thresholdShadow_ = std::make_unique<juce::Image>();
  ratioShadow_     = std::make_unique<juce::Image>();
  styleShadow_     = std::make_unique<juce::Image>();
  tSliderShadow_   = std::make_unique<juce::Image>();
  rSliderShadow_   = std::make_unique<juce::Image>();
  sSliderShadow_   = std::make_unique<juce::Image>();
}
