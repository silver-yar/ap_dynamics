/*
  ==============================================================================

    WaveformWindow.cpp
    Created: 2 Jan 2021 11:54:24pm
    Author:  Johnathan Handy

  ==============================================================================
*/

#include <JuceHeader.h>
#include "WaveformWindow.h"

//==============================================================================
WaveformWindow::WaveformWindow(Ap_dynamicsAudioProcessor& p) : audioProcessor(p)
{
}

WaveformWindow::~WaveformWindow()
{
}

void WaveformWindow::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::darkgrey.withAlpha(0.7f));
    drawWaveform (g);
    drawThreshold (g);
}

void WaveformWindow::resized()
{
}

void WaveformWindow::drawWaveform(juce::Graphics& g)
{
    auto rBounds = getLocalBounds().reduced(4);
    auto waveform = audioProcessor.getWaveForm();
    g.setColour (juce::Colours::snow);

    if (waveform.getNumSamples() > 0)
    {
        // Draw Waveform
        juce::Path p;
        audioPoints_.clear();

        auto ratio = waveform.getNumSamples() / rBounds.getWidth();
        auto buffer = waveform.getReadPointer (0);

        //scale audio file to window on x axis
        for (int sample = 0; sample < waveform.getNumSamples(); sample+=ratio)
        {
            audioPoints_.push_back (buffer[sample]);
        }

        p.startNewSubPath (0, getHeight() / 2);

        //scale on y axis
        for (auto sample = 0; sample < audioPoints_.size(); ++sample)
        {
            auto point = juce::jmap<float> (audioPoints_[sample], -1.25f, 1.25f, getHeight(), 0);
            p.lineTo (sample, point);
        }

        g.strokePath(p, juce::PathStrokeType (2));

    } else {
        g.drawLine (0, getHeight() / 2, rBounds.getWidth(), getHeight() / 2);
    }
}

void WaveformWindow::drawThreshold(juce::Graphics& g) {
    float threshold = juce::Decibels::decibelsToGain(
            audioProcessor.apvts.getRawParameterValue("THR")->load(),
            (float) audioProcessor.getMinDB());
    auto height = juce::jmap<float> (threshold, -1.25f, 1.25f, getHeight(), 0);
    g.setColour (juce::Colours::goldenrod);
    g.drawLine(0, height, getWidth(), height, 1.0f);
}
