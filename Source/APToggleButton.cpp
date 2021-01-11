/*
  ==============================================================================

    APToggleButton.cpp
    Created: 27 Dec 2020 11:55:27pm
    Author:  Johnathan Handy

  ==============================================================================
*/

#include <JuceHeader.h>
#include "APToggleButton.h"

//==============================================================================
APToggleButton::APToggleButton()
{
}

APToggleButton::~APToggleButton()
{
}

void APToggleButton::paint (juce::Graphics& g)
{
    //auto glowBounds = getLocalBounds();
    //auto buttonBounds = glowBounds.reduced(5);
    g.setFont (myFont_.withHeight(10.0f));

    switch (buttonType_) {
        case Boolean:
            if (boolToggle) {
                g.fillAll(juce::Colours::goldenrod.withAlpha(0.7f));
            } else {
                g.fillAll(juce::Colours::darkgrey.withAlpha(0.7f));
            }

            g.setColour(juce::Colours::snow);
            g.drawFittedText(boolText_, getLocalBounds(), juce::Justification::centred, 1);
            break;
        case Multi:
            g.fillAll(juce::Colours::darkgrey.withAlpha(0.7f));
            g.setColour(juce::Colours::snow);
            g.drawFittedText(multiText_[intToggle], getLocalBounds(), juce::Justification::centred, 1);
            break;
        default:
            break;
    }
}

void APToggleButton::resized()
{

}

void APToggleButton::mouseDown(const juce::MouseEvent &e) {
    auto bounds = getLocalBounds();

    if (bounds.contains (e.getMouseDownPosition()))
        onButtonClicked();
}
