/*
  ==============================================================================

    APCompressor.cpp
    Created: 26 Feb 2021 2:08:02pm
    Author:  Johnathan Handy

  ==============================================================================
*/

#include "APCompressor.h"

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

  auto x_uni = abs(sample);
  auto x_dB  = 20 * log10(x_uni);
  if (x_dB < MIN_DB)
    x_dB = MIN_DB;

  float gainSmooth = 0;
  float gain_sc    = 0;

  // Static Characteristics
  if (x_dB > (threshold_ + kneeWidth_ / 2))
    gain_sc = threshold_ + (x_dB - threshold_) / ratio_;  // Perform downwards compression
  else if (x_dB > (threshold_ - kneeWidth_ / 2))
    gain_sc = x_dB + ((1 / ratio_ - 1) * powf((x_dB - threshold_ + kneeWidth_ / 2), 2)) / (2 * kneeWidth_);
  else
    gain_sc = x_dB;

  const float gainChange_dB = gain_sc - x_dB;

  // Smooth gain change (RMS Approximation)
  if (gainChange_dB < prevGainSmooth_)
  {
    // attack mode
    gainSmooth = -sqrt(((1.0f - alphaA) * powf(gainChange_dB, 2.0f)) + (alphaA * powf(prevGainSmooth_, 2.0f)));
  }
  else
  {
    // release mode
    gainSmooth = -sqrt(((1.0f - alphaR) * powf(gainChange_dB, 2.0f)) + (alphaR * powf(prevGainSmooth_, 2.0f)));
  }

  // Convert back to linear amplitude scalar
  auto lin_a  = powf(10, gainSmooth / 20);
  float x_out = lin_a * sample;

  prevGainSmooth_ = gainSmooth;

  return x_out;
}
