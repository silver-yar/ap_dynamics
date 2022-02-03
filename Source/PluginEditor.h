/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include "../Helpers/APDefines.h"
#include "APSlider.h"
#include "MixerButton.h"
#include "PluginProcessor.h"
#include "APParameterMenu.h"

//==============================================================================

class Ap_dynamicsAudioProcessorEditor : public juce::AudioProcessorEditor, public juce::Timer
{
 public:
  explicit Ap_dynamicsAudioProcessorEditor(Ap_dynamicsAudioProcessor&);
  ~Ap_dynamicsAudioProcessorEditor() override;

  //==============================================================================
  void paint(juce::Graphics&) override;
  void resized() override;

  void setupLabelShadow(juce::Image& shadow, const juce::String& name);
  void setupSliderShadow(juce::Image& shadow);
  void setupSlider(std::unique_ptr<APSlider>& apSlider, std::unique_ptr<juce::Label>& label, const juce::String& name,
                   SliderType sliderType, const String& suffix = "s");
  void timerCallback() override;

 private:
  void initializeAssets();

  Ap_dynamicsAudioProcessor& audioProcessor_;

  // Parameter Menu
  std::unique_ptr<juce::DrawableImage> pButtonDef_, pButtonOver_;
  std::unique_ptr<juce::DrawableButton> parameterButton_;
  std::unique_ptr<APParameterMenu> parameterMenu_;

  std::unique_ptr<juce::ImageConvolutionKernel> kernel_;
  std::unique_ptr<juce::Image> bgText_;

  // Shadows
  std::unique_ptr<juce::Image> pButtonShadow_;
  std::unique_ptr<juce::Image> thresholdShadow_, ratioShadow_, styleShadow_;
  std::unique_ptr<juce::Image> tSliderShadow_, rSliderShadow_, sSliderShadow_;
  std::unique_ptr<juce::Image> textShadow_;

  // Parameter Components
  MixerButton stylePicker_;
  std::unique_ptr<APSlider> thresholdSlider_, ratioSlider_;
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> thresholdAttachment_,
      ratioAttachment_;  // Implemented @ initialization
  std::unique_ptr<juce::Label> thresholdLabel_, ratioLabel_, styleLabel_;

  const juce::Point<float> offset_{ 0.0f, -16.0f };
  const float shadowDeltaXY_      = 8.0f;
  const int sliderHeight_ = 185;
  bool isMenuShown_ = false;
  juce::Rectangle<int> thresholdBounds_, ratioBounds_, pickerBounds_, pMenuBounds_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Ap_dynamicsAudioProcessorEditor)
};
