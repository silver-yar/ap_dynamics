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
class Ap_dynamicsAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                         public juce::Timer
{
public:
    Ap_dynamicsAudioProcessorEditor (Ap_dynamicsAudioProcessor&);
    ~Ap_dynamicsAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;


    void setupSlider (std::unique_ptr<juce::Slider>& slider, std::unique_ptr<juce::Label>& label,
                      const juce::String& name, const juce::String& suffix = "s");
    void timerCallback() override;

private:
    Ap_dynamicsAudioProcessor& audioProcessor;

    juce::Image logo_ {juce::ImageCache::getFromMemory(BinaryData::ap_logo_groovees_png,
                                          BinaryData::ap_logo_groovees_pngSize)};

    std::unique_ptr<juce::ComboBox> dynType_, compType_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> dynAttachment_, compAttachment_;

    std::unique_ptr<APPlot> plot_;

    std::unique_ptr<juce::Slider> thresholdSlider_, ratioSlider_, kneeSlider_, attackSlider_,
        releaseSlider_, makeupSlider_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> thresholdAttachment_, 
        ratioAttachment_, kneeAttachment_, attackAttachment_, releaseAttachment_, makeupAttachment_;
    std::unique_ptr<juce::Label> thresholdLabel_, ratioLabel_, kneeLabel_, attackLabel_,
        releaseLabel_, makeupLabel_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Ap_dynamicsAudioProcessorEditor)
};
