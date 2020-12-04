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
    dynType_ = std::make_unique<juce::ComboBox>("dynType");
    dynType_ -> addItem("Compressor", 1);
    dynType_ -> addItem("Expander", 2);
    dynType_ -> setColour(juce::ComboBox::backgroundColourId, juce::Colour(0x00000000));
    dynType_ -> setColour(juce::ComboBox::outlineColourId, juce::Colour(0xff00a9a9));
    dynType_ -> setColour(juce::ComboBox::textColourId, juce::Colour(0xff00a9a9));
    dynType_ -> setColour(juce::ComboBox::arrowColourId, juce::Colour(0xff00a9a9));
    dynAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>
            (audioProcessor.apvts, "DRT", *dynType_);
    addAndMakeVisible(dynType_.get());
    compType_ = std::make_unique<juce::ComboBox>("compType");
    compType_ -> addItem("Feedforward", 1);
    compType_ -> addItem("Feedback", 2);
    compType_ -> addItem("RMS", 3);
    compType_ -> setColour(juce::ComboBox::backgroundColourId, juce::Colour(0x00000000));
    compType_ -> setColour(juce::ComboBox::outlineColourId, juce::Colour(0xff00a9a9));
    compType_ -> setColour(juce::ComboBox::textColourId, juce::Colour(0xff00a9a9));
    compType_ -> setColour(juce::ComboBox::arrowColourId, juce::Colour(0xff00a9a9));
    compAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>
            (audioProcessor.apvts, "CT", *compType_);
    addAndMakeVisible(compType_.get());

    plot_ = std::make_unique<APPlot>(audioProcessor);
    addAndMakeVisible(plot_.get());

    setupSlider(thresholdSlider_, thresholdLabel_, "Threshold", "dBFS");
    thresholdAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>
            (audioProcessor.apvts, "THR", *thresholdSlider_);
    setupSlider(ratioSlider_, ratioLabel_, "Ratio", ": 1");
    ratioAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>
            (audioProcessor.apvts, "RAT", *ratioSlider_);
    setupSlider(kneeSlider_, kneeLabel_, "Knee Width", "dBFS");
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
    g.fillAll (juce::Colour(0xff003232));

    auto menuEnd = getWidth() * 0.3f;
    auto menuTextBounds = juce::Rectangle<int> (0, getHeight() * .05f, menuEnd, getHeight() * .05f);
    g.setColour(juce::Colour(0xff00a9a9));
    g.drawFittedText("Menu", menuTextBounds, juce::Justification::centred, 1);
    g.drawLine(menuEnd, 10, menuEnd, getHeight() - 10);

    auto logoBounds = juce::Rectangle<float> (0, getHeight() * 0.70f, menuEnd, getHeight() * 0.25f);
    g.drawImage(logo_, logoBounds, juce::RectanglePlacement::centred);

    // Volume Meter
    auto hasClipped = juce::Decibels::gainToDecibels (audioProcessor.meterLocalMaxVal.load()) >= 0.0f;
    auto dbVal = juce::Decibels::gainToDecibels (audioProcessor.meterLocalMaxVal.load(),
                                                 -60.0f);
    dbVal = juce::jlimit (-60.0f, 0.0f, dbVal);

    auto meterBounds = juce::Rectangle<float> (getWidth() * .3f, 0, getWidth() * .12f, getHeight() * 0.6);
    meterBounds.reduce (10, 10);

    g.setColour (getLookAndFeel().findColour
        (juce::ResizableWindow::backgroundColourId).withAlpha (0.5f));
    g.fillRect (meterBounds);

    meterBounds.removeFromTop (meterBounds.getHeight() * -dbVal / 100.0f);
    g.setColour (hasClipped ? juce::Colours::red : juce::Colours::green.brighter());
    g.fillRect (meterBounds);
}

void Ap_dynamicsAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    auto rightPane = bounds.removeFromRight(getWidth() * 0.6f);
    auto plotBounds = rightPane.removeFromTop(getHeight() * 0.6f);
    plotBounds.reduce(10, 10);

    auto menuEnd = getWidth() * 0.3f;

    auto dynTypeBounds = juce::Rectangle<int> (0, getHeight() * .15f, menuEnd, getHeight() * .05f);
    dynTypeBounds.reduce(10, 0);
    auto compTypeBounds = juce::Rectangle<int> (0, getHeight() * .25f, menuEnd, getHeight() * .05f);
    compTypeBounds.reduce(10, 0);

    thresholdSlider_ -> setBounds(rightPane.getTopLeft().getX() + 20,rightPane.getTopLeft().getY() + 20,
                                  rightPane.getWidth() * .25, rightPane.getHeight() * .1);
    ratioSlider_ -> setBounds(rightPane.getTopLeft().getX() + 20,rightPane.getTopLeft().getY() + 70,
                                  rightPane.getWidth() * .25, rightPane.getHeight() * .1);
    kneeSlider_ -> setBounds(rightPane.getTopLeft().getX() + 20,rightPane.getTopLeft().getY() + 120,
                                  rightPane.getWidth() * .25, rightPane.getHeight() * .1);
    attackSlider_ -> setBounds(rightPane.getTopLeft().getX() + 120,rightPane.getTopLeft().getY() + 20,
                                  rightPane.getWidth() * .25, rightPane.getHeight() * .1);
    releaseSlider_ -> setBounds(rightPane.getTopLeft().getX() + 120,rightPane.getTopLeft().getY() + 70,
                                  rightPane.getWidth() * .25, rightPane.getHeight() * .1);
    makeupSlider_ -> setBounds(rightPane.getTopLeft().getX() + 120,rightPane.getTopLeft().getY() + 120,
                                rightPane.getWidth() * .25, rightPane.getHeight() * .1);

    plot_ -> setBounds(plotBounds);
    dynType_ -> setBounds(dynTypeBounds);
    compType_ -> setBounds(compTypeBounds);
}

void Ap_dynamicsAudioProcessorEditor::setupSlider(std::unique_ptr<juce::Slider> &slider,
                                                  std::unique_ptr<juce::Label> &label,
                                                  const juce::String &name,
                                                  const juce::String &suffix) {
    slider = std::make_unique<juce::Slider> (juce::Slider::SliderStyle::LinearBar,
                                             juce::Slider::TextEntryBoxPosition::TextBoxAbove);
    slider -> setTextValueSuffix(" " + suffix);
    slider -> setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(0xff00a9a9));
    slider -> setColour(juce::Slider::trackColourId,
                        juce::Colour(0xff00a9a9).withAlpha(0.7f));
    label = std::make_unique<juce::Label> ("", name);
    label -> setJustificationType(juce::Justification::centred);
    label -> attachToComponent(slider.get(), false);

    addAndMakeVisible(slider.get());
    addAndMakeVisible(label.get());
}

void Ap_dynamicsAudioProcessorEditor::timerCallback()
{
    repaint();
}
