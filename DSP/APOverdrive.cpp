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

void APOverdrive::process(const float* audioIn, float* audioOut, int numSamplesToRender)
{
  for (auto i = 0; i < numSamplesToRender; ++i)
  {
    const auto& sample = audioIn[i];
    float out          = 0;

    if (mix_ >= 0.0f && mix_ <= 0.3f)
    {  // No Clipping
      out = sample;
    }
    if (mix_ > 0.3f && mix_ < 0.34f)
    {
      out = mix_ * softClipping(sample) + (1 - mix_) * sample;
    }
    if (mix_ >= 0.34f && mix_ <= 0.6f)
    {  // Soft Clipping
      // Dirtier
      out = softClipping(sample);
      out = mix_ * out + (1.0f - mix_) * sample;
    }
    if (mix_ > 0.6f && mix_ < 0.64f)
    {
      out = mix_ * softClipping(sample) + (1.0f - mix_) * hardClipping(sample);
    }
    if (mix_ >= 0.64f && mix_ <= 1.0f)
    {  // Hard Clipping
      // Dirty
      out = hardClipping(sample);
      out = mix_ * out + (1.0f - mix_) * softClipping(sample);
    }
    audioOut[i] = out;
  }
}

float APOverdrive::softClipping(float sample)
{
  const auto alpha = 5.0f;
  return (2.0f / juce::float_Pi) * atan(alpha * sample);
}
float APOverdrive::hardClipping(float sample)
{
  auto xUni = abs(sample);
  float out = 0.0f;

  if (xUni <= 1.0f / 3.0f)
  {
    out = 2.0f * sample;
  }
  else if (xUni > 2.0f / 3.0f)
  {
    out = sin(sample);
  }
  else
  {
    out = sin(sample) * (3.0f - powf(2.0f - 3.0f * xUni, 2.0f)) * 0.33333f;
  }

  return out;
}
