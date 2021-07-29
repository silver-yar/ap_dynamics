#include <juce_core/juce_core.h>
#include <juce_graphics/juce_graphics.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include "../DSP/APCompressor.h"

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

void processSamples (juce::AudioBuffer<float>& buffer, APCompressor& compressor) {
  juce::ScopedNoDenormals noDenormals;
  auto numChannels            = 2;

  for (int channel = 0; channel < numChannels; ++channel) {
    auto* channelData = buffer.getWritePointer(channel);
    compressor.process(channelData, channelData, buffer.getNumSamples());
  }
}

void plotData(juce::Graphics& g, juce::Rectangle<int>& bounds, juce::AudioBuffer<float>& buffer) {
  g.setColour(juce::Colours::lightcoral);
  g.fillRect(bounds);
  auto num_samples = buffer.getNumSamples();
  std::vector<float> x_scaled_audio;
  auto x_ratio = num_samples / bounds.toFloat().getWidth();
  auto buffer_read = buffer.getReadPointer (0);
  for (int sample = 0; sample < num_samples; sample+=x_ratio)
  {
    x_scaled_audio.emplace_back (buffer_read[sample]);
  }

  juce::Path p;
  p.startNewSubPath(bounds.getX(), bounds.getCentreY());
  for (auto i = 0; i < x_scaled_audio.size(); i++) {
    auto y_value = juce::jmap(buffer_read[i], -1.0f, 1.0f,
                              static_cast<float> (bounds.getHeight()), static_cast<float> (bounds.getY()));
    p.lineTo(i, y_value);
//    p.lineTo(bounds.getX() + (i * step_size), juce::jmap(buffer.getSample(0,i), -1.0f, 1.0f,
//                                             (float) bounds.getBottom(), (float) bounds.getY()));
  }

  g.setColour(juce::Colours::black);
  g.strokePath(p, juce::PathStrokeType(2));
}

void generatePlots(const juce::String& plotName) {
  juce::ScopedJuceInitialiser_GUI myInit;

  juce::Image plot_image (juce::Image::PixelFormat::ARGB, 500, 500, true);
  auto plot_graphics = juce::Graphics(plot_image);
  auto top_plot_bounds = juce::Rectangle<int> (30, 30, 440, 205);
  auto bottom_plot_bounds = juce::Rectangle<int> (30, 265, 440, 205);
  int buffer_size = 256;
  juce::AudioBuffer<float> audio_buffer {2, buffer_size};
  APCompressor compressor;
  juce::String file_path = "/Users/silveryar/development/juce/ap_dynamics/Tests/noise.wav";

  loadFile(file_path, &audio_buffer, buffer_size);
  plotData(plot_graphics, top_plot_bounds, audio_buffer); // Before compression
  processSamples (audio_buffer, compressor);
  plotData(plot_graphics, bottom_plot_bounds, audio_buffer); // After compression

  juce::File plot_file = juce::File::createTempFile( ".png");
  juce::PNGImageFormat plot_format;
  juce::FileOutputStream plot_output_stream (plot_file);
  auto write_result = plot_format.writeImageToStream(plot_image, plot_output_stream);
  if (!write_result) { DBG("Image write unsuccessful");}

  DBG(plot_file.getFullPathName());
}

int main()
{
  generatePlots("Hello");

  return 1;
}