/*
  ==============================================================================

    APCompressor.cpp
    Created: 26 Feb 2021 2:08:02pm
    Author:  Johnathan Handy

  ==============================================================================
*/

#include "APCompressor.h"

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_core/juce_core.h>
#include "APDefines.h"



APCompressor::APCompressor() = default;

APCompressor::~APCompressor() = default;

void APCompressor::process(const float* audioIn, float* audioOut, const int numSamplesToRender)
{
  for (int i = 0; i < numSamplesToRender; ++i)
  {
    audioOut[i] = applyRMSCompression(audioIn[i]);
  }
}

std::pair<float, float> APCompressor::_applyRMSCompression(const float sample, const float sampleRate, const float threshold,
                                                           const float ratio, const float attack, const float release,
                                                           const float kneeWidth, const float prevGainSmoothed)
{
  const auto alphaA = static_cast<float>(exp(-log(9) / (sampleRate * attack)));
  const auto alphaR = static_cast<float>(exp(-log(9) / (sampleRate * release)));

  const auto xUni = abs(sample);
  auto xDb        = juce::Decibels::gainToDecibels(xUni, APConstants::Math::MINUS_INF_DB);
  if (xDb < APConstants::Math::MINUS_INF_DB)
    xDb = APConstants::Math::MINUS_INF_DB;

  auto gainSmooth = 0.0f;
  auto gainSc     = 0.0f;

  // Static Characteristics
  if (xDb > (threshold + kneeWidth / 2))
    gainSc = threshold + (xDb - threshold) / ratio;  // Perform downwards compression
  else if (xDb > (threshold - kneeWidth / 2))
    gainSc = xDb + ((1 / ratio - 1) * powf((xDb - threshold + kneeWidth / 2), 2)) / (2 * kneeWidth);
  else
    gainSc = xDb;

  const auto gainChangeDb = gainSc - xDb;

  // Smooth gain change (RMS Approximation)
  if (gainChangeDb < prevGainSmoothed)
  {
    // attack mode
    gainSmooth = -sqrt(((1.0f - alphaA) * powf(gainChangeDb, 2.0f)) + (alphaA * powf(prevGainSmoothed, 2.0f)));
  }
  else
  {
    // release mode
    gainSmooth = -sqrt(((1.0f - alphaR) * powf(gainChangeDb, 2.0f)) + (alphaR * powf(prevGainSmoothed, 2.0f)));
  }

  // Convert back to linear amplitude scalar
  const auto linA = powf(10, gainSmooth / 20);
  const auto xOut = linA * sample;

  return { xOut, gainSmooth };
}

float APCompressor::applyRMSCompression(const float sample)
{
  const auto [result, gainSmoothed] =
      _applyRMSCompression(sample, sampleRate_, threshold_, ratio_, attack_, release_, kneeWidth_, prevGainSmooth_);
  prevGainSmooth_ = gainSmoothed;
  return result;
}
