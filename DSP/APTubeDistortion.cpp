/*
  ==============================================================================

    APTubeDistortion.cpp
    Created: 2 Mar 2021 1:42:57pm
    Author:  Johnathan Handy

  ==============================================================================
*/

#include "APTubeDistortion.h"

#include <cmath>

APTubeDistortion::APTubeDistortion() = default;

APTubeDistortion::~APTubeDistortion() = default;

void APTubeDistortion::process(const float* audioIn, const float maxBufferVal, const float distGain, const float Q,
                               const float distChar, float* audioOut, const int numSamplesToRender) const
{
  auto maxZ = 0.0;

  // Calculate z
  for (auto i = 0; i < numSamplesToRender; ++i)
  {
    if (maxBufferVal > 0)
    {
      const auto& in = audioIn[i];
      double z       = 0.0;
      const auto q   = maxBufferVal > 0 ? in * distGain / maxBufferVal : 0;  // Normalization

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

      audioOut[i] = static_cast<float>((mix_ * z * (maxBufferVal / maxZ) + (1.0f - mix_) * in) * maxBufferVal / maxZ);
    }
    else
    {
      audioOut[i] = 0;
    }
  }
}
