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
    dynToggle_.setButtonType(APToggleButton::Multi);
    dynToggle_.setMultiText({"Compressor", "Expander"});
    dynToggle_.onButtonClicked = []() {

    };
    addAndMakeVisible(dynToggle_);

    compToggle_.setButtonType(APToggleButton::Multi);
    juce::Array<juce::String> options = { "Feedforward", "Feedback", "RMS" };
    compToggle_.setMultiText(options);
    compToggle_.onButtonClicked = [&, this]() {
        if (compToggle_.intToggle < 2) {
            compToggle_.intToggle++;
            audioProcessor.setCompTypeID(compToggle_.intToggle);
        } else {
            compToggle_.intToggle = 0;
        }
    };
    addAndMakeVisible(compToggle_);

    overdriveToggle_.setButtonType(APToggleButton::Boolean);
    overdriveToggle_.onButtonClicked = [this]() {
        audioProcessor.setOverdrive(!audioProcessor.getOverdrive());
        overdriveToggle_.boolToggle = audioProcessor.getOverdrive();
    };
    overdriveToggle_.setBoolText("Overdrive");
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

    grid.items.add (juce::GridItem (dynToggle_));
    grid.items.add (juce::GridItem (compToggle_));
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
    grid.performLayout (getLocalBounds().withTrimmedRight(10)
                                        .reduced(10)
                                        );
}
