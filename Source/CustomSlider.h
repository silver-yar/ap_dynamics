/*
  ==============================================================================

    CustomSlider.h
    Created: 20 Jan 2021 4:38:04pm
    Author:  Johnathan Handy

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class CustomSlider : public juce::Slider
{
public:
    CustomSlider();
    ~CustomSlider() override;
private:
};

class MyLookAndFeel : public juce::LookAndFeel_V4
{
public:
    MyLookAndFeel(Ap_dynamicsAudioProcessor&);

    void drawLinearSlider (juce::Graphics &, int x, int y, int width, int height, float sliderPos,
                           float minSliderPos, float maxSliderPos, const juce::Slider::SliderStyle, juce::Slider &) override;
    void drawLinearSliderBackground (juce::Graphics &, int x, int y, int width, int height, float sliderPos,
                                     float minSliderPos, float maxSliderPos, const juce::Slider::SliderStyle style, juce::Slider &) override;
    juce::Label* createSliderTextBox (juce::Slider &) override;
    void drawLabel (juce::Graphics &, juce::Label &) override;

private:
    Ap_dynamicsAudioProcessor& audioProcessor;
    juce::Font labelFont_ {juce::Typeface::createSystemTypefaceFor(BinaryData::VarelaRound_ttf,
                                                                BinaryData::VarelaRound_ttfSize)};


    float bubblePosY1_ = bubStart_;
    float bubblePosX1_ = 20;
    float bubblePosY2_ = bubStart_;
    float bubblePosX2_ = 70;
    float bubblePosY3_ = bubStart_;
    float bubblePosX3_ = 120;
    int bubStart_;
    int labelMargin_ = 70;
    float lastSliderPos_ = 0;
    float sliderWidth_ = 0;
};