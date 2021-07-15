/*
  ==============================================================================

    APCompressor.h
    Created: 26 Feb 2021 2:08:02pm
    Author:  Johnathan Handy

  ==============================================================================
*/

#pragma once
#include <utility>

class APCompressor
{
 public:
  static constexpr float minusInfinityDb = -96.0f;

  APCompressor();
  ~APCompressor();

  void setSampleRate(float sampleRate) { sampleRate_ = sampleRate; }
  void updateParameters(float threshold, float ratio)
  {
    threshold_ = threshold;
    ratio_     = ratio;
  }
  void reset()
  {
    prevGainSmooth_ = 0;
  };

  void process(const float* audioIn, float* audioOut, float numSamplesToRender);

  static std::pair<float, float> _applyRMSCompression(float sample,  float sampleRate, float threshold, float ratio,
                                    float attack, float release, float kneeWidth,
                                    float prevGainSmoothed);
  float applyRMSCompression(float sample);

 private:
  float sampleRate_     = 0.0f;
  float threshold_      = 0.0f;
  float ratio_          = 1.0f;
  float attack_         = 0.02f;  // 50 ms
  float release_        = 0.08f;  // 80 ms
  float kneeWidth_      = 6.0f;
  float prevGainSmooth_ = 0.0f;
};
