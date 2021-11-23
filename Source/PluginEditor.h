/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include "APDefines.h"
#include "APSlider.h"
#include "MixerButton.h"
#include "PluginProcessor.h"

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
  void setupSlider(std::unique_ptr<APSlider>& slider, std::unique_ptr<juce::Label>& label,
                   const juce::String& name, SliderType sliderType,
                   const juce::String& suffix = "s");
  void timerCallback() override;

 private:
  Ap_dynamicsAudioProcessor& audioProcessor;

  juce::ImageConvolutionKernel kernel_{ 16 };

  juce::Image thresholdShadow_, ratioShadow_, styleShadow_;
  juce::Image tSliderShadow_, rSliderShadow_, sSliderShadow_;

  juce::Image bgText_{ juce::ImageCache::getFromMemory(BinaryData::logo_clean_png, BinaryData::logo_clean_pngSize) };
  juce::Image textShadow_{ juce::ImageCache::getFromMemory(BinaryData::shadow_png, BinaryData::shadow_pngSize) };

  juce::Font myFont_{ juce::Typeface::createSystemTypefaceFor(BinaryData::VarelaRound_ttf,
                                                              BinaryData::VarelaRound_ttfSize) };
  MyLookAndFeel thresholdLook_, ratioLook_;

  MixerButton stylePicker_;
  std::unique_ptr<APSlider> thresholdSlider_, ratioSlider_;
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> thresholdAttachment_, ratioAttachment_;
  std::unique_ptr<juce::Label> thresholdLabel_, ratioLabel_, styleLabel_;
  std::unique_ptr<juce::Label> lshdwT_, lshdwR_, lshdwS_;

  juce::Point<float> offset_{ 0.0f, -16.0f };
  int shadowDeltaXY_      = 8;
  const int sliderHeight_ = 185;
  juce::Rectangle<int> thresholdBounds_, ratioBounds_, pickerBounds_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Ap_dynamicsAudioProcessorEditor)
};
