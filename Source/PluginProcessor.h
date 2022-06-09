/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "juce_dsp/juce_dsp.h"

#include "../DSP/APCompressor.h"
#include "../DSP/APOverdrive.h"
#include "../DSP/APTubeDistortion.h"

//==============================================================================
/**
 */
class Ap_dynamicsAudioProcessor : public juce::AudioProcessor, public juce::ValueTree::Listener
{
 public:
  //==============================================================================
  Ap_dynamicsAudioProcessor();
  ~Ap_dynamicsAudioProcessor() override;

  //==============================================================================
  void prepareToPlay(double sampleRate, int samplesPerBlock) override;
  void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
  bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

  void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

  //==============================================================================
  juce::AudioProcessorEditor* createEditor() override;
  bool hasEditor() const override;

  //==============================================================================
  const juce::String getName() const override;

  bool acceptsMidi() const override;
  bool producesMidi() const override;
  bool isMidiEffect() const override;
  double getTailLengthSeconds() const override;

  //==============================================================================
  int getNumPrograms() override;
  int getCurrentProgram() override;
  void setCurrentProgram(int index) override;
  const juce::String getProgramName(int index) override;
  void changeProgramName(int index, const juce::String& newName) override;

  //==============================================================================
  void getStateInformation(juce::MemoryBlock& destData) override;
  void setStateInformation(const void* data, int sizeInBytes) override;

  //==============================================================================

  // ValueTree
  juce::AudioProcessorValueTreeState apvts;

  std::atomic<float> meterLocalMaxVal {0.0f};
  std::atomic<float> meterGlobalMaxVal {0.0f};

  // Updates DSP when user changes parameters
  void update();
  // Overrides AudioProcessor reset, reset DSP parameters
  void reset() override;
  // Create parameter layout for apvts
  static juce::AudioProcessorValueTreeState::ParameterLayout createParameters();

 private:
  std::atomic<bool> mustUpdateProcessing_{ false }, isActive_{ false }; // TODO: Consider making std::atomic<bool>
  std::atomic<float> makeupSmoothed_ {0.0f};

  using Filter = juce::dsp::ProcessorDuplicator<dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>>;
  std::unique_ptr<Filter> postHighPass_;

  std::unique_ptr<APCompressor> compressor_;
  std::unique_ptr<APTubeDistortion> tubeDistortion_;
  std::unique_ptr<APOverdrive> overdrive_;

  // Callback for DSP parameter changes
  void valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyChanged, const juce::Identifier& property) override
  {
    // Function parameters not needed for value tree state
    ignoreUnused(treeWhosePropertyChanged);
    ignoreUnused(property);

    mustUpdateProcessing_ = true;
  }
  //==============================================================================
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Ap_dynamicsAudioProcessor)
};
