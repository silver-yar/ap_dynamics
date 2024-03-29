/*
  ==============================================================================

    APOverdrive.h
    Created: 26 Feb 2021 2:08:15pm
    Author:  Johnathan Handy

  ==============================================================================
*/

#pragma once

class APOverdrive
{
 public:
  APOverdrive();
  ~APOverdrive();

  void updateParameters(const float mix) { mix_ = mix; }

  void process(const float* audioIn, float* audioOut, int numSamplesToRender) const;

  static float softClipping(float sample);
  static float hardClipping(float sample);

 private:
  float mix_ = 0.0f;
};
