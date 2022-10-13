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
class MixerButton : public juce::Component
{
 public:
  explicit MixerButton(Ap_dynamicsAudioProcessor&);
  ~MixerButton() override;

  void paint(juce::Graphics&) override;
  void resized() override;
  void mouseDown(const juce::MouseEvent&) override;
  void mouseDrag(const juce::MouseEvent&) override;
  void mouseUp(const juce::MouseEvent&) override;

 private:
  void mapMouseToValue(const juce::Point<int>&);

  Ap_dynamicsAudioProcessor& audioProcessor_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MixerButton)
};
