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
      return softClipping(sample);
    }();
    audioOut[i] = out;
  }
}

void APOverdrive::dynamicProcess(const float* audioIn, float* audioOut, int numSamplesToRender, float mix) const
{
  for (auto i = 0; i < numSamplesToRender; ++i)
  {
    const auto& sample = audioIn[i];
    const auto out     = [&]()
    {
      if (mix > 0.8f)
        return hardClipping(sample);
      return softClipping(sample);
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
