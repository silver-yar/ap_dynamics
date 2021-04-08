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
        float out = 0;

        // TODO: Fix Overdrive incoming samples modified based on value via piecewise
        //  function (3 diff piecewise functions)
        if (mix >= 0.0f && mix <= 0.3f) {                               // No Clipping
            out = sample;
        }
        if (mix > 0.3f && mix < 0.34f) {
            out = mix * softClipping (sample) + (1 - mix) * sample;
        }
        if (mix >= 0.34f && mix <= 0.6f) {                              // Soft Clipping
            // Dirtier
            out = softClipping (sample);
            out = mix * out + (1 - mix) * sample;
        }
        if (mix > 0.6f && mix < 0.64f) {
            out = mix * softClipping (sample) + (1 - mix) * hardClipping(sample);
        }
        if (mix >= 0.64f && mix <= 1.0f) {                              // Hard Clipping
            // Dirty
            out = hardClipping (sample);
            out = mix * out + (1 - mix) * softClipping (sample);
        }
        audioOut[i] = out;
    }
}

float APOverdrive::softClipping(float sample)
{
    auto alpha = 5.0f;
    return (2 / juce::float_Pi) * atan (alpha * sample);
}
float APOverdrive::hardClipping(float sample)
{
    auto x_uni = abs (sample);
    float out;

    if (x_uni <= 1 / 3.0f)
    {
        out = 2 * sample;
    }
    else if (x_uni > 2 / 3.0f)
    {
        out = sin (sample);
    }
    else
    {
        out = sin(sample) * (3 - powf(2 - 3 * x_uni, 2)) / 3;;
    }

    return out;
}
