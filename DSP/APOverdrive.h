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

  void process(float* audioIn, float mix, float* audioOut, int numSamplesToRender);

  float softClipping(float sample);
  float hardClipping(float sample);

 private:
};
