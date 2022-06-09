/*
  ==============================================================================

    APTubeDistortion.cpp
    Created: 2 Mar 2021 1:42:57pm
    Author:  Johnathan Handy

  ==============================================================================
*/

#include "APTubeDistortion.h"

#include <cmath>

APTubeDistortion::APTubeDistortion() {
//  postHighPass_ = std::make_unique<juce::dsp::IIR::Filter<float>>();
}

APTubeDistortion::~APTubeDistortion() = default;

void APTubeDistortion::process(const float* audioIn, const float maxBufferVal, const float distGain, const float Q,
                               const float distChar, float* audioOut, const int numSamplesToRender) const
{
//  postHighPass_->snapToZero();
  auto maxZ = 0.0;
  auto maxOut = 0.0f;

  // Calculate z
  for (auto i = 0; i < numSamplesToRender; ++i)
  {
    const auto& in = audioIn[i];

    if (in == 0.0f || maxBufferVal == 0.0f)
    {
      audioOut[i] = in;
    }
    else
    {
      double z     = 0.0;
      const auto q = in * distGain / maxBufferVal;  // Normalization 0.0f - 1.0f

      if (Q == 0)
      {
        z = q / (1.0 - exp(-distChar * q));
        if (q == Q)
        {
          z = 1.0 / distChar;
        }
      }
      else
      {
        z = (q - Q) / (1.0 - exp(-distChar * (q - Q))) + Q / (1.0 - exp(distChar * Q));
        if (q == Q)
        {
          z = 1 / distChar + Q / (1.0 - exp(distChar * Q));
        }
      }

      if (maxZ < z)
        maxZ = z;

      auto out = static_cast<float>(z * maxBufferVal / maxZ );

      if (maxOut < out)
        maxOut = out;

      out = out * maxBufferVal / maxOut;
      // HP Filter

      audioOut[i] = out;
    }
  }
}