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


APCompressor::APCompressor() = default;

APCompressor::~APCompressor() = default;

void APCompressor::process(const float* audioIn, float* audioOut, int numSamplesToRender)
{
  for (int i = 0; i < numSamplesToRender; ++i)
  {
    audioOut[i] = applyRMSCompression(audioIn[i]);
  }
}

std::pair<float, float> APCompressor::_applyRMSCompression(float sample, float sampleRate, float threshold, float ratio,
                                  float attack, float release, float kneeWidth,
                                  float prevGainSmoothed)
{
  auto alphaA = static_cast<const float>(exp(-log(9) / (sampleRate * attack)));
  auto alphaR = static_cast<const float>(exp(-log(9) / (sampleRate * release)));

  auto xUni = abs(sample);
  auto xDb  = juce::Decibels::gainToDecibels(xUni, minusInfinityDb);
  if (xDb < minusInfinityDb)
    xDb = minusInfinityDb;

  float gainSmooth = 0;
  float gainSc     = 0;

  // Static Characteristics
  if (xDb > (threshold + kneeWidth / 2))
    gainSc = threshold + (xDb - threshold) / ratio;  // Perform downwards compression
  else if (xDb > (threshold - kneeWidth / 2))
    gainSc = xDb + ((1 / ratio - 1) * powf((xDb - threshold + kneeWidth / 2), 2)) / (2 * kneeWidth);
  else
    gainSc = xDb;

  const float gainChangeDb = gainSc - xDb;

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
  float xOut      = linA * sample;

  return {xOut, gainSmooth};
}

float APCompressor::applyRMSCompression(float sample)
{
  auto [result, gainSmoothed] = _applyRMSCompression(sample, sampleRate_, threshold_, ratio_,
                                                     attack_, release_, kneeWidth_, prevGainSmooth_);
  prevGainSmooth_ = gainSmoothed;
  return result;
}
