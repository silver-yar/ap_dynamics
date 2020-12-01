/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "APPlot.h"

//==============================================================================
/**
*/
class Ap_dynamicsAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    Ap_dynamicsAudioProcessorEditor (Ap_dynamicsAudioProcessor&);
    ~Ap_dynamicsAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void setupSlider (std::unique_ptr<juce::Slider>& slider, std::unique_ptr<juce::Label>& label,
                      const juce::String& name, const juce::String& suffix = "s");

private:
    Ap_dynamicsAudioProcessor& audioProcessor;

    std::unique_ptr<juce::ComboBox> dynType_;
    std::unique_ptr<APPlot> plot_;
    std::unique_ptr<juce::Slider> thresholdSlider_, ratioSlider_, kneeSlider_, attackSlider_, releaseSlider_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> thresholdAttachment_, 
    ratioAttachment_, kneeAttachment_, attackAttachment_, releaseAttachment_;
    std::unique_ptr<juce::Label> thresholdLabel_, ratioLabel_, kneeLabel_, attackLabel_, releaseLabel_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Ap_dynamicsAudioProcessorEditor)
};
