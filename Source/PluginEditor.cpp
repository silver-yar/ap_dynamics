/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
Ap_dynamicsAudioProcessorEditor::Ap_dynamicsAudioProcessorEditor (Ap_dynamicsAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), apSliderLook_ (p), stylePicker_ (p)
{
    setupSlider(thresholdSlider_, thresholdLabel_, "Threshold", true, "dB");
    thresholdAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>
            (audioProcessor.apvts, "THR", *thresholdSlider_);

    setupSlider(ratioSlider_, ratioLabel_, "Ratio", false, ": 1");
    ratioAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>
            (audioProcessor.apvts, "RAT", *ratioSlider_);

    styleLabel_ = std::make_unique<juce::Label> ("", "style");
    styleLabel_ -> setJustificationType (juce::Justification::centred);
    styleLabel_ -> setText("style", juce::dontSendNotification);
    styleLabel_ -> setFont (myFont_.withHeight (24.0f));
    styleLabel_ -> setBorderSize(juce::BorderSize<int> (10, 50, 10, 0));
    styleLabel_ -> setColour (juce::Label::textColourId, juce::Colours::snow);
    styleLabel_ -> attachToComponent (&stylePicker_, false);
    addAndMakeVisible (stylePicker_);
//    styleSlider_ = std::make_unique<juce::ComboBox> ("Style");
//    styleSlider_->addItemList(juce::StringArray { "Clean", "Dirty", "Dirtier" }, 1);
//    addAndMakeVisible (styleSlider_.get());
//    styleAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>
//            (audioProcessor.apvts, "STY", *styleSlider_);


    auto min_width = 700;
    auto min_height = 500;
//    setResizeLimits(min_width, min_height, min_width * 2, min_height * 2);
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
//    g.setGradientFill (
//            juce::ColourGradient (juce::Colour(0xFFFFD479), getWidth() / 2, getHeight() / 4,
//                                  juce::Colour(0xFFFFD479),50,50,true)
//    );
    g.fillAll (juce::Colour(0xFFFFD479));

    g.drawImage (bgText_, getLocalBounds().removeFromTop(getHeight() * 0.4)
        .reduced(80, 40)
        .toFloat(),
        juce::RectanglePlacement::fillDestination);

}

void Ap_dynamicsAudioProcessorEditor::resized()
{
    auto sBounds = getLocalBounds().removeFromBottom(getHeight() * 0.5).reduced(50, 40);
    thresholdSlider_->setBounds(juce::Rectangle<int> (40, 275,
                                                        200, 185));
    ratioSlider_->setBounds(juce::Rectangle<int> (280, 275,
                                                        200, 185));
    stylePicker_.setBounds(juce::Rectangle<int> (460, 275,
                                                        200, 185));
//    ratioSlider_ -> setBounds(sBounds.removeFromLeft(200));
//    stylePicker_.setBounds(sBounds);
//    auto bounds = getLocalBounds();
//    auto sBounds = bounds.removeFromBottom(getHeight() * 0.5);
//
//    juce::Grid grid;
//    using Track = juce::Grid::TrackInfo;
//    using Fr = juce::Grid::Fr;
//
//    grid.items.add (juce::GridItem (thresholdSlider_.get()));
//    grid.items.add (juce::GridItem (ratioSlider_.get()));
//    grid.items.add (juce::GridItem (stylePicker_));
//
//
//    grid.templateColumns = {
//            Track (Fr (1)),
//            Track (Fr (1)),
//            Track (Fr (1))
//    };
//    grid.templateRows = { Track (Fr (1))};
//    grid.columnGap = juce::Grid::Px (5);
//    grid.performLayout (sBounds.reduced(60, 30));
}

void Ap_dynamicsAudioProcessorEditor::setupSlider(std::unique_ptr<CustomSlider> &slider,
                                                  std::unique_ptr<juce::Label> &label,
                                                  const juce::String &name,
                                                  bool showMeter,
                                                  const juce::String &suffix) {
    slider = std::make_unique<CustomSlider>();
    if (showMeter) {
        slider -> setTextBoxIsEditable(showMeter);
    } else {
        slider -> setTextBoxIsEditable(showMeter);
    }
    slider -> setSliderStyle(juce::Slider::LinearBarVertical);
    slider -> setLookAndFeel(&apSliderLook_);
    slider -> setTextValueSuffix(" " + suffix);
    slider -> setColour (juce::Slider::trackColourId, juce::Colour(0xFFFFD479));
    slider -> setColour (juce::Slider::textBoxTextColourId, juce::Colours::snow);
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
