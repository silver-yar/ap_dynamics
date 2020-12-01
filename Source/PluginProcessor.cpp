/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
Ap_dynamicsAudioProcessor::Ap_dynamicsAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input", juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
, apvts (*this, nullptr, "Parameters", createParameters())
{
    apvts.state.addListener (this);
}

Ap_dynamicsAudioProcessor::~Ap_dynamicsAudioProcessor()
{
}

//==============================================================================
const juce::String Ap_dynamicsAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

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

double Ap_dynamicsAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int Ap_dynamicsAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int Ap_dynamicsAudioProcessor::getCurrentProgram()
{
    return 0;
}

void Ap_dynamicsAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String Ap_dynamicsAudioProcessor::getProgramName (int index)
{
    return {};
}

void Ap_dynamicsAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void Ap_dynamicsAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
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
bool Ap_dynamicsAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void Ap_dynamicsAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    if (!isActive_) return;
    if (mustUpdateProcessing_) update();

    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    auto numSamples = buffer.getNumSamples();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);

        for (int sample = 0; sample < numSamples; ++sample) {
            switch (compType_) {
                case feedfoward:
                    channelData[sample] = applyFFCompression(channelData[sample]);
                    break;
                case feedback:
                    channelData[sample] = applyFBCompression(channelData[sample]);
                    break;
                default:
                    break;
            }
        }
    }
}

//==============================================================================
bool Ap_dynamicsAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* Ap_dynamicsAudioProcessor::createEditor()
{
    return new Ap_dynamicsAudioProcessorEditor (*this);
}

//==============================================================================
void Ap_dynamicsAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // Save state information to xml -> binary to retrieve on startup
    juce::ValueTree copyState = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml = copyState.createXml();
    copyXmlToBinary (*xml, destData);
}

void Ap_dynamicsAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml = getXmlFromBinary (data, sizeInBytes);
    juce::ValueTree copyState = juce::ValueTree::fromXml (*xml);
    apvts.replaceState (copyState);
}

float Ap_dynamicsAudioProcessor::applyFFCompression(float sample)
{
    auto alphaA = exp(-log(9) / (getSampleRate() * attack_));
    auto alphaR = exp(-log(9) / (getSampleRate() * release_));

    float gainSmooth = 0;
    float gain_sc = 0;

    auto x_uni = abs(sample);
    auto x_dB = 20 * log10(x_uni);
    if (x_dB < -96)
        x_dB = -96;
    // Static Characteristics
    if (x_dB > (threshold_ + kneeWidth_ / 2))
        gain_sc = threshold_ + (x_dB - threshold_) / ratio_; // Perform downwards compression
    else if (x_dB > (threshold_ - kneeWidth_ / 2))
        gain_sc = x_dB + ((1 / ratio_ - 1) * powf((x_dB - threshold_ + kneeWidth_ / 2), 2)) / (2 * kneeWidth_);
    else
        gain_sc = x_dB;

    float gainChange_dB = gain_sc - x_dB;

    // Smooth gain change
    if (gainChange_dB < prevGainSmooth_) {
        // attack mode
        gainSmooth = ((1 - alphaA) * gainChange_dB) + (alphaA * prevGainSmooth_);
    } else {
        // release mode
        gainSmooth = ((1 - alphaR) * gainChange_dB) + (alphaR * prevGainSmooth_);
    }

    // Convert back to linear amplitude scalar
    auto lin_a = powf(10, gainSmooth / 20);
    float x_out = lin_a * sample;

    prevGainSmooth_ = gainSmooth;

    // Apply Compression
    return x_out;
}

float Ap_dynamicsAudioProcessor::applyFBCompression(float sample)
{
    auto alphaA = exp(-log(9) / (getSampleRate() * attack_));
    auto alphaR = exp(-log(9) / (getSampleRate() * release_));

    float gainSmooth = 0;
    float gain_sc = 0;

    auto y_uni = abs(y_prev_);
    auto y_dB = 20 * log10(y_uni);
    if (y_dB < -96)
        y_dB = -96;
    // Static Characteristics
    if (y_dB > (threshold_ + kneeWidth_ / 2))
        gain_sc = threshold_ + (y_dB - threshold_) / ratio_; // Perform downwards compression
    else if (y_dB > (threshold_ - kneeWidth_ / 2))
        gain_sc = y_dB + ((1 / ratio_ - 1) * powf((y_dB - threshold_ + kneeWidth_ / 2), 2)) / (2 * kneeWidth_); // Compression with ramp
    else
        gain_sc = y_dB;

    float gainChange_dB = gain_sc - y_dB;

    // Smooth gain change
    if (gainChange_dB < prevGainSmooth_) {
        // attack mode
        gainSmooth = ((1 - alphaA) * gainChange_dB) + (alphaA * prevGainSmooth_);
    } else {
        // release mode
        gainSmooth = ((1 - alphaR) * gainChange_dB) + (alphaR * prevGainSmooth_);
    }

    // Convert back to linear amplitude scalar
    auto lin_a = powf(10, gainSmooth / 20);
    float y_out = lin_a * sample;

    y_prev_ = y_out;
    prevGainSmooth_ = gainSmooth;

    // Apply Compression
    return y_out;
}

void Ap_dynamicsAudioProcessor::prepare(double sampleRate, int samplesPerBlock)
{
}

void Ap_dynamicsAudioProcessor::update()
{
    mustUpdateProcessing_ = false;
    
    threshold_ = apvts.getRawParameterValue("THR")->load();
    ratio_ = apvts.getRawParameterValue("RAT")->load();
    kneeWidth_ = apvts.getRawParameterValue("KW")->load();
    attack_ = apvts.getRawParameterValue("ATT")->load();
    release_ = apvts.getRawParameterValue("REL")->load();
}

void Ap_dynamicsAudioProcessor::reset()
{
    prevGainSmooth_ = 0;
    y_prev_ = 0;
}

juce::AudioProcessorValueTreeState::ParameterLayout Ap_dynamicsAudioProcessor::createParameters()
{
    // Create parameter layout for apvts
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> parameters;

    auto valueToTextFunction = [](float val, int len) { return juce::String(val, len); };
    auto textToValueFunction = [](const juce::String& text) { return text.getFloatValue(); };

    // **Threshold**
    parameters.emplace_back (std::make_unique<juce::AudioParameterFloat>(
            "THR",
            "Threshold",
            juce::NormalisableRange<float>(-96.0f, 0.0f, 0.1f),
            0.0f,
            "dBFS",
            juce::AudioProcessorParameter::genericParameter,
            valueToTextFunction,
            textToValueFunction
    ));
    // **Ratio**
    parameters.emplace_back (std::make_unique<juce::AudioParameterFloat>(
            "RAT",
            "Ratio",
            juce::NormalisableRange<float>(1.0f, 100.0f, 0.1f),
            0.0f,
            "",
            juce::AudioProcessorParameter::genericParameter,
            valueToTextFunction,
            textToValueFunction
    ));
    // **Knee Width**
    parameters.emplace_back (std::make_unique<juce::AudioParameterFloat>(
            "KW",
            "Knee Width",
            juce::NormalisableRange<float>(0.0f, 10.0f, 0.1f),
            6.0f,
            "dB",
            juce::AudioProcessorParameter::genericParameter,
            valueToTextFunction,
            textToValueFunction
    ));
    // **Attack**
    parameters.emplace_back (std::make_unique<juce::AudioParameterFloat>(
            "ATT",
            "Attack",
            juce::NormalisableRange<float>(0.0f, 2.0f, 0.01f),
            0.05f,
            "s",
            juce::AudioProcessorParameter::genericParameter,
            valueToTextFunction,
            textToValueFunction
    ));
    // **Release**
    parameters.emplace_back (std::make_unique<juce::AudioParameterFloat>(
            "REL",
            "Release",
            juce::NormalisableRange<float>(0.0f, 2.0f, 0.01f),
            0.25f,
            "s",
            juce::AudioProcessorParameter::genericParameter,
            valueToTextFunction,
            textToValueFunction
    ));

    return { parameters.begin(), parameters.end() };
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Ap_dynamicsAudioProcessor();
}
