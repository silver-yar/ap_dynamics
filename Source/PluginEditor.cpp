/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
Ap_dynamicsAudioProcessorEditor::Ap_dynamicsAudioProcessorEditor (Ap_dynamicsAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    plot_ = std::make_unique<APPlot>(audioProcessor);
    addAndMakeVisible(plot_.get());
    buttonMenu_ = std::make_unique<ButtonMenu> (audioProcessor);
    addAndMakeVisible(buttonMenu_.get());
    waveformWindow_ = std::make_unique<WaveformWindow> (audioProcessor);
    addAndMakeVisible (waveformWindow_.get());

    setupSlider(thresholdSlider_, thresholdLabel_, "Threshold", "dBFS");
    thresholdAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>
            (audioProcessor.apvts, "THR", *thresholdSlider_);
    setupSlider(ratioSlider_, ratioLabel_, "Ratio", ": 1");
    ratioAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>
            (audioProcessor.apvts, "RAT", *ratioSlider_);
    styleSlider_ = std::make_unique<juce::ComboBox> ("Style");
    styleSlider_->addItemList(juce::StringArray { "Clean", "Dirty", "Dirtier" }, 1);
    addAndMakeVisible (styleSlider_.get());
    styleAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>
            (audioProcessor.apvts, "STY", *styleSlider_);


    auto min_width = 500;
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
    g.setGradientFill (
            juce::ColourGradient (juce::Colours::lightgrey, getWidth() / 2, getHeight() / 2,juce::Colours::grey,0,0,true)
    );
    g.fillAll ();

    g.drawImage (bgText_, getLocalBounds().removeFromTop(getHeight() * 0.5).toFloat(),
                 juce::RectanglePlacement::fillDestination);

    //g.drawRect(graphArea.reduced(10), 1);

    // Volume Meter
//    auto hasClipped = juce::Decibels::gainToDecibels (audioProcessor.meterLocalMaxVal.load()) >= 0.0f;
//    auto dbVal = juce::Decibels::gainToDecibels (audioProcessor.meterLocalMaxVal.load(),
//                                                 -60.0f);
//    dbVal = juce::jlimit (-60.0f, 0.0f, dbVal);
//
//    auto meterBounds = juce::Rectangle<float> (getWidth() * .3f, 0, getWidth() * .12f, getHeight() * 0.6);
//    meterBounds.reduce (10, 10);
//
//    g.setColour (getLookAndFeel().findColour
//        (juce::ResizableWindow::backgroundColourId).withAlpha (0.5f));
//    g.fillRect (meterBounds);
//
//    meterBounds.removeFromTop (meterBounds.getHeight() * -dbVal / 100.0f);
//    g.setColour (hasClipped ? juce::Colours::red : juce::Colours::green.brighter());
//    g.fillRect (meterBounds);
}

void Ap_dynamicsAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    auto sBounds = bounds.removeFromBottom(getHeight() * 0.5);

    juce::Grid grid;
    using Track = juce::Grid::TrackInfo;
    using Fr = juce::Grid::Fr;

    grid.items.add (juce::GridItem (thresholdSlider_.get()));
    grid.items.add (juce::GridItem (ratioSlider_.get()));
    grid.items.add (juce::GridItem (styleSlider_.get()));


    grid.templateColumns = {
            Track (Fr (1)),
            Track (Fr (1)),
            Track (Fr (1))
    };
    grid.templateRows = { Track (Fr (1))};
    grid.columnGap = juce::Grid::Px (10);
    grid.performLayout (sBounds.reduced(10));
}

void Ap_dynamicsAudioProcessorEditor::setupSlider(std::unique_ptr<juce::Slider> &slider,
                                                  std::unique_ptr<juce::Label> &label,
                                                  const juce::String &name,
                                                  const juce::String &suffix) {
    slider = std::make_unique<juce::Slider> (juce::Slider::SliderStyle::LinearBarVertical,
                                             juce::Slider::TextEntryBoxPosition::TextBoxAbove);
    slider -> setLookAndFeel(&apSliderLook_);
    slider -> setTextValueSuffix(" " + suffix);
    slider -> setColour (juce::Slider::trackColourId, juce::Colours::darkgrey.withAlpha(0.7f));
    slider -> setColour (juce::Slider::textBoxTextColourId, juce::Colours::snow);
    label = std::make_unique<juce::Label> ("", name);
    label -> setJustificationType(juce::Justification::centred);
    label -> setText (name.toLowerCase(), juce::dontSendNotification);
    label -> setBorderSize(juce::BorderSize<int> (10));
    label -> setColour (juce::Label::textColourId, juce::Colours::snow);
    label -> setFont (myFont_.withHeight (16.0f));
    label -> attachToComponent(slider.get(), false);

    addAndMakeVisible(slider.get());
    addAndMakeVisible(label.get());
}

void Ap_dynamicsAudioProcessorEditor::timerCallback()
{
    repaint();
}
