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
#include "ShowHideContainer.h"

//==============================================================================
/**
*/
const static int M_HEIGHT = 500;
const static int M_WIDTH = 700;
const static float FONT_HEIGHT = 24.0f;
const static float SHADOW_FONT_HEIGHT = 28.0f;

const static juce::Colour SHADOW_COLOR = juce::Colours::black.withAlpha(0.15f);
const static juce::Colour INNER_GRADIENT_BG = juce::Colour(0xFFFFD479);
const static juce::Colour OUTER_GRADIENT_BG = juce::Colour(0xFFFFC446);

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
                      std::unique_ptr<juce::Label>& labelShadow,
                      const juce::String& name, SliderType sliderType,
                      const juce::String& suffix = "s");
    void timerCallback() override;

private:
    Ap_dynamicsAudioProcessor& audioProcessor;

    juce::Image bgText_ {juce::ImageCache::getFromMemory(BinaryData::logo_clean_png,
                                                         BinaryData::logo_clean_pngSize)};
    juce::Image textShadow_ {juce::ImageCache::getFromMemory(BinaryData::shadow_png,
                                                            BinaryData::shadow_pngSize)};

    juce::Font myFont_ {juce::Typeface::createSystemTypefaceFor(BinaryData::VarelaRound_ttf,
                                                                BinaryData::VarelaRound_ttfSize)};
    MyLookAndFeel thresholdLook_, ratioLook_;

    MixerButton stylePicker_;
    std::unique_ptr<CustomSlider_> thresholdSlider_, ratioSlider_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> thresholdAttachment_, ratioAttachment_;
    std::unique_ptr<juce::Label> thresholdLabel_, ratioLabel_, styleLabel_;
    std::unique_ptr<juce::Label> lshdwT_, lshdwR_, lshdwS_;

//    juce::Point<float> offset_ {0, -10};
//    int shadowDeltaXY_ = 5;
    const int sliderHeight_ = 185;
    juce::Rectangle<int> thresholdBounds_, ratioBounds_, pickerBounds_;
    ShowHideContainer testBox_;
//    juce::TextEditor xOffset_, yOffset_, deltaEditor_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Ap_dynamicsAudioProcessorEditor)
};
