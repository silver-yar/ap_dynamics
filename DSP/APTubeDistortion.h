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

  void updateParameters(float mix) { mix_ = mix; }

  // Based off DAFX 2nd edition pg. 123
  void process(const float* audioIn,
               float distGain,  // distortion amount
               float Q,         // work point
               float distChar,  // distortion character
               float* audioOut, int numSamplesToRender);

  void processDAFX(const float* audioIn, const float maxBufferVal,
                   float distGain,  // distortion amount
                   float Q,         // work point
                   float distChar,  // distortion character
                   float* audioOut, int numSamplesToRender);

 private:
  float mix_ = 0.0f;
};
