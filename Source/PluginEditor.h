/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "APSliderLook.h"
#include "CustomSlider.h"
#include "MixerButton.h"

//==============================================================================
/**
*/
class Ap_dynamicsAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                         public juce::Timer
{
public:
    Ap_dynamicsAudioProcessorEditor (Ap_dynamicsAudioProcessor&);
    ~Ap_dynamicsAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;


    void setupSlider (std::unique_ptr<CustomSlider_>& slider, std::unique_ptr<juce::Label>& label,
                      const juce::String& name, SliderType sliderType,
                      const juce::String& suffix = "s");
    void timerCallback() override;

private:
    Ap_dynamicsAudioProcessor& audioProcessor;

    juce::Image bgText_ {juce::ImageCache::getFromMemory(BinaryData::logo3_png,
                                                         BinaryData::logo3_pngSize)};

    juce::Font myFont_ {juce::Typeface::createSystemTypefaceFor(BinaryData::VarelaRound_ttf,
                                                                BinaryData::VarelaRound_ttfSize)};
    MyLookAndFeel thresholdLook_, ratioLook_;

    MixerButton stylePicker_;
    std::unique_ptr<CustomSlider_> thresholdSlider_, ratioSlider_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> thresholdAttachment_, ratioAttachment_;
    std::unique_ptr<juce::Label> thresholdLabel_, ratioLabel_, styleLabel_;

    const int sliderHeight_ = 185;
    juce::Rectangle<int> thresholdBounds_, ratioBounds_, pickerBounds_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Ap_dynamicsAudioProcessorEditor)
};
