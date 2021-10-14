#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_core/juce_core.h>
#include <juce_dsp/juce_dsp.h>
#include <juce_graphics/juce_graphics.h>

#include "../DSP/APCompressor.h"
#include "../DSP/APOverdrive.h"
#include "../DSP/APTubeDistortion.h"

enum class ProcessType
{
  Compression,
  Distortion,
  Overdrive,
  All
};

juce::AudioBuffer<float> loadFile(const juce::String& path)
{
  auto file = juce::File(path);

  juce::AudioFormatManager format_manager;
  format_manager.registerBasicFormats();

  std::unique_ptr<juce::AudioFormatReader> format_reader{ format_manager.createReaderFor(file) };
  auto num_samples = static_cast<int>(format_reader->lengthInSamples);
  juce::AudioBuffer<float> returnBuffer{ 1, num_samples };
  format_reader->read(&returnBuffer, 0, num_samples, 0, true, false);  // [td] could mix down to mono

  return returnBuffer;
}

void processSamplesWithCompression(juce::AudioBuffer<float>& buffer,
                                   APCompressor& compressor)  // [td] make process non-destructive
{
  juce::ScopedNoDenormals noDenormals;
  auto numChannels = buffer.getNumChannels();

  for (int channel = 0; channel < numChannels; ++channel)
  {
    auto* channelData = buffer.getWritePointer(channel);
    compressor.process(channelData, channelData, buffer.getNumSamples());
  }
}

void processSamplesWithDistortion(juce::AudioBuffer<float>& buffer, APTubeDistortion& distortion)
{
  juce::ScopedNoDenormals noDenormals;
  auto numChannels = buffer.getNumChannels();
  auto numSamples = buffer.getNumSamples();

  DBG(buffer.getSample(0,100));

  for (int channel = 0; channel < numChannels; ++channel)
  {
    auto* channelData = buffer.getWritePointer(channel);

    // Find the buffer's max magnitude
    auto bufferMinMax = buffer.findMinMax(channel, 0, numSamples);
    auto minMag = abs(bufferMinMax.getStart());
    auto maxMag = abs(bufferMinMax.getEnd());
    auto bufferMaxVal = juce::jmax(minMag, maxMag);

    DBG("bufferMaxVal: " << bufferMaxVal );

    APTubeDistortion::processDAFX(channelData, bufferMaxVal,
                                  1.0f, -0.4, 8.0f, 1.0f, channelData, numSamples);


//    APTubeDistortion::process(channelData, 3.0f, -0.2f, 8.0f, 0.8f, channelData, buffer.getNumSamples());
  }

  DBG(buffer.getSample(0, 100));
}

void processSamplesWithOverdrive(juce::AudioBuffer<float>& buffer, APOverdrive& overdrive)
{
  juce::ScopedNoDenormals noDenormals;
  auto numChannels = buffer.getNumChannels();

  for (int channel = 0; channel < numChannels; ++channel)
  {
    auto* channelData = buffer.getWritePointer(channel);
    APOverdrive::process(channelData, 0.5f, channelData, buffer.getNumSamples());
  }
}

void plotData(juce::Graphics& g, juce::Rectangle<int>& bounds, const juce::AudioBuffer<float>& buffer,
              const juce::Colour& lineColor = juce::Colours::black)
{
  float num_samples = buffer.getNumSamples();
  float x_ratio     = num_samples / bounds.toFloat().getWidth();
  jassert(x_ratio > 0.0f);

  std::vector<float> audioPoints;

  juce::Path p;
  p.startNewSubPath(bounds.getX(), bounds.getCentreY());

  for (auto sample = 0; sample < num_samples; sample += x_ratio)
  {
    auto sampleVal = buffer.getSample(0, sample);
    if (isnan(sampleVal)) sampleVal = 0;
    audioPoints.push_back(sampleVal);
  }

  for (int i = 0; i < bounds.getWidth(); ++i)
  {
    auto sampleVal = audioPoints[i];
    auto y_value     = juce::jmap(sampleVal, -1.0f, 1.0f, static_cast<float>(bounds.getBottom()),
                              static_cast<float>(bounds.getY()));
    p.lineTo(bounds.getX() + i, y_value);
  }

  g.setColour(lineColor);
  g.strokePath(p, juce::PathStrokeType(2));

  // Draw plot title
  g.setColour(juce::Colours::black);
  g.drawFittedText("Waveform", bounds.getCentreX() - 50, bounds.getY() + 20, 100, 10, juce::Justification::centred, 1);
}

void plotSpectrum(juce::Graphics& g, juce::Rectangle<int>& bounds, const juce::AudioBuffer<float>& buffer,
                  const juce::Colour& lineColor = juce::Colours::white)
{
  // Determine the size of the fft window by 2^order
  constexpr auto fftOrder = 11;
  constexpr auto fftSize  = 1 << fftOrder;  // 2048 (100000000000)

  // Initialize Fast Fourier Transform object and array to store data*
  // Windowing function used to counteract the effects of aliasing on samples
  // *fftData array size doubled to account for generated positive frequencies and their negative counterparts
  juce::dsp::FFT forwardFFT{ fftOrder };
  juce::dsp::WindowingFunction<float> window{ fftSize, juce::dsp::WindowingFunction<float>::hann };
  std::array<float, fftSize * 2> fftData{};
  std::array<float, fftSize> summedFft{};
  const int numLoops = static_cast<int>(ceilf(static_cast<float>(buffer.getNumSamples()) / fftSize));

  jassert(buffer.getNumSamples() > fftSize);
  for (auto i = 0; i < numLoops; ++i)
  {
    std::fill(fftData.begin(), fftData.end(), 0);  // zero out fftData

    // Fill first half of fftData with sample values from audio buffer
    const int firstFrameIndex = i * fftSize;
    for (auto j = 0; j < std::min(fftSize, buffer.getNumSamples() - firstFrameIndex); ++j)
    {
      fftData[j] = buffer.getSample(0, j + firstFrameIndex);
    }

    // Apply windowing function to sample data
    window.multiplyWithWindowingTable(fftData.data(), fftSize);
    // Perform FFT in place on fftData
    forwardFFT.performFrequencyOnlyForwardTransform(fftData.data());

    // Average fft windows
    juce::FloatVectorOperations::add(summedFft.data(), fftData.data(), fftSize);
  }
  juce::FloatVectorOperations::multiply(summedFft.data(), 1.0f / static_cast<float>(numLoops), fftSize);

  // Define dB bounds
  auto mindB = -100.0f;
  auto maxdB = 0.0f;

  auto width = bounds.getWidth();
  juce::Path p;
  p.startNewSubPath(bounds.getX(), bounds.getBottom());

  for (int i = 0; i < width; ++i)
  {
    auto skewedProportionX = 1.0f - std::exp(std::log(1.0f - (float)i / (float)width) * 0.2f);
    auto fftDataIndex      = juce::jlimit(0, fftSize / 2, (int)(skewedProportionX * (float)fftSize * 0.5f));
    auto y                 = juce::jmap(juce::jlimit(mindB, maxdB,
                                     juce::Decibels::gainToDecibels(summedFft[fftDataIndex]) -
                                         juce::Decibels::gainToDecibels((float)fftSize)),
                        mindB, maxdB, static_cast<float>(bounds.getY()), static_cast<float>(bounds.getBottom()));

    auto x = bounds.getX() + i;
    p.lineTo(x, y);
  }

  // Draw spectrum analyzer path
  g.setColour(lineColor);
  g.strokePath(p, juce::PathStrokeType(2));

  // Draw plot title
  g.setColour(juce::Colours::white);
  g.drawFittedText("Spectrum Analyzer", bounds.getCentreX() - 50, bounds.getY() + 20,
                   100, 10, juce::Justification::centred,
                   1);
}

juce::Image generatePlots(const juce::String& file_path, ProcessType type)
{
  juce::ScopedJuceInitialiser_GUI myInit;

  juce::Image plot_image(juce::Image::PixelFormat::ARGB, 500, 500, true);
  auto plot_graphics      = juce::Graphics(plot_image);
  auto top_plot_bounds    = juce::Rectangle<int>(30, 30, 440, 205);
  auto bottom_plot_bounds = juce::Rectangle<int>(30, 265, 440, 205);

  auto audio_buffer = loadFile(file_path);

  APCompressor compressor;
  compressor.setSampleRate(44100.0f);
  compressor.updateParameters(-24.0f, 14.0f);
  APTubeDistortion distortion;
  APOverdrive overdrive;

  // Draw top and bottom plot backgrounds
  plot_graphics.setColour(juce::Colours::white);
  plot_graphics.fillRect(top_plot_bounds);
  plot_graphics.setColour(juce::Colours::black);
  plot_graphics.fillRect(bottom_plot_bounds);

  auto beforeColor = juce::Colours::red;
  plotData(plot_graphics, top_plot_bounds, audio_buffer, beforeColor);         // Before compression
  plotSpectrum(plot_graphics, bottom_plot_bounds, audio_buffer, beforeColor);  // Before compression

  switch (type)
  {
    case ProcessType::Compression: processSamplesWithCompression(audio_buffer, compressor); break;
    case ProcessType::Distortion: processSamplesWithDistortion(audio_buffer, distortion); break;
    case ProcessType::Overdrive: processSamplesWithOverdrive(audio_buffer, overdrive); break;
    case ProcessType::All:
      processSamplesWithCompression(audio_buffer, compressor);
      processSamplesWithDistortion(audio_buffer, distortion);
      processSamplesWithOverdrive(audio_buffer, overdrive);
      break;
    default: break;
  }

  auto afterColor = juce::Colours::green;
  plotData(plot_graphics, top_plot_bounds, audio_buffer, afterColor);         // After compression
  plotSpectrum(plot_graphics, bottom_plot_bounds, audio_buffer, afterColor);  // After compression

  return plot_image;
}

bool writeImageToPngFile(const juce::Image& plot_image, juce::File& plot_file)
{
  juce::PNGImageFormat plot_format;
  juce::FileOutputStream plot_output_stream(plot_file);
  auto write_result = plot_format.writeImageToStream(plot_image, plot_output_stream);

  return write_result;
}

int main()
{
  const auto plot_image_all =
      generatePlots("/Users/silveryar/development/juce/ap_dynamics/Tests/conk.wav", ProcessType::Distortion);

  juce::File plot_file = juce::File::createTempFile(".png");
  auto write_result    = writeImageToPngFile(plot_image_all, plot_file);
  if (write_result)
  {
    plot_file.startAsProcess();
  }
  else
  {
    DBG("Image write unsuccessful");
  }

  return 1;
}