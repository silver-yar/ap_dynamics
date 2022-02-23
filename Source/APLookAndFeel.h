//
// Created by Johnathan Handy on 2/22/22.
//

#pragma once

#include "juce_gui_basics/juce_gui_basics.h"
#include "../Helpers/APDefines.h"

class MenuLookAndFeel : public juce::LookAndFeel_V4
{
  juce::Label* createSliderTextBox(juce::Slider&) override;
};

class MainSliderLookAndFeel : public juce::LookAndFeel_V4
{
 public:
  MainSliderLookAndFeel();

  void drawLinearSlider(juce::Graphics &, int x, int y, int width, int height, float sliderPos, float minSliderPos,
                        float maxSliderPos, juce::Slider::SliderStyle, juce::Slider &) override;
  juce::Label *createSliderTextBox(juce::Slider &) override;
  void drawLabel(juce::Graphics &, juce::Label &) override;
  std::function<juce::String()> getLabelText = nullptr;

 private:
  void initializeAssets();
  std::unique_ptr<juce::ImageConvolutionKernel> kernel_;
  std::unique_ptr<juce::Image> shadow_;

  int lastSliderPos_ = 0;
  int sliderWidth_   = 0;
};

