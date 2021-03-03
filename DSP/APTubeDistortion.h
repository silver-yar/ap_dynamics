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
    void process (float* audioIn,
                  float distGain, // distortion amount
                  float Q, // work point
                  float distChar, // distortion character
                  float mix, // mix of original and distorted sample
                  float* audioOut,
                  float numSamplesToRender);
private:
};
