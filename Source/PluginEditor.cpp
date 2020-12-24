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
//    plot_ = std::make_unique<APPlot>(audioProcessor);
//    addAndMakeVisible(plot_.get());
    buttonMenu_ = std::make_unique<ButtonMenu> (audioProcessor);
    addAndMakeVisible(buttonMenu_.get());

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
    g.drawRect(graphArea.reduced(10), 1);
    g.drawRect(plotArea.reduced(10), 1);

//    auto menuEnd = getWidth() * 0.3f;
//    auto menuTextBounds = juce::Rectangle<int> (0, getHeight() * .05f, menuEnd, getHeight() * .05f);
//    g.setColour(juce::Colour(0xff00a9a9));
//    g.drawFittedText("Menu", menuTextBounds, juce::Justification::centred, 1);
//    g.drawLine(menuEnd, 10, menuEnd, getHeight() - 10);

//    auto logoBounds = juce::Rectangle<float> (0, getHeight() * 0.70f, menuEnd, getHeight() * 0.25f);
//    g.drawImage(logo_, logoBounds, juce::RectanglePlacement::centred);

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
    bounds.removeFromBottom (getHeight() * 0.4);
    // auto graphArea = bounds.removeFromBottom(getHeight() * 0.4);
    // auto plotArea = graphArea.removeFromRight(getWidth() * 0.3);
    bounds.removeFromTop (40);
    auto menuBounds = bounds.removeFromRight(100);

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
    grid.performLayout (bounds.reduced(20));
    buttonMenu_ -> setBounds (menuBounds);
//    auto bounds = getLocalBounds();
//    auto rightPane = bounds.removeFromRight(getWidth() * 0.6f);
//    auto plotBounds = rightPane.removeFromTop(getHeight() * 0.6f);
//    plotBounds.reduce(10, 10);
//
//    auto menuEnd = getWidth() * 0.3f;
//
//    auto dynTypeBounds = juce::Rectangle<int> (0, getHeight() * .15f, menuEnd, getHeight() * .05f);
//    dynTypeBounds.reduce(10, 0);
//    auto compTypeBounds = juce::Rectangle<int> (0, getHeight() * .25f, menuEnd, getHeight() * .05f);
//    compTypeBounds.reduce(10, 0);
//
//    thresholdSlider_ -> setBounds(rightPane.getTopLeft().getX() + 10,rightPane.getTopLeft().getY() + 20,
//                                  rightPane.getWidth() * .25, rightPane.getHeight() * .1);
//    ratioSlider_ -> setBounds(rightPane.getTopLeft().getX() + 10,rightPane.getTopLeft().getY() + 70,
//                                  rightPane.getWidth() * .25, rightPane.getHeight() * .1);
//    kneeSlider_ -> setBounds(rightPane.getTopLeft().getX() + 10,rightPane.getTopLeft().getY() + 120,
//                                  rightPane.getWidth() * .25, rightPane.getHeight() * .1);
//    attackSlider_ -> setBounds(rightPane.getTopLeft().getX() + 110,rightPane.getTopLeft().getY() + 20,
//                                  rightPane.getWidth() * .25, rightPane.getHeight() * .1);
//    releaseSlider_ -> setBounds(rightPane.getTopLeft().getX() + 110,rightPane.getTopLeft().getY() + 70,
//                                  rightPane.getWidth() * .25, rightPane.getHeight() * .1);
//    makeupSlider_ -> setBounds(rightPane.getTopLeft().getX() + 110,rightPane.getTopLeft().getY() + 120,
//                                rightPane.getWidth() * .25, rightPane.getHeight() * .1);
//    toneSlider_ -> setBounds(rightPane.getTopLeft().getX() + 110,rightPane.getTopLeft().getY() + 170,
//                               rightPane.getWidth() * .25, rightPane.getHeight() * .1);
//
//    plot_ -> setBounds(plotBounds);
//    dynType_ -> setBounds(dynTypeBounds);
//    compType_ -> setBounds(compTypeBounds);
}

void Ap_dynamicsAudioProcessorEditor::setupSlider(std::unique_ptr<juce::Slider> &slider,
                                                  std::unique_ptr<juce::Label> &label,
                                                  const juce::String &name,
                                                  const juce::String &suffix) {
    slider = std::make_unique<juce::Slider> (juce::Slider::SliderStyle::LinearBarVertical,
                                             juce::Slider::TextEntryBoxPosition::TextBoxAbove);
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
