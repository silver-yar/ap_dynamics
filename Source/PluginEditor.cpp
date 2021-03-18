/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
Ap_dynamicsAudioProcessorEditor::Ap_dynamicsAudioProcessorEditor (Ap_dynamicsAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),
                                 thresholdLook_ (p, Invert),
                                 ratioLook_ (p, Normal),
                                 stylePicker_ (p)
{
    setupSlider(thresholdSlider_, thresholdLabel_, lshdwT_, "Threshold",
                SliderType::Invert, "dB");
    thresholdSlider_->slider.setLookAndFeel (&thresholdLook_);
    thresholdAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>
            (audioProcessor.apvts, "THR", thresholdSlider_->slider);

    setupSlider(ratioSlider_, ratioLabel_, lshdwR_, "Ratio",
                SliderType::Normal, ": 1");
    ratioSlider_->slider.setLookAndFeel (&ratioLook_);
    ratioAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>
            (audioProcessor.apvts, "RAT", ratioSlider_->slider);

    styleLabel_ = std::make_unique<juce::Label> ("", "style");
    styleLabel_ -> setJustificationType (juce::Justification::centred);
    styleLabel_ -> setText("style", juce::dontSendNotification);
    styleLabel_ -> setFont (myFont_.withHeight (FONT_HEIGHT));
    styleLabel_ -> setBorderSize(juce::BorderSize<int> (10, 50, 30, 0));
    styleLabel_ -> setColour (juce::Label::textColourId, AP::Colors::DarkGrey);
    styleLabel_ -> attachToComponent (&stylePicker_, false);

    lshdwS_ = std::make_unique<juce::Label> ("", "style");
    lshdwS_ -> setJustificationType (juce::Justification::centred);
    lshdwS_ -> setText("style", juce::dontSendNotification);
    lshdwS_ -> setFont (myFont_.withHeight (SHADOW_FONT_HEIGHT));
    lshdwS_ -> setBorderSize(juce::BorderSize<int> (10, 50, 38, 0));
    lshdwS_ -> setColour (juce::Label::textColourId, SHADOW_COLOR);
    lshdwS_ -> attachToComponent (&stylePicker_, false);
    addAndMakeVisible (stylePicker_);

    thresholdBounds_ = juce::Rectangle<int> (40, SLIDER_Y, SLIDER_WIDTH, sliderHeight_);

    ratioBounds_ = juce::Rectangle<int> (280, SLIDER_Y, SLIDER_WIDTH, sliderHeight_);

    pickerBounds_ = juce::Rectangle<int> (470, SLIDER_Y, SLIDER_WIDTH, sliderHeight_);

    setSize (M_WIDTH, M_HEIGHT);
    startTimerHz (60);

    kernel_.createGaussianBlur (8.0f);
    auto logoBounds = getLocalBounds().removeFromTop(getHeight() * 0.45f).reduced(80, 40);
    kernel_.applyToImage(textShadow_, textShadow_, textShadow_.getBounds());
    
//    testBox_.setBounds(0, 0, 100, 150);
//    addAndMakeVisible (testBox_);
}

Ap_dynamicsAudioProcessorEditor::~Ap_dynamicsAudioProcessorEditor()
{
    stopTimer();
}

int mouseX = 0;
int mouseY = 0;

//==============================================================================
void Ap_dynamicsAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.setGradientFill (
            juce::ColourGradient (INNER_GRADIENT_BG, getWidth() * 0.5f, getHeight() * 0.93f,
                                  OUTER_GRADIENT_BG,-80,-80,true)
    );
    g.fillAll();

//    g.setColour(juce::Colours::black);
//    g.drawText("x: " + juce::String((float) mouseX / getWidth()) + ", y: " +
//                juce::String((float) mouseY / getHeight()), 10, 10, 200, 50,
//               juce::Justification::centred, false);

    // Logo
    constexpr int textDeltaX = 80;
    constexpr int textDeltaY = 40;
    const int textHeight = getHeight() * 0.45f;
    const auto textBounds = getLocalBounds().removeFromTop(textHeight)
            .reduced(textDeltaX, textDeltaY)
            .toFloat();

    // Draw Test Label
//    shadow_.drawForImage(g, labelImage_);
//    g.drawImage(labelImage_,
//                juce::Rectangle<float> (mouseX, mouseY, 200, 100),
//                juce::RectanglePlacement::fillDestination);

    // Draw Logo Shadow
//    shadowEffect.applyEffect (bgText_, g, 1.0f, 0.5f);
//    shadow.drawForImage(g, bgText_);
//    g.setOpacity(1.0f);
    g.drawImage (textShadow_,
                 textBounds.withY(textBounds.getY() - 15).expanded(testBox_.shadowDeltaXY * 1.5f) +
                 testBox_.offset,
                 juce::RectanglePlacement::fillDestination);

    g.drawImage (bgText_, textBounds,
        juce::RectanglePlacement::fillDestination);


    // Slider Shadows
    g.setColour(SHADOW_COLOR);

    auto shadowBounds = juce::Rectangle<float> (40, SLIDER_Y, 130, sliderHeight_);

    g.fillRoundedRectangle(shadowBounds.expanded(testBox_.shadowDeltaXY) +
                                                        testBox_.offset, 10.0f);
    g.fillRoundedRectangle(shadowBounds.withX(280).expanded(testBox_.shadowDeltaXY) +
                                                        testBox_.offset, 10.0f);
    g.fillRoundedRectangle(shadowBounds.withX(530).expanded(testBox_.shadowDeltaXY) +
                                                        testBox_.offset, 10.0f);
}

void Ap_dynamicsAudioProcessorEditor::resized()
{
    thresholdSlider_->setBounds(thresholdBounds_);

    ratioSlider_->setBounds(ratioBounds_);

    stylePicker_.setBounds(pickerBounds_);
}

void Ap_dynamicsAudioProcessorEditor::setupSlider(std::unique_ptr<CustomSlider_> &slider,
                                                  std::unique_ptr<juce::Label> &label,
                                                  std::unique_ptr<juce::Label> &labelShadow,
                                                  const juce::String &name,
                                                  SliderType sliderType,
                                                  const juce::String &suffix) {
    slider = std::make_unique<CustomSlider_> (audioProcessor, sliderType);
    slider -> slider.setTextBoxIsEditable (false);
    slider -> slider.setSliderStyle(juce::Slider::LinearBarVertical);
    slider -> slider.setTextValueSuffix (" " + suffix);
    slider -> slider.setColour (juce::Slider::trackColourId, juce::Colour(0xFFFFD479));
    slider -> slider.setColour (juce::Slider::textBoxTextColourId, juce::Colours::snow);
    
    label = std::make_unique<juce::Label> ("", name);
    label -> setJustificationType(juce::Justification::centred);
    label -> setText (name.toLowerCase(), juce::dontSendNotification);
    label -> setBorderSize(juce::BorderSize<int> (10, 0, 30, 70));
    label -> setColour (juce::Label::textColourId, AP::Colors::DarkGrey);
    label -> setFont (myFont_.withHeight (FONT_HEIGHT));
    label -> attachToComponent(slider.get(), false);

    labelShadow = std::make_unique<juce::Label> ("", name);
    labelShadow -> setJustificationType(juce::Justification::centred);
    labelShadow -> setText (name.toLowerCase(), juce::dontSendNotification);
    labelShadow -> setBorderSize(juce::BorderSize<int> (10, 0, 38, 70));
    labelShadow -> setColour (juce::Label::textColourId, SHADOW_COLOR);
    labelShadow -> setFont (myFont_.withHeight (SHADOW_FONT_HEIGHT));
    labelShadow -> attachToComponent(slider.get(), false);

    addAndMakeVisible(slider.get());
}

void Ap_dynamicsAudioProcessorEditor::timerCallback()
{
    auto window = juce::Desktop::getInstance().getComponent(0);
    auto pos = juce::Desktop::getInstance().getMousePosition();
    mouseX = pos.x - window->getX();
    mouseY = pos.y - window->getY();

//    juce::uint8 mappedColor = juce::jmap(mouseY , 0, getHeight(),0, 255);
//    auto color = juce::Colour(mappedColor, mappedColor, mappedColor, (juce::uint8) 255);
//    thresholdLabel_->setColour(juce::Label::textColourId, AP::Colors::DarkGrey);
//    ratioLabel_->setColour(juce::Label::textColourId, AP::Colors::DarkGrey);
//    styleLabel_->setColour(juce::Label::textColourId, AP::Colors::DarkGrey);

    repaint();
}
