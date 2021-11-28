/*
  ==============================================================================

    CustomSlider.h
    Created: 20 Jan 2021 4:38:04pm
    Author:  Johnathan Handy

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include "./OpenGL/SliderBarGL.h"
#include "APDefines.h"
#include "PluginProcessor.h"

enum SliderType
{
  Normal = 1,
  Invert = 2
};

class MyLookAndFeel : public juce::LookAndFeel_V4
{
 public:
  explicit MyLookAndFeel(SliderType);

  void drawLinearSlider(juce::Graphics &, int x, int y, int width, int height, float sliderPos, float minSliderPos,
                        float maxSliderPos, juce::Slider::SliderStyle, juce::Slider &) override;
  juce::Label *createSliderTextBox(juce::Slider &) override;
  void drawLabel(juce::Graphics &, juce::Label &) override;

 private:
  void initializeAssets();
  std::unique_ptr<juce::ImageConvolutionKernel> kernel_;
  std::unique_ptr<juce::Image> shadow_;

  SliderType sliderType_;

  int lastSliderPos_ = 0;
  int sliderWidth_   = 0;
};

class APSlider : public juce::Component, public juce::Timer
{
 public:
  APSlider(Ap_dynamicsAudioProcessor &, SliderType);
  ~APSlider() override;

  void resized() override;
  void timerCallback() override;

  juce::Slider slider;

 private:
  Ap_dynamicsAudioProcessor &audioProcessor;
  SliderType sliderType_;
  std::unique_ptr<SliderBarGL> sliderBarGl_;
  std::unique_ptr<MyLookAndFeel> lookAndFeel_;
};
