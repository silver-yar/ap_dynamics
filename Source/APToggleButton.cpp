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


}

void APToggleButton::resized()
{
//    juce::Grid grid;
//    using Track = juce::Grid::TrackInfo;
//    using Fr = juce::Grid::Fr;
//
//    for (auto choice : multiText_)
//    {
//        grid.items.add()
//    }
}

void APToggleButton::mouseDown(const juce::MouseEvent &e) {
    auto bounds = getLocalBounds();

    if (bounds.contains (e.getMouseDownPosition()))
        onButtonClicked();
}
