/*
  ==============================================================================

    APTubeDistortion.cpp
    Created: 2 Mar 2021 1:42:57pm
    Author:  Johnathan Handy

  ==============================================================================
*/

#include "APTubeDistortion.h"
#include <stdlib.h>
#include <math.h>
#include <vector>

APTubeDistortion::APTubeDistortion() = default;

APTubeDistortion::~APTubeDistortion() = default;

void APTubeDistortion::process(const float* audioIn, float distGain, float Q, float distChar, float mix, float* audioOut,
                               int numSamplesToRender)
{
  for (auto i = 0; i < numSamplesToRender; ++i)
  {
    const auto& in = audioIn[i];
    double z       = 0.0;
    double out     = 0.0;
    auto q         = in * distGain / abs(in);

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
    out = (mix * z * abs(in)) / (abs(z) + (1.0 - mix) * in);
    out *= abs(in) / abs(out);

    audioOut[i] = in != 0 ? static_cast<float>(out) : static_cast<float>(in);
  }
}
void APTubeDistortion::processDAFX(const float* audioIn, const float maxBufferVal, float distGain, float Q, float distChar,
                                   float mix, float* audioOut, int numSamplesToRender)
{
  double maxZ = 0.0;
  std::vector<double> zArray;

  // Calculate z
  for (auto i = 0; i < numSamplesToRender; ++i)
  {
    const auto& in = audioIn[i];
    double z       = 0.0;
    auto q         = in * distGain / maxBufferVal; // Normalization

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

    zArray.push_back(z);
    if (maxZ < z) maxZ = z;
  }

  // Mixing
  for (auto i = 0; i < numSamplesToRender; ++i)
  {
    const auto& in = audioIn[i];
    double out     = 0.0;
    out = mix * zArray[i] * (maxBufferVal / maxZ) + (1.0f - mix) * in;
    out *= maxBufferVal / maxZ;

    audioOut[i] = static_cast<float>(out);
  }
}
