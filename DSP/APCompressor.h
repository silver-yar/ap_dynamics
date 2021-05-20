/*
  ==============================================================================

    APCompressor.h
    Created: 26 Feb 2021 2:08:02pm
    Author:  Johnathan Handy

  ==============================================================================
*/

#pragma once

const static int MIN_DB = -144;

class APCompressor
{
public:
    APCompressor();
    ~APCompressor();

    void setSampleRate (float sampleRate) { sampleRate_ = sampleRate; }
    void updateParameters (float threshold, float ratio) { threshold_ = threshold; ratio_ = ratio; }
    void reset() { prevGainSmooth_ = 0; y_prev_ = 0;};

    void process (float* audioIn,
                  float* audioOut,
                  float numSamplesToRender
                  );

    float applyFFCompression (float sample);
    float applyFBCompression (float sample);
    float applyRMSCompression (float sample);

private:
    float sampleRate_;
    float threshold_ = 0.0f;
    float ratio_ = 1.0f;
    float attack_ = 0.02f; // 50 ms
    float release_ = 0.08f; // 80 ms
    float kneeWidth_ = 6.0f;
    float prevGainSmooth_ = 0.0f;
    float y_prev_ = 0.0f;
};
