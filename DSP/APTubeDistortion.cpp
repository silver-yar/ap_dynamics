/*
  ==============================================================================

    APTubeDistortion.cpp
    Created: 2 Mar 2021 1:42:57pm
    Author:  Johnathan Handy

  ==============================================================================
*/

#include "APTubeDistortion.h"

#include "JuceHeader.h"

APTubeDistortion::APTubeDistortion() { }

APTubeDistortion::~APTubeDistortion() { }

void APTubeDistortion::process(float* audioIn, float distGain, float Q, float distChar, float mix, float* audioOut,
                               float numSamplesToRender)
{
  for (auto i = 0; i < numSamplesToRender; ++i)
  {
    const auto& in = audioIn[i];
    double z;
    double out;
    auto q = in * distGain / abs(in);

    if (!Q)
    {
      z = q / (1 - exp(-distChar * q));
      if (q == Q)
      {
        z = 1 / distChar;
      }
    }
    else
    {
      z = (q - Q) / (1 - exp(-distChar * (q - Q))) + Q / (1 - exp(distChar * Q));
      if (q == Q)
      {
        z = 1 / distChar + Q / (1 - exp(distChar * Q));
      }
    }
    out = mix * z * abs(in) / abs(z) + (1 - mix) * in;
    out *= abs(in) / abs(out);

    audioOut[i] = in != 0 ? out : in;
  }
}
