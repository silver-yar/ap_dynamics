//
// Created by Johnathan Handy on 12/20/21.
//

#pragma once
#include "juce_gui_basics/juce_gui_basics.h"
#include "juce_audio_processors/juce_audio_processors.h"
#include "PluginProcessor.h"


class ParameterGrid : public juce::Component
{
 public:
  ParameterGrid(juce::AudioProcessor&, juce::AudioProcessorValueTreeState&);
  ~ParameterGrid();

  struct SliderObject {
    std::unique_ptr<juce::Slider> slider = nullptr;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sliderAttachment = nullptr;
    std::unique_ptr<juce::Label> label = nullptr;
  };

  void paint(juce::Graphics& g) override;
  void resized() override;
  void initializeAssets();

 private:
  juce::AudioProcessor& audioProcessor_;
  juce::AudioProcessorValueTreeState& apvts_;

  std::vector<SliderObject> sliders_;
};

class APParameterMenu : public juce::Component
{
 public:
  APParameterMenu(juce::AudioProcessor&, juce::AudioProcessorValueTreeState&);
  ~APParameterMenu();

  void paint(juce::Graphics& g) override;
  void resized() override;
  void initializeAssets();
 private:
  juce::AudioProcessor& audioProcessor_;
  juce::AudioProcessorValueTreeState& apvts_;

  std::unique_ptr<juce::DrawableImage> closeIcon_;
  std::unique_ptr<juce::DrawableImage> closeIconOver_;
  std::unique_ptr<juce::DrawableButton> closeButton_;
  std::unique_ptr<ParameterGrid> parameterGrid_;
};

