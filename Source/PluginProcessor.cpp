/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"

#include "APDefines.h"
#include "PluginEditor.h"

//==============================================================================
Ap_dynamicsAudioProcessor::Ap_dynamicsAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if !JucePlugin_IsMidiEffect
#if !JucePlugin_IsSynth
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
                         )
#endif
      ,
      apvts(*this, nullptr, "Parameters", createParameters())
{
  compressor_     = std::make_unique<APCompressor>();
  tubeDistortion_ = std::make_unique<APTubeDistortion>();
  overdrive_      = std::make_unique<APOverdrive>();

  apvts.state.addListener(this);
}

Ap_dynamicsAudioProcessor::~Ap_dynamicsAudioProcessor() { }

//==============================================================================
const juce::String Ap_dynamicsAudioProcessor::getName() const { return JucePlugin_Name; }

bool Ap_dynamicsAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
  return true;
#else
  return false;
#endif
}

bool Ap_dynamicsAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
  return true;
#else
  return false;
#endif
}

bool Ap_dynamicsAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
  return true;
#else
  return false;
#endif
}

double Ap_dynamicsAudioProcessor::getTailLengthSeconds() const { return 0.0; }

int Ap_dynamicsAudioProcessor::getNumPrograms()
{
  return 1;  // NB: some hosts don't cope very well if you tell them there are 0 programs,
             // so this should be at least 1, even if you're not really implementing programs.
}

int Ap_dynamicsAudioProcessor::getCurrentProgram() { return 0; }

void Ap_dynamicsAudioProcessor::setCurrentProgram(int index)
{
  // Unused Parameters
  ignoreUnused(index);
}

const juce::String Ap_dynamicsAudioProcessor::getProgramName(int index)
{
  // Unused Parameters
  ignoreUnused(index);
  return {};
}

void Ap_dynamicsAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
  // Unused Parameters
  ignoreUnused(index);
  ignoreUnused(newName);
}

//==============================================================================
void Ap_dynamicsAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
  // Unused Parameters
  ignoreUnused(samplesPerBlock);

  compressor_->setSampleRate(static_cast<float>(sampleRate));
  update();
  reset();
  isActive_ = true;
}

void Ap_dynamicsAudioProcessor::releaseResources()
{
  // When playback stops, you can use this as an opportunity to free up any
  // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool Ap_dynamicsAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
  juce::ignoreUnused(layouts);
  return true;
#else
  // This is the place where you check if the layout is supported.
  // In this template code we only support mono or stereo.
  if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() &&
      layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
    return false;

    // This checks if the input layout matches the output layout
#if !JucePlugin_IsSynth
  if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
    return false;
#endif

  return true;
#endif
}
#endif

void Ap_dynamicsAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
  // Unused Parameters
  ignoreUnused(midiMessages);

  if (!isActive_)
    return;
  if (mustUpdateProcessing_)
    update();

  juce::ScopedNoDenormals noDenormals;
  const auto totalNumInputChannels  = getTotalNumInputChannels();
  const auto totalNumOutputChannels = getTotalNumOutputChannels();
  const auto numChannels            = juce::jmin(totalNumInputChannels, totalNumOutputChannels);
  const auto numSamples             = buffer.getNumSamples();

  auto sumMaxVal     = 0.0f;
  auto currentMaxVal = meterGlobalMaxVal.load();

  for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
  {
    buffer.clear(i, 0, numSamples);
  }

  for (int channel = 0; channel < totalNumInputChannels; ++channel)
  {
    auto* channelData = buffer.getWritePointer(channel);

    {
      // Find the channel max gain value
      auto channelMaxVal = 0.0f;

      for (int sample = 0; sample < numSamples; ++sample)
      {
        auto rectifiedVal = std::abs(channelData[sample]);
        if (channelMaxVal < rectifiedVal)
          channelMaxVal = rectifiedVal;
        if (currentMaxVal < rectifiedVal)
          currentMaxVal = rectifiedVal;
      }

      sumMaxVal += channelMaxVal;  // Sum of channel 0 and channel 1 max values

      meterGlobalMaxVal = currentMaxVal;
    }

    // Find the buffer's max magnitude
    const auto bufferMinMax = buffer.findMinMax(channel, 0, numSamples);
    const auto minMag       = abs(bufferMinMax.getStart());
    const auto maxMag       = abs(bufferMinMax.getEnd());
    const auto bufferMaxVal = juce::jmax(minMag, maxMag);

    // DSP Processing
    compressor_->process(channelData, channelData, buffer.getNumSamples());
    overdrive_->process(channelData, mix_, channelData, buffer.getNumSamples());
    tubeDistortion_->processDAFX(channelData, bufferMaxVal, 1.0f, -0.2f, 4.0f, mix_, channelData, buffer.getNumSamples());

    // Makeup
    for (int sample = 0; sample < numSamples; ++sample)
    {
      channelData[sample] *= makeupSmoothed_;
    }
  }

  meterLocalMaxVal = sumMaxVal / static_cast<float>(numChannels);
}

//==============================================================================
bool Ap_dynamicsAudioProcessor::hasEditor() const
{
  return true;  // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* Ap_dynamicsAudioProcessor::createEditor() { return new Ap_dynamicsAudioProcessorEditor(*this); }

//==============================================================================
void Ap_dynamicsAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
  // Save state information to xml -> binary to retrieve on startup
  const auto copyState = apvts.copyState();
  auto xml             = copyState.createXml();
  copyXmlToBinary(*xml, destData);
}

void Ap_dynamicsAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
  auto xml             = getXmlFromBinary(data, sizeInBytes);
  const auto copyState = juce::ValueTree::fromXml(*xml);
  apvts.replaceState(copyState);
}

void Ap_dynamicsAudioProcessor::update()
{
  mustUpdateProcessing_ = false;

  compressor_->updateParameters(apvts.getRawParameterValue("THR")->load(), apvts.getRawParameterValue("RAT")->load());
  mix_ = apvts.getRawParameterValue("MIX")->load();

  const auto makeup =
      juce::Decibels::decibelsToGain(apvts.getRawParameterValue("MUP")->load(), AP::Constants::minusInfinityDb);
  makeupSmoothed_ = makeupSmoothed_ - 0.004f * (makeupSmoothed_ - makeup);
}

void Ap_dynamicsAudioProcessor::reset()
{
  compressor_->reset();

  auto zero_f = 0.0f;
  meterLocalMaxVal.store(zero_f);
  meterGlobalMaxVal.store(zero_f);
  makeupSmoothed_.store(zero_f);
}

juce::AudioProcessorValueTreeState::ParameterLayout Ap_dynamicsAudioProcessor::createParameters()
{
  // Create parameter layout for apvts
  std::vector<std::unique_ptr<juce::RangedAudioParameter>> parameters;

  const auto valueToTextFunction = [](float val, int len) { return juce::String(val, len); };
  const auto textToValueFunction = [](const juce::String& text) { return text.getFloatValue(); };

  // **Threshold**
  parameters.emplace_back(std::make_unique<juce::AudioParameterFloat>(
      "THR", "Threshold", juce::NormalisableRange<float>(-96.0f, 0.0f, 0.1f), 0.0f, "dBFS",
      juce::AudioProcessorParameter::genericParameter, valueToTextFunction, textToValueFunction));
  // **Ratio**
  parameters.emplace_back(std::make_unique<juce::AudioParameterFloat>(
      "RAT", "Ratio", juce::NormalisableRange<float>(1.0f, 100.0f, 0.1f, 0.3f), 1.0f, "",
      juce::AudioProcessorParameter::genericParameter, valueToTextFunction, textToValueFunction));
  parameters.emplace_back(std::make_unique<juce::AudioParameterFloat>(
      "MIX", "Global Mix", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.0f, "%",
      juce::AudioProcessorParameter::genericParameter, valueToTextFunction, textToValueFunction));
  parameters.emplace_back(std::make_unique<juce::AudioParameterFloat>(
      "DSQ", "Distortion Q", juce::NormalisableRange<float>(-1.0f, 1.0f, 0.1f), 0.0f, "",
      juce::AudioProcessorParameter::genericParameter, valueToTextFunction, textToValueFunction));
  parameters.emplace_back(std::make_unique<juce::AudioParameterFloat>(
      "MUP", "Makeup", juce::NormalisableRange<float>(-40.0f, 40.0f, 1.0f), 6.0f, "dB",
      juce::AudioProcessorParameter::genericParameter, valueToTextFunction, textToValueFunction));

  return { parameters.begin(), parameters.end() };
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new Ap_dynamicsAudioProcessor(); }
