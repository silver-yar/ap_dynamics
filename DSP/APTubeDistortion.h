/*
  ==============================================================================

    APTubeDistortion.h
    Created: 2 Mar 2021 1:42:57pm
    Author:  Johnathan Handy

  ==============================================================================
*/

#pragma once

class APTubeDistortion
{
 public:
  APTubeDistortion();
  ~APTubeDistortion();

  // Based off DAFX 2nd edition pg. 123
  static void process(const float* audioIn,
                      float distGain,  // distortion amount
                      float Q,         // work point
                      float distChar,  // distortion character
                      float mix,       // mix of original and distorted sample
                      float* audioOut, int numSamplesToRender);

  static void processDAFX(const float* audioIn, const float maxBufferVal,
                          float distGain,  // distortion amount
                          float Q,         // work point
                          float distChar,  // distortion character
                          float mix,       // mix of original and distorted sample
                          float* audioOut, int numSamplesToRender);

 private:
};
