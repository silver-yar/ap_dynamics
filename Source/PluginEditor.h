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


    void setupSlider (std::unique_ptr<CustomSlider>& slider, std::unique_ptr<juce::Label>& label,
                      const juce::String& name, bool showMeter, const juce::String& suffix = "s");
    void timerCallback() override;

private:
    Ap_dynamicsAudioProcessor& audioProcessor;

    juce::Image bgText_ {juce::ImageCache::getFromMemory(BinaryData::logotwo_png,
                                                         BinaryData::logotwo_pngSize)};
    juce::Font myFont_ {juce::Typeface::createSystemTypefaceFor(BinaryData::VarelaRound_ttf,
                                                                BinaryData::VarelaRound_ttfSize)};
    MyLookAndFeel apSliderLook_;

    MixerButton stylePicker_;
    std::unique_ptr<juce::ComboBox> styleSlider_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> styleAttachment_;
    std::unique_ptr<CustomSlider> thresholdSlider_, ratioSlider_, kneeSlider_, attackSlider_,
        releaseSlider_, makeupSlider_, toneSlider_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> thresholdAttachment_, 
        ratioAttachment_, kneeAttachment_, attackAttachment_, releaseAttachment_,
        makeupAttachment_, toneAttachment_;
    std::unique_ptr<juce::Label> thresholdLabel_, ratioLabel_, kneeLabel_, attackLabel_,
        releaseLabel_, makeupLabel_, toneLabel_, styleLabel_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Ap_dynamicsAudioProcessorEditor)
};
