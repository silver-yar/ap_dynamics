#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_core/juce_core.h>
#include <juce_dsp/juce_dsp.h>
#include <juce_graphics/juce_graphics.h>

#include "../DSP/APCompressor.h"
#include "../DSP/APOverdrive.h"
#include "../DSP/APTubeDistortion.h"

// Create a juce audio buffer - x
// Add data to buffer via generated sample values or from file
// Loop buffer values through compression algorithm and store in output buffer - x
// Create function to intialize graphics context and image - x
// Create a function to take graphics context, buffer, and bounds and draws generatePlot - x

void loadFile(const juce::String& path, juce::AudioBuffer<float>* buffer, int bufferSize)
{
  auto file = juce::File(path);

  juce::AudioFormatManager format_manager;
  format_manager.registerBasicFormats();

  std::unique_ptr<juce::AudioFormatReader> format_reader{ format_manager.createReaderFor(file) };
  auto num_samples = static_cast<int>(format_reader->lengthInSamples);
  format_reader->read(buffer, 0, bufferSize, 0, true, true);
}

void processSamplesWithCompression(juce::AudioBuffer<float>& buffer, APCompressor& compressor)
{
  juce::ScopedNoDenormals noDenormals;
  auto numChannels = 2;

  for (int channel = 0; channel < numChannels; ++channel)
  {
    auto* channelData = buffer.getWritePointer(channel);
    compressor.process(channelData, channelData, buffer.getNumSamples());
  }
}

void processSamplesWithDistortion(juce::AudioBuffer<float>& buffer, APTubeDistortion& distortion)
{
  juce::ScopedNoDenormals noDenormals;
  auto numChannels = 2;

  for (int channel = 0; channel < numChannels; ++channel)
  {
    auto* channelData = buffer.getWritePointer(channel);
    distortion.process(channelData, 1.0f, -0.2f, 8.0f, 1.0f, channelData, buffer.getNumSamples());
  }
}

void processSamplesWithOverdrive(juce::AudioBuffer<float>& buffer, APOverdrive& overdrive)
{
  juce::ScopedNoDenormals noDenormals;
  auto numChannels = 2;

  for (int channel = 0; channel < numChannels; ++channel)
  {
    auto* channelData = buffer.getWritePointer(channel);
    overdrive.process(channelData, 1.0f, channelData, buffer.getNumSamples());
  }
}

void plotData(juce::Graphics& g, juce::Rectangle<int>& bounds,
              juce::AudioBuffer<float>& buffer, const juce::Colour& lineColor = juce::Colours::black)
{
  auto buffer_read  = buffer.getReadPointer(0);
  float num_samples = buffer.getNumSamples();
  float x_ratio     = num_samples / bounds.toFloat().getWidth();
  jassert(x_ratio > 0.0f);

  juce::Path p;
  p.startNewSubPath(bounds.getX(), bounds.getCentreY());
  for (int i = 0; i < bounds.getWidth(); i++)
  {
    int sample_index = static_cast<int>(static_cast<float>(i) * x_ratio);
    auto y_value     = juce::jmap(buffer_read[sample_index], -1.0f, 1.0f, static_cast<float>(bounds.getBottom()),
                              static_cast<float>(bounds.getY()));
    p.lineTo(bounds.getX() + i, y_value);
  }

  g.setColour(lineColor);
  g.strokePath(p, juce::PathStrokeType(2));
}

void plotSpectrum(juce::Graphics& g, juce::Rectangle<int>& bounds,
                  juce::AudioBuffer<float>& buffer, const juce::Colour& lineColor = juce::Colours::white)
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

  std::fill(fftData.begin(), fftData.end(), 0);  // zero out fftData

  // Fill first half of fftData with sample values from audio buffer
  if (fftSize == buffer.getNumSamples())
  {
    for (auto i = 0; i < buffer.getNumSamples(); ++i)
    {
      fftData[i] = buffer.getSample(0, i);
    }
  }
  else
  {
    DBG("Buffer size does not match fft size.");
  }

  // Apply windowing function to sample data
  window.multiplyWithWindowingTable(fftData.data(), fftSize);
  // Perform FFT in place on fftData
  forwardFFT.performFrequencyOnlyForwardTransform(fftData.data());
  // Define dB bounds
  auto mindB = -100.0f;
  auto maxdB = 0.0f;

  auto width = bounds.getWidth();
  juce::Path p;
  p.startNewSubPath(bounds.getX(), bounds.getBottom());

  for (int i = 0; i < width; ++i)
  {
    auto skewedProportionX = 1.0f - std::exp(std::log(1.0f - (float) i / (float) width) * 0.2f);
    auto fftDataIndex      = juce::jlimit(0, fftSize / 2,
                                     (int) (skewedProportionX * (float) fftSize * 0.5f));
    auto y                 = juce::jmap(juce::jlimit(mindB, maxdB,
                                     juce::Decibels::gainToDecibels(fftData[fftDataIndex]) -
                                         juce::Decibels::gainToDecibels((float) fftSize)),
                        mindB, maxdB, static_cast<float>(bounds.getY()), static_cast<float>(bounds.getBottom()));

    auto x = bounds.getX() + i;
    p.lineTo(x, y);
  }

  // Draw spectrum analyzer path
  g.setColour(lineColor);
  g.strokePath(p, juce::PathStrokeType(2));
}

juce::Image generatePlots(const juce::String& file_path, int plot_type)
{
  juce::ScopedJuceInitialiser_GUI myInit;

  juce::Image plot_image(juce::Image::PixelFormat::ARGB, 500, 500, true);
  auto plot_graphics      = juce::Graphics(plot_image);
  auto top_plot_bounds    = juce::Rectangle<int>(30, 30, 440, 205);
  auto bottom_plot_bounds = juce::Rectangle<int>(30, 265, 440, 205);
  int buffer_size         = 2048;
  juce::AudioBuffer<float> audio_buffer{ 2, buffer_size };

  loadFile(file_path, &audio_buffer, buffer_size);

  APCompressor compressor;
  compressor.setSampleRate(44100.0f);
  compressor.updateParameters(-20.0f, 10.0f);
  APTubeDistortion distortion;
  APOverdrive overdrive;

  // Draw top and bottom plot backgrounds
  plot_graphics.setColour(juce::Colours::white);
  plot_graphics.fillRect(top_plot_bounds);
  plot_graphics.setColour(juce::Colours::black);
  plot_graphics.fillRect(bottom_plot_bounds);

  auto beforeColor = juce::Colours::red;
  plotData(plot_graphics, top_plot_bounds, audio_buffer, beforeColor);  // Before compression
  plotSpectrum(plot_graphics, bottom_plot_bounds, audio_buffer, beforeColor);  // Before compression

  switch (plot_type)  // 1 = compression, 2 = tube distortion, 3 = overdrive, 4 = all
  {
    case 1: processSamplesWithCompression(audio_buffer, compressor); break;
    case 2: processSamplesWithDistortion(audio_buffer, distortion); break;
    case 3: processSamplesWithOverdrive(audio_buffer, overdrive); break;
    case 4:
      processSamplesWithCompression(audio_buffer, compressor);
      processSamplesWithDistortion(audio_buffer, distortion);
      processSamplesWithOverdrive(audio_buffer, overdrive);
      break;
    default: break;
  }

  auto afterColor = juce::Colours::green;
  plotData(plot_graphics, top_plot_bounds, audio_buffer, afterColor);  // After compression
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
  //  const auto plot_image_comp =
  //      generatePlots("/Users/silveryar/development/juce/ap_dynamics/Tests/noise.wav", 1);
  const auto plot_image_all = generatePlots("/Users/silveryar/development/juce/ap_dynamics/Tests/noise.wav", 4);

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