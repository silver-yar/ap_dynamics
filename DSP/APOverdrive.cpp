/*
  ==============================================================================

    APOverdrive.cpp
    Created: 26 Feb 2021 2:08:15pm
    Author:  Johnathan Handy

  ==============================================================================
*/

#include "APOverdrive.h"
#include "JuceHeader.h"

APOverdrive::APOverdrive()
{

}

APOverdrive::~APOverdrive()
{

}

void APOverdrive::process (float* audioIn,
              float mix,
              float* audioOut,
              int numSamplesToRender)
{
    for (auto i = 0; i < numSamplesToRender; ++i) {
        const auto& sample = audioIn[i];
        float x_uni = abs(sample);
        float out = 0;

        if (mix >= 0.0f && mix <= 0.3f) {
            // Clean
            out = sample;
        }
        if (mix > 0.3f && mix < 0.34f) {
            out = sample;
        }
        if (mix >= 0.34f && mix <= 0.6f) {
            // Dirtier
            out = sin(sample);
            out = mix * out + (1 - mix) * sample;
        }
        if (mix > 0.6f && mix < 0.64f) {
            out = sin(sample);
        }
        if (mix >= 0.64f && mix <= 1.0f) {
            // Dirty
            out = sin(sample) * (3 - powf(2 - 3 * x_uni, 2)) / 3;
            out = mix * out + (1 - mix) * sample;
        }
        audioOut[i] = out;
    }
}
