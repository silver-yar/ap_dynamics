#include <juce_core/juce_core.h>
#include <juce_graphics/juce_graphics.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include "../DSP/APCompressor.h"
#include "../DSP/APTubeDistortion.h"
#include "../DSP/APOverdrive.h"

// Create a juce audio buffer - x
// Add data to buffer via generated sample values or from file
// Loop buffer values through compression algorithm and store in output buffer - x
// Create function to intialize graphics context and image - x
// Create a function to take graphics context, buffer, and bounds and draws generatePlot - x

void loadFile(const juce::String& path, juce::AudioBuffer<float>* buffer, int bufferSize) {
  auto file = juce::File (path);

  juce::AudioFormatManager format_manager;
  format_manager.registerBasicFormats();

  std::unique_ptr<juce::AudioFormatReader> format_reader{ format_manager.createReaderFor(file) };
  auto num_samples = static_cast<int>(format_reader->lengthInSamples);
  format_reader->read(buffer, 0, bufferSize, 0, true, true);
}

void processSamplesWithCompression(juce::AudioBuffer<float>& buffer, APCompressor& compressor) {
  juce::ScopedNoDenormals noDenormals;
  auto numChannels            = 2;

  for (int channel = 0; channel < numChannels; ++channel) {
    auto* channelData = buffer.getWritePointer(channel);
    compressor.process(channelData, channelData, buffer.getNumSamples());
  }
}

void processSamplesWithDistortion(juce::AudioBuffer<float>& buffer, APTubeDistortion& distortion) {
  juce::ScopedNoDenormals noDenormals;
  auto numChannels            = 2;

  for (int channel = 0; channel < numChannels; ++channel) {
    auto* channelData = buffer.getWritePointer(channel);
    distortion.process(channelData, 1.0f, -0.2f, 8.0f, 1.0f, channelData, buffer.getNumSamples());
  }
}

void processSamplesWithOverdrive(juce::AudioBuffer<float>& buffer, APOverdrive& overdrive) {
  juce::ScopedNoDenormals noDenormals;
  auto numChannels            = 2;

  for (int channel = 0; channel < numChannels; ++channel) {
    auto* channelData = buffer.getWritePointer(channel);
    overdrive.process(channelData, 1.0f, channelData, buffer.getNumSamples());
  }
}

void plotData(juce::Graphics& g, juce::Rectangle<int>& bounds, juce::AudioBuffer<float>& buffer) {
  g.setColour(juce::Colours::lightcoral);
  g.fillRect(bounds);
  auto buffer_read = buffer.getReadPointer(0);
  float num_samples = buffer.getNumSamples();
  float x_ratio = num_samples / bounds.toFloat().getWidth();
  jassert(x_ratio > 0.0f);

  juce::Path p;
  p.startNewSubPath(bounds.getX(), bounds.getCentreY());
  for (int i = 0; i < bounds.getWidth(); i++) {
    int sample_index = static_cast<int> (static_cast<float>(i) * x_ratio);
    auto y_value = juce::jmap(buffer_read[sample_index], -1.0f, 1.0f,
                              static_cast<float> (bounds.getBottom()), static_cast<float> (bounds.getY()));
    p.lineTo(bounds.getX() + i, y_value);
  }

  g.setColour(juce::Colours::black);
  g.strokePath(p, juce::PathStrokeType(2));
}

juce::Image generatePlots(const juce::String& file_path, int plot_type) {
  juce::ScopedJuceInitialiser_GUI myInit;

  juce::Image plot_image (juce::Image::PixelFormat::ARGB, 500, 500, true);
  auto plot_graphics = juce::Graphics(plot_image);
  auto top_plot_bounds = juce::Rectangle<int> (30, 30, 440, 205);
  auto bottom_plot_bounds = juce::Rectangle<int> (30, 265, 440, 205);
  int buffer_size = 256;
  juce::AudioBuffer<float> audio_buffer {2, buffer_size};

  loadFile(file_path, &audio_buffer, buffer_size);
  plotData(plot_graphics, top_plot_bounds, audio_buffer); // Before compression

  APCompressor compressor;
  compressor.setSampleRate(44100.0f);
  compressor.updateParameters(-20.0f, 10.0f);
  APTubeDistortion distortion;
  APOverdrive overdrive;
  switch (plot_type)  // 1 = compression, 2 = tube distortion, 3 = overdrive, 4 = all
  {
    case 1:
      processSamplesWithCompression(audio_buffer, compressor);
      break;
    case 2:
      processSamplesWithDistortion(audio_buffer, distortion);
      break;
    case 3:
      processSamplesWithOverdrive(audio_buffer, overdrive);
      break;
    case 4:
      processSamplesWithCompression(audio_buffer, compressor);
      processSamplesWithDistortion(audio_buffer, distortion);
      processSamplesWithOverdrive(audio_buffer, overdrive);
      break;
    default: break;
  }
  plotData(plot_graphics, bottom_plot_bounds, audio_buffer); // After compression

  return plot_image;
}

bool writeImageToPngFile(const juce::Image& plot_image, juce::File& plot_file) {
  juce::PNGImageFormat plot_format;
  juce::FileOutputStream plot_output_stream (plot_file);
  auto write_result = plot_format.writeImageToStream(plot_image, plot_output_stream);

  return write_result;
}

int main()
{
//  const auto plot_image_comp =
//      generatePlots("/Users/silveryar/development/juce/ap_dynamics/Tests/noise.wav", 1);
  const auto plot_image_all =
      generatePlots("/Users/silveryar/development/juce/ap_dynamics/Tests/noise.wav", 4);

  juce::File plot_file = juce::File::createTempFile( ".png");
  auto write_result = writeImageToPngFile(plot_image_all, plot_file);
  if (write_result) {
    plot_file.startAsProcess ();
  } else {
    DBG("Image write unsuccessful");
  }

  return 1;
}