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
    setupSlider(kneeSlider_, kneeLabel_, "KWidth", "dBFS");
    kneeAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>
            (audioProcessor.apvts, "KW", *kneeSlider_);
    setupSlider(attackSlider_, attackLabel_, "Attack");
    attackAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>
            (audioProcessor.apvts, "ATT", *attackSlider_);
    setupSlider(releaseSlider_, releaseLabel_, "Release");
    releaseAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>
            (audioProcessor.apvts, "REL", *releaseSlider_);
    setupSlider(makeupSlider_, makeupLabel_, "Makeup", "dB");
    makeupAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>
            (audioProcessor.apvts, "MU", *makeupSlider_);
    setupSlider(toneSlider_, toneLabel_, "Tone", "Hz");
    toneAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>
            (audioProcessor.apvts, "TN", *toneSlider_);

    auto min_width = 750;
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

    g.drawImage (bgText_, getLocalBounds().toFloat(), juce::RectanglePlacement::fillDestination);

    auto bounds = getLocalBounds();
    auto graphArea = bounds.removeFromBottom(getHeight() * 0.4);
    auto plotArea = graphArea.removeFromRight(getWidth() * 0.3);

    g.setColour(juce::Colours::aquamarine);
    //g.drawRect(graphArea.reduced(10), 1);
    g.drawRect(plotArea.reduced(10), 1);

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
    //bounds.removeFromBottom (getHeight() * 0.4);
    auto wfBounds = bounds.removeFromBottom(getHeight() * 0.4);
    auto plotBounds = wfBounds.removeFromRight(getWidth() * 0.3);
    bounds.removeFromTop (40);
    auto menuBounds = bounds.removeFromRight(120);

    juce::Grid grid;
    using Track = juce::Grid::TrackInfo;
    using Fr = juce::Grid::Fr;

    grid.items.add (juce::GridItem (thresholdSlider_.get()));
    grid.items.add (juce::GridItem (ratioSlider_.get()));
    grid.items.add (juce::GridItem (kneeSlider_.get()));
    grid.items.add (juce::GridItem (attackSlider_.get()));
    grid.items.add (juce::GridItem (releaseSlider_.get()));
    grid.items.add (juce::GridItem (makeupSlider_.get()));
    grid.items.add (juce::GridItem (toneSlider_.get()));

    grid.templateColumns = {
            Track (Fr (1)),
            Track (Fr (1)),
            Track (Fr (1)),
            Track (Fr (1)),
            Track (Fr (1)),
            Track (Fr (1)),
            Track (Fr (1)),
    };
    grid.templateRows = { Track (Fr (1))};
    grid.columnGap = juce::Grid::Px (10);
    grid.performLayout (bounds.reduced(10));

    buttonMenu_ -> setBounds (menuBounds);
    waveformWindow_ -> setBounds (wfBounds.reduced(10));
    plot_ -> setBounds (plotBounds.reduced(10));
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
