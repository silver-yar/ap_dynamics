/*
  ==============================================================================

    APCompressor.cpp
    Created: 26 Feb 2021 2:08:02pm
    Author:  Johnathan Handy

  ==============================================================================
*/

#include "APCompressor.h"
#include "JuceHeader.h"

APCompressor::APCompressor()
{

}

APCompressor::~APCompressor()
{

}

void APCompressor::process (float* audioIn,
                            float* audioOut,
                            float numSamplesToRender)
{
    for (auto i = 0; i < numSamplesToRender; ++i)
    {
        audioOut[i] = applyRMSCompression (audioIn[i]);
    }
}

float APCompressor::applyFFCompression (float sample)
{
    const auto alphaA = exp(-log(9) / (sampleRate_ * attack_));
    const auto alphaR = exp(-log(9) / (sampleRate_ * release_));

    float gainSmooth = 0;
    float gain_sc = 0;

    auto x_uni = abs(sample);
    auto x_dB = 20 * log10(x_uni);
    if (x_dB < MIN_DB)
        x_dB = MIN_DB;
    // Static Characteristics
    if (x_dB > (threshold_ + kneeWidth_ / 2))
        gain_sc = threshold_ + (x_dB - threshold_) / ratio_; // Perform downwards compression
    else if (x_dB > (threshold_ - kneeWidth_ / 2))
        gain_sc = x_dB + ((1 / ratio_ - 1) * powf((x_dB - threshold_ + kneeWidth_ / 2), 2)) / (2 * kneeWidth_);
    else
        gain_sc = x_dB;

    float gainChange_dB = gain_sc - x_dB;

    // Smooth gain change
    if (gainChange_dB < prevGainSmooth_) {
        // attack mode
        gainSmooth = ((1 - alphaA) * gainChange_dB) + (alphaA * prevGainSmooth_);
    } else {
        // release mode
        gainSmooth = ((1 - alphaR) * gainChange_dB) + (alphaR * prevGainSmooth_);
    }

    // Convert back to linear amplitude scalar
    auto lin_a = powf(10, gainSmooth / 20);
    float x_out = lin_a * sample;

    prevGainSmooth_ = gainSmooth;

    return x_out;
}

float APCompressor::applyFBCompression (float sample)
{
    const auto alphaA = exp(-log(9) / (sampleRate_ * attack_));
    const auto alphaR = exp(-log(9) / (sampleRate_ * release_));

    float gainSmooth = 0;
    float gain_sc = 0;

    auto y_uni = abs(y_prev_);
    auto y_dB = 20 * log10(y_uni);
    if (y_dB < MIN_DB)
        y_dB = MIN_DB;
    // Static Characteristics
    if (y_dB > (threshold_ + kneeWidth_ / 2))
        gain_sc = threshold_ + (y_dB - threshold_) / ratio_; // Perform downwards compression
    else if (y_dB > (threshold_ - kneeWidth_ / 2))
        gain_sc = y_dB + ((1 / ratio_ - 1) * powf((y_dB - threshold_ + kneeWidth_ / 2), 2)) / (2 * kneeWidth_); // Compression with ramp
    else
        gain_sc = y_dB;

    float gainChange_dB = gain_sc - y_dB;

    // Smooth gain change
    if (gainChange_dB < prevGainSmooth_) {
        // attack mode
        gainSmooth = ((1 - alphaA) * gainChange_dB) + (alphaA * prevGainSmooth_);
    } else {
        // release mode
        gainSmooth = ((1 - alphaR) * gainChange_dB) + (alphaR * prevGainSmooth_);
    }

    // Convert back to linear amplitude scalar
    auto lin_a = powf(10, gainSmooth / 20);
    float y_out = lin_a * sample;

    y_prev_ = y_out;
    prevGainSmooth_ = gainSmooth;

    return y_out;
}

float APCompressor::applyRMSCompression (float sample)
{
    const auto alphaA = exp(-log(9) / (sampleRate_ * attack_));
    const auto alphaR = exp(-log(9) / (sampleRate_ * release_));

    float gainSmooth = 0;
    float gain_sc = 0;

    auto x_uni = abs(sample);
    auto x_dB = 20 * log10(x_uni);
    if (x_dB < MIN_DB)
        x_dB = MIN_DB;
    // Static Characteristics
    if (x_dB > (threshold_ + kneeWidth_ / 2))
        gain_sc = threshold_ + (x_dB - threshold_) / ratio_; // Perform downwards compression
    else if (x_dB > (threshold_ - kneeWidth_ / 2))
        gain_sc = x_dB + ((1 / ratio_ - 1) * powf((x_dB - threshold_ + kneeWidth_ / 2), 2)) / (2 * kneeWidth_);
    else
        gain_sc = x_dB;

    float gainChange_dB = gain_sc - x_dB;

    // Smooth gain change (RMS Approximation)
    if (gainChange_dB < prevGainSmooth_) {
        // attack mode
        gainSmooth = -sqrt(((1 - alphaA) * powf(gainChange_dB,2))
                           + (alphaA * powf(prevGainSmooth_, 2)));
    } else {
        // release mode
        gainSmooth = -sqrt(((1 - alphaR) * powf(gainChange_dB,2))
                           + (alphaR * powf(prevGainSmooth_, 2)));
    }

    // Convert back to linear amplitude scalar
    auto lin_a = powf(10, gainSmooth / 20);
    float x_out = lin_a * sample;

    prevGainSmooth_ = gainSmooth;

    return x_out;
}
