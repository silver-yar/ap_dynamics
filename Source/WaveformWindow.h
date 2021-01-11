/*
  ==============================================================================

    WaveformWindow.h
    Created: 2 Jan 2021 11:54:24pm
    Author:  Johnathan Handy

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/*
*/
class WaveformWindow  : public juce::Component
{
public:
    WaveformWindow(Ap_dynamicsAudioProcessor&);
    ~WaveformWindow() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void drawWaveform (juce::Graphics&);
    void drawThreshold (juce::Graphics&);

private:
    Ap_dynamicsAudioProcessor& audioProcessor;
    std::vector<float> audioPoints_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WaveformWindow)
};
