//
// Created by Johnathan Handy on 12/20/21.
//

#pragma once
#include "PluginProcessor.h"
#include "juce_audio_processors/juce_audio_processors.h"
#include "juce_gui_basics/juce_gui_basics.h"

class APParameterMenu : public juce::Viewport
{
 public:
  APParameterMenu(juce::AudioProcessor&, juce::AudioProcessorValueTreeState&);
  ~APParameterMenu() override;

  void paint(juce::Graphics& g) override;
  void resized() override;
  void setMenuWidth(int w) { parameterGrid_->width = w; }
  void setBackgroundImage(const juce::Image& backgroundImage);
  void initializeAssets();
  void setParameterFilter(const std::function<bool(juce::AudioProcessorParameter*)>& newFilter) {
    if (newFilter == nullptr)
      return;
    parameterGrid_->parameterFilter = newFilter;
  }

  class ParameterGrid : public juce::Component
  {
   public:
    ParameterGrid(juce::AudioProcessor& p, juce::AudioProcessorValueTreeState& s) : audioProcessor_(p), apvts_(s)
    {
      initializeAssets();
    }
    ~ParameterGrid() override = default;

    struct SliderObject
    {
      std::unique_ptr<juce::Slider> slider                                                   = nullptr;
      std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sliderAttachment = nullptr;
      std::unique_ptr<juce::Label> label                                                     = nullptr;
    };

    void paint(juce::Graphics& g) override;
    void resized() override;
    void initializeAssets();


    std::function<bool(juce::AudioProcessorParameter* parameter)> parameterFilter = [](auto*) { return true; };
    int width = 0;
   private:
    juce::AudioProcessor& audioProcessor_;
    juce::AudioProcessorValueTreeState& apvts_;

    std::vector<SliderObject> sliders_;
  };


 private:
  juce::AudioProcessor& audioProcessor_;
  juce::AudioProcessorValueTreeState& apvts_;

  std::unique_ptr<juce::DrawableImage> closeIcon_     = nullptr;
  std::unique_ptr<juce::DrawableImage> closeIconOver_ = nullptr;
  std::unique_ptr<juce::DrawableButton> closeButton_  = nullptr;
  std::unique_ptr<ParameterGrid> parameterGrid_       = nullptr;
  std::unique_ptr<juce::Image> backgroundImage_       = nullptr;
};
