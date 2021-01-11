/*
  ==============================================================================

    APSliderLook.h
    Created: 29 Dec 2020 1:56:17pm
    Author:  Johnathan Handy

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class APSliderLook : public juce::LookAndFeel_V4
{
public:
    juce::Label* createSliderTextBox (juce::Slider& slider) override
    {
        auto* l = juce::LookAndFeel_V4::createSliderTextBox (slider);

        if (getCurrentColourScheme() == juce::LookAndFeel_V4::getGreyColourScheme() && (slider.getSliderStyle() == juce::Slider::LinearBar
                                                                                   || slider.getSliderStyle() == juce::Slider::LinearBarVertical))
        {
            l->setColour (juce::Label::textColourId, juce::Colours::black.withAlpha (0.7f));
            l->setFont (myFont_);
        }

        return l;
    }

private:
    juce::Font myFont_ {juce::Typeface::createSystemTypefaceFor(BinaryData::Antipasto_Med_ttf,
                                                                BinaryData::Antipasto_Med_ttfSize)};
};