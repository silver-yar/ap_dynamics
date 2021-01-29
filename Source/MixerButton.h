/*
  ==============================================================================

    MixerButton.h
    Created: 21 Jan 2021 7:10:13pm
    Author:  Johnathan Handy

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/*
*/
class MixerButton  : public juce::Component
{
public:
    MixerButton(Ap_dynamicsAudioProcessor&);
    ~MixerButton() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent&) override;
    void mouseDrag(const juce::MouseEvent&) override;

private:
    Ap_dynamicsAudioProcessor& audioProcessor;
    juce::Font labelFont_ {juce::Typeface::createSystemTypefaceFor(BinaryData::VarelaRound_ttf,
                                                                       BinaryData::VarelaRound_ttfSize)};
    juce::Point<int> pointerPos_ {100, 50};
    float mixValue_ = 0.0f;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MixerButton)
};
