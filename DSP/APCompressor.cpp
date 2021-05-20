/*
  ==============================================================================

    APCompressor.cpp
    Created: 26 Feb 2021 2:08:02pm
    Author:  Johnathan Handy

  ==============================================================================
*/

#include "APCompressor.h"

#include "../Source/APDefines.h"
#include "JuceHeader.h"

APCompressor::APCompressor() = default;

APCompressor::~APCompressor() = default;

void APCompressor::process(float* audioIn, float* audioOut, float numSamplesToRender)
{
  for (int i = 0; i < numSamplesToRender; ++i)
  {
    audioOut[i] = applyRMSCompression(audioIn[i]);
  }
}

float APCompressor::applyRMSCompression(float sample)
{
  const float alphaA = exp(-log(9) / (sampleRate_ * attack_));
  const float alphaR = exp(-log(9) / (sampleRate_ * release_));

  auto xUni = abs(sample);
  auto xDb  = juce::Decibels::gainToDecibels(xUni, AP::Constants::minusInfinityDb);
  if (xDb < MIN_DB)
    xDb = MIN_DB;

  float gainSmooth = 0;
  float gainSc     = 0;

  // Static Characteristics
  if (xDb > (threshold_ + kneeWidth_ / 2))
    gainSc = threshold_ + (xDb - threshold_) / ratio_;  // Perform downwards compression
  else if (xDb > (threshold_ - kneeWidth_ / 2))
    gainSc = xDb + ((1 / ratio_ - 1) * powf((xDb - threshold_ + kneeWidth_ / 2), 2)) / (2 * kneeWidth_);
  else
    gainSc = xDb;

  const float gainChangeDb = gainSc - xDb;

  // Smooth gain change (RMS Approximation)
  if (gainChangeDb < prevGainSmooth_)
  {
    // attack mode
    gainSmooth = -sqrt(((1.0f - alphaA) * powf(gainChangeDb, 2.0f)) + (alphaA * powf(prevGainSmooth_, 2.0f)));
  }
  else
  {
    // release mode
    gainSmooth = -sqrt(((1.0f - alphaR) * powf(gainChangeDb, 2.0f)) + (alphaR * powf(prevGainSmooth_, 2.0f)));
  }

  // Convert back to linear amplitude scalar
  const auto linA = powf(10, gainSmooth / 20);
  float xOut      = linA * sample;

  prevGainSmooth_ = gainSmooth;

  return xOut;
}
