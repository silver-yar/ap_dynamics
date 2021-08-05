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

APTubeDistortion::APTubeDistortion() = default;

APTubeDistortion::~APTubeDistortion() = default;

void APTubeDistortion::process(const float* audioIn, float distGain, float Q, float distChar, float mix, float* audioOut,
                               float numSamplesToRender)
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
    out = mix * z * abs(in) / abs(z) + (1.0 - mix) * in;
    out *= abs(in) / abs(out);

    audioOut[i] = in != 0 ? static_cast<float>(out) : static_cast<float>(in);
  }
}
