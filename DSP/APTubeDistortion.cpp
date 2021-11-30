/*
  ==============================================================================

    APTubeDistortion.cpp
    Created: 2 Mar 2021 1:42:57pm
    Author:  Johnathan Handy

  ==============================================================================
*/

#include "APTubeDistortion.h"

#include <cmath>
#include <vector>

APTubeDistortion::APTubeDistortion() = default;

APTubeDistortion::~APTubeDistortion() = default;

void APTubeDistortion::process(const float* audioIn, const float maxBufferVal, const float distGain, const float Q,
                               const float distChar, float* audioOut, const int numSamplesToRender) const
{
  auto maxZ = 0.0;
  std::vector<double> zArray;

  // Calculate z
  for (auto i = 0; i < numSamplesToRender; ++i)
  {
    const auto& in = audioIn[i];
    double z       = 0.0;
    const auto q   = in * distGain / maxBufferVal;  // Normalization

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

    zArray.emplace_back(z);
    if (maxZ < z)
      maxZ = z;
  }

  // mix_ing
  for (auto i = 0; i < numSamplesToRender; ++i)
  {
    const auto& in = audioIn[i];
    const auto out = [&]()
    {
      auto result        = mix_ * zArray[static_cast<unsigned long>(i)] * (maxBufferVal / maxZ) + (1.0f - mix_) * in;
      result *= maxBufferVal / maxZ;
      return result;
    }();

    audioOut[i] = static_cast<float>(out);
  }
}
