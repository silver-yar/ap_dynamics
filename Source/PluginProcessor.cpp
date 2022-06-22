/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"

#include "../Helpers/APDefines.h"
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
  ampConvolution_ = std::make_unique<APConvolution>();
//  tubeDistortion_ = std::make_unique<APTubeDistortion>();
  postHighPass_   = std::make_unique<Filter>();
  postLowPass_    = std::make_unique<Filter>();

  apvts.state.addListener(this);
}

Ap_dynamicsAudioProcessor::~Ap_dynamicsAudioProcessor() = default;

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
  if (sampleRate < 0 || samplesPerBlock < 0)
  {
    sampleRate = 44100;
  }

  auto channels = static_cast<uint32>(jmin(getMainBusNumInputChannels(), getMainBusNumOutputChannels()));
  dsp::ProcessSpec spec{ sampleRate, static_cast<uint32>(samplesPerBlock), channels };

  ampConvolution_->prepare(spec);
  postHighPass_->prepare(spec);
  postLowPass_->prepare(spec);

  auto rh = 0.98f;
  auto r1 = 0.8f;

  *postHighPass_->state = dsp::IIR::Coefficients<float>(1.0f, -2.0f, 1.0f, 1.0f, -2.0f * rh, powf(rh, 2.0f));
  *postLowPass_->state  = dsp::IIR::Coefficients<float>(1.0f - r1, 0.0f, 0.0f, 1.0f, -r1, 0.0f);

  mixBuffer_.setSize(static_cast<int>(channels), samplesPerBlock);
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
  juce::dsp::AudioBlock<float> block(buffer);
  juce::dsp::ProcessContextReplacing<float> context(block);

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

  // Mix Buffer Feeding
//  for (auto channel = 0; channel < numChannels; channel++)
//    mixBuffer_.copyFrom(channel, 0, buffer, channel, 0, numSamples);

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
//    const auto bufferMinMax = buffer.findMinMax(channel, 0, numSamples);

    compressor_->process(channelData, channelData, buffer.getNumSamples());  // comp -> ok
    //    overdrive_->process(channelData, channelData, buffer.getNumSamples());
//    mixBuffer_.copyFrom(channel, 0, buffer, channel, 0, numSamples);

//    tubeDistortion_->process(channelData, bufferMinMax.getStart(), bufferMinMax.getEnd(), 1.0f, distQ_, distChar_,
//                             channelData, buffer.getNumSamples());
  }
  ampConvolution_->process (context);

  // Post-Filtering
  postHighPass_->process(context);
  postLowPass_->process(context);

  // Mix Processing
//  dryGain_.applyGain(mixBuffer_, numSamples);
//  wetGain_.applyGain(buffer, numSamples);

  // -- MixBuffer Convolution
//  for (auto channel = 0; channel < numChannels; channel++)
//    buffer.addFrom(channel, 0, mixBuffer_,  channel, 0, numSamples);

  // Makeup
  makeup_.applyGain(buffer, numSamples);

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
  const auto mix        = apvts.getRawParameterValue("MIX")->load();
  dryGain_.setCurrentAndTargetValue(1.0f - mix);
  wetGain_.setCurrentAndTargetValue(mix);

  compressor_->updateParameters(apvts.getRawParameterValue("THR")->load(), apvts.getRawParameterValue("RAT")->load());

  distQ_.store(apvts.getRawParameterValue(APParameters::DISTQ_ID)->load());
  distChar_.store(apvts.getRawParameterValue(APParameters::DIST_CHAR_ID)->load());

  const auto makeup =
      juce::Decibels::decibelsToGain(apvts.getRawParameterValue(APParameters::MAKEUP_ID)->load(), APConstants::Math::MINUS_INF_DB);
  makeupSmoothed_ = makeupSmoothed_ - 0.004f * (makeupSmoothed_ - makeup);
  makeup_.setCurrentAndTargetValue(makeupSmoothed_);
}

void Ap_dynamicsAudioProcessor::reset()
{
  compressor_->reset();

  auto zero_f = 0.0f;
  meterLocalMaxVal.store(zero_f);
  meterGlobalMaxVal.store(zero_f);
  makeupSmoothed_.store(zero_f);
  distQ_.store(zero_f);
  distChar_.store(zero_f);
  mixBuffer_.applyGain(0.0f);
  dryGain_.reset(getSampleRate(), 0.05);
  wetGain_.reset(getSampleRate(), 0.05);
  makeup_.reset(getSampleRate(), 0.05);
}

juce::AudioProcessorValueTreeState::ParameterLayout Ap_dynamicsAudioProcessor::createParameters()
{
  // Create parameter layout for apvts
  std::vector<std::unique_ptr<juce::RangedAudioParameter>> parameters;

  const auto valueToTextFunction = [](float val, int len) { return juce::String(val, len); };
  const auto textToValueFunction = [](const juce::String& text) { return text.getFloatValue(); };

  // Threshold
  parameters.emplace_back(std::make_unique<juce::AudioParameterFloat>(
      APParameters::THRESHOLD_ID, APParameters::THRESHOLD_NAME,
      juce::NormalisableRange<float>(APParameters::THRESHOLD_START, APParameters::THRESHOLD_END,
                                     APParameters::THRESHOLD_INTERVAL),
      APParameters::THRESHOLD_DEFAULT, APParameters::THRESHOLD_SUFFIX, juce::AudioProcessorParameter::genericParameter,
      valueToTextFunction, textToValueFunction));
  // Ratio
  parameters.emplace_back(std::make_unique<juce::AudioParameterFloat>(
      APParameters::RATIO_ID, APParameters::RATIO_NAME,
      juce::NormalisableRange<float>(APParameters::RATIO_START, APParameters::RATIO_END, APParameters::RATIO_INTERVAL,
                                     APParameters::RATIO_SKEW),
      APParameters::RATIO_DEFAULT, APParameters::RATIO_SUFFIX, juce::AudioProcessorParameter::genericParameter,
      valueToTextFunction, textToValueFunction));
  // Global Mix
  parameters.emplace_back(std::make_unique<juce::AudioParameterFloat>(
      APParameters::MIX_ID, APParameters::MIX_NAME,
      juce::NormalisableRange<float>(APParameters::MIX_START, APParameters::MIX_END, APParameters::MIX_INTERVAL),
      APParameters::MIX_DEFAULT, APParameters::MIX_SUFFIX, juce::AudioProcessorParameter::genericParameter,
      valueToTextFunction, textToValueFunction));
  // Distortion Q
  parameters.emplace_back(std::make_unique<juce::AudioParameterFloat>(
      APParameters::DISTQ_ID, APParameters::DISTQ_NAME,
      juce::NormalisableRange<float>(APParameters::DISTQ_START, APParameters::DISTQ_END, APParameters::DISTQ_INTERVAL),
      APParameters::DISTQ_DEFAULT, APParameters::DISTQ_SUFFIX, juce::AudioProcessorParameter::genericParameter,
      valueToTextFunction, textToValueFunction));
  // Distortion Characteristic
  parameters.emplace_back(std::make_unique<juce::AudioParameterFloat>(
      APParameters::DIST_CHAR_ID, APParameters::DIST_CHAR_NAME,
      juce::NormalisableRange<float>(APParameters::DIST_CHAR_START, APParameters::DIST_CHAR_END,
                                     APParameters::DIST_CHAR_INTERVAL),
      APParameters::DIST_CHAR_DEFAULT, APParameters::DIST_CHAR_SUFFIX, juce::AudioProcessorParameter::genericParameter,
      valueToTextFunction, textToValueFunction));
  // Makeup
  parameters.emplace_back(std::make_unique<juce::AudioParameterFloat>(
      APParameters::MAKEUP_ID, APParameters::MAKEUP_NAME,
      juce::NormalisableRange<float>(APParameters::MAKEUP_START, APParameters::MAKEUP_END, APParameters::MAKEUP_INTERVAL),
      APParameters::MAKEUP_DEFAULT, APParameters::MAKEUP_SUFFIX, juce::AudioProcessorParameter::genericParameter,
      valueToTextFunction, textToValueFunction));

  return { parameters.begin(), parameters.end() };
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new Ap_dynamicsAudioProcessor(); }
