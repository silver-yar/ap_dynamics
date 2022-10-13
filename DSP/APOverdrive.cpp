/*
  ==============================================================================

    APOverdrive.cpp
    Created: 26 Feb 2021 2:08:15pm
    Author:  Johnathan Handy

  ==============================================================================
*/

#include "APOverdrive.h"

#include <juce_core/juce_core.h>

APOverdrive::APOverdrive() = default;

APOverdrive::~APOverdrive() = default;

void APOverdrive::process(const float* audioIn, float* audioOut, const int numSamplesToRender) const
{
  for (auto i = 0; i < numSamplesToRender; ++i)
  {
    const auto& sample = audioIn[i];
    const auto out     = [&]()
    {
      float result = 0.0f;
      if (mix_ >= 0.0f && mix_ <= 0.3f)
      {  // No Clipping
        result = sample;
      }
      if (mix_ > 0.3f && mix_ < 0.34f)
      {
        result = mix_ * softClipping(sample) + (1 - mix_) * sample;
      }
      if (mix_ >= 0.34f && mix_ <= 0.6f)
      {  // Soft Clipping
        // Dirtier
        result = softClipping(sample);
        result = mix_ * result + (1.0f - mix_) * sample;
      }
      if (mix_ > 0.6f && mix_ < 0.64f)
      {
        result = mix_ * softClipping(sample) + (1.0f - mix_) * hardClipping(sample);
      }
      if (mix_ >= 0.64f && mix_ <= 1.0f)
      {  // Hard Clipping
        // Dirty
        result = hardClipping(sample);
        result = mix_ * result + (1.0f - mix_) * softClipping(sample);
      }
      return result;
    }();
    audioOut[i] = out;
  }
}

float APOverdrive::softClipping(const float sample)
{
  const auto alpha = 5.0f;
  return (2.0f / juce::float_Pi) * atan(alpha * sample);
}
float APOverdrive::hardClipping(const float sample)
{
  const auto xUni = abs(sample);
  const auto out  = [&]()
  {
    float result;
    if (xUni <= 1.0f / 3.0f)
    {
      result = 2.0f * sample;
    }
    else if (xUni > 2.0f / 3.0f)
    {
      result = sin(sample);
    }
    else
    {
      result = sin(sample) * (3.0f - powf(2.0f - 3.0f * xUni, 2.0f)) * 0.33333f;
    }
    return result;
  }();

  return out;
}
