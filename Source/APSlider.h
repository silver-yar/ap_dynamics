/*
  ==============================================================================

    CustomSlider.h
    Created: 20 Jan 2021 4:38:04pm
    Author:  Johnathan Handy

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include "../Helpers/APDefines.h"
#include "./OpenGL/SliderBarGL.h"
#include "PluginProcessor.h"
#include "APLookAndFeel.h"

enum SliderType
{
  Normal = 1,
  Invert = 2
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
  std::unique_ptr<MainSliderLookAndFeel> lookAndFeel_;
};
