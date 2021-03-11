/*
  ==============================================================================

    ShowHideContainer.h
    Created: 9 Mar 2021 12:03:16am
    Author:  Johnathan Handy

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*/
class ShowHideContainer  : public juce::Component
{
public:
    ShowHideContainer()
    {
        setAlpha (0.2f);

        xOffset_.setInputRestrictions (0, "1234567890-");
        xOffset_.setText(juce::String(offset.getX()), false);
        xOffset_.setReturnKeyStartsNewLine (false);
        xOffset_.setMultiLine (false);
        xOffset_.onReturnKey = [this]() {
            if (!xOffset_.isEmpty())
            {
                offset.setX (xOffset_.getText().getFloatValue());
                resized();
            }
        };

        xOffset_.setBounds(0,0,100,50);
        addAndMakeVisible (xOffset_);

        yOffset_.setInputRestrictions (0, "1234567890-");
        yOffset_.setText(juce::String(offset.getY()), false);
        yOffset_.setReturnKeyStartsNewLine (false);
        yOffset_.setMultiLine (false);
        yOffset_.onReturnKey = [this]() {
            if (!yOffset_.isEmpty())
            {
                offset.setY (yOffset_.getText().getFloatValue());
                resized();
            }
        };

        yOffset_.setBounds(0,50,100,50);
        addAndMakeVisible (yOffset_);

        deltaEditor_.setInputRestrictions(0, "1234567890");
        deltaEditor_.setText(juce::String(shadowDeltaXY), false);
        deltaEditor_.setReturnKeyStartsNewLine (false);
        deltaEditor_.setMultiLine (false);
        deltaEditor_.onReturnKey = [this]() {
            if (!deltaEditor_.isEmpty())
            {
                shadowDeltaXY = deltaEditor_.getText().getFloatValue();
                resized();
            }
        };

        deltaEditor_.setBounds(0,100,100,50);
        addAndMakeVisible (deltaEditor_);
    }

    ~ShowHideContainer() override
    {
    }

    juce::Point<float> offset {0, -10};
    int shadowDeltaXY = 5;

private:
    juce::TextEditor xOffset_, yOffset_, deltaEditor_;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ShowHideContainer)
};
