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
#include "./OpenGL/SliderBarGL.h"
#include "APDefines.h"

enum SliderType
{
    Normal = 1,
    Invert = 2
};

class CustomSlider : public juce::Slider
{
public:
    CustomSlider();
    ~CustomSlider() override;

private:
};

class CustomSlider_ : public juce::Component, public juce::Timer
{
public:
    static constexpr float cornerSize = 10.0f;

    CustomSlider_(Ap_dynamicsAudioProcessor&, SliderType);
    ~CustomSlider_();

    void resized() override;
    void timerCallback() override;

    CustomSlider slider;
private:
    Ap_dynamicsAudioProcessor& audioProcessor;
    SliderType sliderType_;
    SliderBarGL threshSliderBar_ {"liquidmetal.shader"};
    SliderBarGL ratioSliderBar_ {"basic.shader"};

    juce::Rectangle<int> threshBounds_, ratioBounds_;
};

class MyLookAndFeel : public juce::LookAndFeel_V4
{
public:
    MyLookAndFeel(Ap_dynamicsAudioProcessor&, SliderType);

    void drawLinearSlider (juce::Graphics &, int x, int y, int width, int height, float sliderPos,
                           float minSliderPos, float maxSliderPos, const juce::Slider::SliderStyle, juce::Slider &) override;
    void drawLinearSliderBackground (juce::Graphics &, int x, int y, int width, int height, float sliderPos,
                                     float minSliderPos, float maxSliderPos, const juce::Slider::SliderStyle style, juce::Slider &) override;
    juce::Label* createSliderTextBox (juce::Slider &) override;
    void drawLabel (juce::Graphics &, juce::Label &) override;

private:
    Ap_dynamicsAudioProcessor& audioProcessor;

    juce::ImageConvolutionKernel kernel_ { 16 };
    juce::Image shadow_;

    juce::Font labelFont_ {juce::Typeface::createSystemTypefaceFor(BinaryData::VarelaRound_ttf,
                                                                BinaryData::VarelaRound_ttfSize)};
    SliderType sliderType_;

    int labelMargin_ = 70;
    float lastSliderPos_ = 0;
    float sliderWidth_ = 0;
};