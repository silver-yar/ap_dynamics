/*
  ==============================================================================

    APTubeDistortion.h
    Created: 2 Mar 2021 1:42:57pm
    Author:  Johnathan Handy

  ==============================================================================
*/

#pragma once
#include "juce_core/juce_core.h"
#include "juce_dsp/juce_dsp.h"

class APTubeDistortion
{
 public:
  APTubeDistortion();
  ~APTubeDistortion();

  // Based off DAFX 2nd edition pg. 123
  void process(const float* audioIn, float minBufferVal, float maxBufferVal,
                   float distGain,  // distortion amount
                   float Q,         // work point, more negative = more linear
                   float distChar,  // distortion character, higher = harder, >0
                   float* audioOut, int numSamplesToRender) const;

 private:
};
