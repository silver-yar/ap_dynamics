/*
  ==============================================================================

    APStyle.h
    Created: 1 Dec 2020 1:53:47am
    Author:  Johnathan Handy

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class APStyle : public juce::LookAndFeel_V4
{
    APStyle() = default;;
    ~APStyle() override = default;;

    void drawLinearSlider	(juce::Graphics &, int x, int y, int width, int height,
                           float sliderPos, float minSliderPos, float maxSliderPos,
                           const juce::Slider::SliderStyle, juce::Slider &) override;
};
