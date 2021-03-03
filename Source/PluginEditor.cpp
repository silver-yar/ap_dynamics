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
    setupSlider(thresholdSlider_, thresholdLabel_, "Threshold",
                SliderType::Invert, "dB");
    thresholdSlider_->slider.setLookAndFeel (&thresholdLook_);
    thresholdAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>
            (audioProcessor.apvts, "THR", thresholdSlider_->slider);

    setupSlider(ratioSlider_, ratioLabel_, "Ratio",
                SliderType::Normal, ": 1");
    ratioSlider_->slider.setLookAndFeel (&ratioLook_);
    ratioAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>
            (audioProcessor.apvts, "RAT", ratioSlider_->slider);

    styleLabel_ = std::make_unique<juce::Label> ("", "style");
    styleLabel_ -> setJustificationType (juce::Justification::centred);
    styleLabel_ -> setText("style", juce::dontSendNotification);
    styleLabel_ -> setFont (myFont_.withHeight (24.0f));
    styleLabel_ -> setBorderSize(juce::BorderSize<int> (10, 50, 10, 0));
    styleLabel_ -> setColour (juce::Label::textColourId, juce::Colours::snow);
    styleLabel_ -> attachToComponent (&stylePicker_, false);
    addAndMakeVisible (stylePicker_);

    thresholdBounds_ = juce::Rectangle<int> (40, 270, 200, sliderHeight_);

    ratioBounds_ = juce::Rectangle<int> (280, 270, 200, sliderHeight_);

    pickerBounds_ = juce::Rectangle<int> (470, 270, 200, sliderHeight_);

    auto min_width = 700;
    auto min_height = 500;

    setSize (min_width, min_height);
    startTimerHz(60);
}

Ap_dynamicsAudioProcessorEditor::~Ap_dynamicsAudioProcessorEditor()
{
    stopTimer();
}

//==============================================================================
void Ap_dynamicsAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.setGradientFill (
            juce::ColourGradient (juce::Colour(0xFFFFD479), getWidth() / 2, getHeight() / 2,
                                  juce::Colour(0xFFFFC446),-80,-80,true)
    );
    g.fillAll ();

    // Logo
    g.drawImage (bgText_, getLocalBounds().removeFromTop(getHeight() * 0.4)
        .reduced(80, 40)
        .toFloat(),
        juce::RectanglePlacement::fillDestination);

    // Shadows
    g.setColour(juce::Colours::black.withAlpha(0.2f));

    auto shadowBounds = juce::Rectangle<float> (80, 290, 130, sliderHeight_);

    g.fillRoundedRectangle(shadowBounds,10.0f);
    g.fillRoundedRectangle(shadowBounds.withX(255.0f),10.0f);
    g.fillRoundedRectangle(shadowBounds.withX(305.0f),10.0f);
    g.fillRoundedRectangle(shadowBounds.withX(490),10.0f);
}

void Ap_dynamicsAudioProcessorEditor::resized()
{
    thresholdSlider_->setBounds(thresholdBounds_);

    ratioSlider_->setBounds(ratioBounds_);

    stylePicker_.setBounds(pickerBounds_);
}

void Ap_dynamicsAudioProcessorEditor::setupSlider(std::unique_ptr<CustomSlider_> &slider,
                                                  std::unique_ptr<juce::Label> &label,
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
    label -> setBorderSize(juce::BorderSize<int> (10, 0, 10, 70));
    label -> setColour (juce::Label::textColourId, juce::Colours::snow);
    label -> setFont (myFont_.withHeight (24.0f));
    label -> attachToComponent(slider.get(), false);

    addAndMakeVisible(slider.get());
    addAndMakeVisible(label.get());
}

void Ap_dynamicsAudioProcessorEditor::timerCallback()
{
    repaint();
}
