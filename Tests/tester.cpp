#define CATCH_CONFIG_RUNNER

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_core/juce_core.h>
#include <juce_graphics/juce_graphics.h>

#include <catch2/catch.hpp>
#include <iostream>

#include "../DSP/APCompressor.h"

/*
 * DSP Tests
 * 6/10/2021
 */

void fillBufferSampleData(juce::AudioBuffer<float>& buffer)
{
  for (auto channel = 0; channel < buffer.getNumChannels(); ++channel)
  {
    auto value = 0.0f;
    for (auto sample = 0; sample < buffer.getNumSamples() / 2; ++sample)
    {
      buffer.setSample(channel, sample, value);
      value += 0.2f;
    }
  }
}

TEST_CASE("DSP TESTS")
{
  constexpr int numChannelsToAllocate = 2;
  constexpr int numSamplesToAllocate  = 12;
  constexpr float sampleRate          = 48000.0f;
  constexpr float threshold           = -6.0f;
  constexpr float ratio               = 3.0f;
  constexpr float attack              = 0.02f;  // 50 ms
  constexpr float release             = 0.08f;  // 80 ms
  constexpr float kneeWidth           = 6.0f;

  std::vector<double> sample_periods;

  juce::AudioBuffer<float> inputBuffer{ numChannelsToAllocate, numSamplesToAllocate };
  fillBufferSampleData(inputBuffer);
  juce::AudioBuffer<float> outputBuffer{ numChannelsToAllocate, numSamplesToAllocate };
  outputBuffer.clear();

  APCompressor compressor;
  compressor.setSampleRate(sampleRate);
  compressor.updateParameters(threshold, ratio);

  for (auto channel = 0; channel < inputBuffer.getNumChannels(); ++channel)
  {
    auto* channelDataInput  = inputBuffer.getReadPointer(channel);
    auto* channelDataOutput = outputBuffer.getWritePointer(channel);
    compressor.process(channelDataInput, channelDataOutput, inputBuffer.getNumSamples());
    compressor.reset();
  }

  bool allGood = true;

  for (auto channel = 0; channel < inputBuffer.getNumChannels(); ++channel)
  {
    float prevGainSmoothed = 0.0f;
    for (auto sample = 0; sample < inputBuffer.getNumSamples(); ++sample)
    {
      auto sp_start = std::chrono::high_resolution_clock::now();

      const auto [result, gainSmoothed] =
          compressor._applyRMSCompression(inputBuffer.getSample(channel, sample), sampleRate, threshold, ratio, attack,
                                          release, kneeWidth, prevGainSmoothed);

      auto sp_end        = std::chrono::high_resolution_clock::now();
      auto sample_period = std::chrono::duration_cast<std::chrono::duration<double>>(sp_end - sp_start);
//      DBG("Sample Processing Period (s): " << sample_period.count() << " s");
      sample_periods.emplace_back(sample_period.count());

      prevGainSmoothed = gainSmoothed;
      if (result != outputBuffer.getSample(channel, sample))
      {
        allGood = false;
      }
    }
    compressor.updateParameters(-10.0f, 4);
  }

  double sp_avg = 0.0;
  for (const auto &sp: sample_periods) {
    sp_avg += sp;
  }
  sp_avg = sp_avg / sample_periods.size();
//  DBG("\nAverage Sample Processing Period (s): " << sp_avg << " s");
//  DBG("Average Sample Processing Frequency (Hz): " << 1.0 / sp_avg << " Hz");

  CHECK(allGood);
}


void plot(const juce::String& plotData) {
  juce::ScopedJuceInitialiser_GUI myInit;

//  auto plot_image = std::make_unique<juce::Image>
//      (juce::Image::PixelFormat::ARGB, 200, 200, true);
  juce::Image plot_image (juce::Image::PixelFormat::ARGB, 200, 200, true);
//  auto plot_graphics = std::make_unique<juce::Graphics> (plot_image);
  auto plot_graphics = juce::Graphics(plot_image);
  plot_graphics.fillAll(juce::Colours::yellow);

  juce::File plot_file = juce::File::createTempFile( ".png");
  juce::PNGImageFormat plot_format;
  juce::FileOutputStream plot_output_stream (plot_file);
  auto write_result = plot_format.writeImageToStream(plot_image, plot_output_stream);
  if (!write_result) { DBG("Image write unsuccessful");}

  DBG(plot_file.getFullPathName());
}

void callGraphics() {
  juce::ScopedJuceInitialiser_GUI myInit;

  juce::Image my_image (juce::Image::PixelFormat::ARGB, 200, 200, true);
  {
    juce::Graphics myGraphics(my_image);
  }
}

int main(int argc, char* argv[])
{
  int testResult = Catch::Session().run(argc, argv);
  plot("Hello");
//  callGraphics();

  return testResult;
}