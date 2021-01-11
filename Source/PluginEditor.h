/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "APPlot.h"
#include "ButtonMenu.h"
#include "APSliderLook.h"
#include "WaveformWindow.h"

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

    juce::Image bgText_ {juce::ImageCache::getFromMemory(BinaryData::liq_dyn_text_png,
                                                         BinaryData::liq_dyn_text_pngSize)};
    juce::Font myFont_ {juce::Typeface::createSystemTypefaceFor(BinaryData::Antipasto_Med_ttf,
                                                                BinaryData::Antipasto_Med_ttfSize)};
    APSliderLook apSliderLook_;

    std::unique_ptr<ButtonMenu> buttonMenu_;
    std::unique_ptr<WaveformWindow> waveformWindow_;
    std::unique_ptr<APPlot> plot_;

    std::unique_ptr<juce::Slider> thresholdSlider_, ratioSlider_, kneeSlider_, attackSlider_,
        releaseSlider_, makeupSlider_, toneSlider_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> thresholdAttachment_, 
        ratioAttachment_, kneeAttachment_, attackAttachment_, releaseAttachment_,
        makeupAttachment_, toneAttachment_;
    std::unique_ptr<juce::Label> thresholdLabel_, ratioLabel_, kneeLabel_, attackLabel_,
        releaseLabel_, makeupLabel_, toneLabel_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Ap_dynamicsAudioProcessorEditor)
};
