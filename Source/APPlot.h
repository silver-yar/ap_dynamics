/*
  ==============================================================================

    APPlot.h
    Created: 30 Nov 2020 7:31:17pm
    Author:  Johnathan Handy

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/*
*/
class APPlot  : public juce::Component
{
public:
    APPlot(Ap_dynamicsAudioProcessor&);
    ~APPlot() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void drawGrid (juce::Graphics&, juce::Rectangle<int>);
    void drawXAxis (juce::Graphics&, juce::Rectangle<int>);
    void drawYAxis (juce::Graphics&, juce::Rectangle<int>);
    void drawPlot (juce::Graphics&, juce::Rectangle<int>);

private:
    Ap_dynamicsAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (APPlot)
};
