/*
  ==============================================================================

    ButtonMenu.cpp
    Created: 24 Dec 2020 12:43:15pm
    Author:  Johnathan Handy

  ==============================================================================
*/

#include <JuceHeader.h>
#include "ButtonMenu.h"

//==============================================================================
ButtonMenu::ButtonMenu(Ap_dynamicsAudioProcessor& p) : audioProcessor (p)
{
    dynType_ = std::make_unique<juce::ComboBox>("dynType");
    dynType_ -> addItem("Compressor", 1);
    dynType_ -> addItem("Expander", 2);
//    dynType_ -> setColour(juce::ComboBox::backgroundColourId, juce::Colour(0x00000000));
//    dynType_ -> setColour(juce::ComboBox::outlineColourId, juce::Colour(0xff00a9a9));
//    dynType_ -> setColour(juce::ComboBox::textColourId, juce::Colour(0xff00a9a9));
//    dynType_ -> setColour(juce::ComboBox::arrowColourId, juce::Colour(0xff00a9a9));
    dynAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>
            (audioProcessor.apvts, "DRT", *dynType_);
    addAndMakeVisible(dynType_.get());
    compType_ = std::make_unique<juce::ComboBox>("compType");
    compType_ -> addItem("Feedforward", 1);
    compType_ -> addItem("Feedback", 2);
    compType_ -> addItem("RMS", 3);
//    compType_ -> setColour(juce::ComboBox::backgroundColourId, juce::Colour(0x00000000));
//    compType_ -> setColour(juce::ComboBox::outlineColourId, juce::Colour(0xff00a9a9));
//    compType_ -> setColour(juce::ComboBox::textColourId, juce::Colour(0xff00a9a9));
//    compType_ -> setColour(juce::ComboBox::arrowColourId, juce::Colour(0xff00a9a9));
    compAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>
            (audioProcessor.apvts, "CT", *compType_);
    addAndMakeVisible(compType_.get());

    overdriveToggle_.setButtonText("Overdrive");
    addAndMakeVisible(overdriveToggle_);
}

ButtonMenu::~ButtonMenu()
{
}

void ButtonMenu::paint (juce::Graphics& g)
{
}

void ButtonMenu::resized()
{
    juce::Grid grid;
    using Track = juce::Grid::TrackInfo;
    using Fr = juce::Grid::Fr;

    grid.items.add (juce::GridItem (dynType_.get()));
    grid.items.add (juce::GridItem (compType_.get()));
    grid.items.add (juce::GridItem (overdriveToggle_));

    grid.templateColumns = {
            Track (Fr (1)),
    };
    grid.templateRows = {
            Track (Fr (1)),
            Track (Fr (1)),
            Track (Fr (1))
    };
    grid.rowGap = juce::Grid::Px (30);
    grid.performLayout (getLocalBounds().reduced(10));
}
