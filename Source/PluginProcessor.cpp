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
    inputAmps_.resize(pBufferSize_);
    outputAmps_.resize(pBufferSize_);
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
    auto numChannels = juce::jmin (totalNumInputChannels, totalNumOutputChannels);
    auto numSamples = buffer.getNumSamples();

    auto sumMaxVal = 0.0f;
    auto currentMaxVal = meterGlobalMaxVal.load();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i) {
        buffer.clear(i, 0, buffer.getNumSamples());
//        inputAmps_.clear(i, 0, inputAmps_.getNumSamples());
//        outputAmps_.clear(i, 0, outputAmps_.getNumSamples());
    }


    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);
//        auto* input = inputAmps_.getWritePointer(channel);
//        auto* output = outputAmps_.getWritePointer(channel);
        auto channelMaxVal = 0.0f;

        for (int sample = 0; sample < numSamples; ++sample) {
//            input[sample] = channelData[sample];
            switch ((int) apvts.getRawParameterValue("CT")->load()) {
                case 0:
                    channelData[sample] = applyFFCompression(channelData[sample]);
                    break;
                case 1:
                    channelData[sample] = applyFBCompression(channelData[sample]);
                    break;
                case 2:
                    channelData[sample] = applyRMSCompression(channelData[sample]);
                    break;
                default:
                    break;
            }
        }

        tone_[channel].processSamples (channelData, numSamples);
        makeup_[channel].applyGain (channelData, numSamples);

        // Find max value in buffer channel
        for (int sample = 0; sample < numSamples; ++sample)
        {
            channelData[sample] = applyPiecewiseOverdrive(channelData[sample]);

            auto rectifiedVal = std::abs (channelData[sample]);
            if (channelMaxVal < rectifiedVal) channelMaxVal = rectifiedVal;
            if (currentMaxVal < rectifiedVal) currentMaxVal = rectifiedVal;
        }

//        makeup_[channel].applyGain (channelData, numSamples);

        sumMaxVal += channelMaxVal; // Sum of channel 0 and channel 1 max values

        meterGlobalMaxVal.store (currentMaxVal);
    }

    meterLocalMaxVal.store (sumMaxVal / (float) numChannels);
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
    if (x_dB < mindB_)
        x_dB = mindB_;
    // Static Characteristics
    if (x_dB > (threshold_ + kneeWidth_ / 2))
        gain_sc = threshold_ + (x_dB - threshold_) / ratio_; // Perform downwards compression
    else if (x_dB > (threshold_ - kneeWidth_ / 2))
        gain_sc = x_dB + ((1 / ratio_ - 1) * powf((x_dB - threshold_ + kneeWidth_ / 2), 2)) / (2 * kneeWidth_);
    else
        gain_sc = x_dB;

    fillPlotBuffer(x_dB, gain_sc);

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
    if (y_dB < mindB_)
        y_dB = mindB_;
    // Static Characteristics
    if (y_dB > (threshold_ + kneeWidth_ / 2))
        gain_sc = threshold_ + (y_dB - threshold_) / ratio_; // Perform downwards compression
    else if (y_dB > (threshold_ - kneeWidth_ / 2))
        gain_sc = y_dB + ((1 / ratio_ - 1) * powf((y_dB - threshold_ + kneeWidth_ / 2), 2)) / (2 * kneeWidth_); // Compression with ramp
    else
        gain_sc = y_dB;

    fillPlotBuffer(y_dB, gain_sc);

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

float Ap_dynamicsAudioProcessor::applyRMSCompression(float sample)
{
    auto alphaA = exp(-log(9) / (getSampleRate() * attack_));
    auto alphaR = exp(-log(9) / (getSampleRate() * release_));

    float gainSmooth = 0;
    float gain_sc = 0;

    auto x_uni = abs(sample);
    auto x_dB = 20 * log10(x_uni);
    if (x_dB < mindB_)
        x_dB = mindB_;
    // Static Characteristics
    if (x_dB > (threshold_ + kneeWidth_ / 2))
        gain_sc = threshold_ + (x_dB - threshold_) / ratio_; // Perform downwards compression
    else if (x_dB > (threshold_ - kneeWidth_ / 2))
        gain_sc = x_dB + ((1 / ratio_ - 1) * powf((x_dB - threshold_ + kneeWidth_ / 2), 2)) / (2 * kneeWidth_);
    else
        gain_sc = x_dB;

    fillPlotBuffer(x_dB, gain_sc);

    float gainChange_dB = gain_sc - x_dB;

    // Smooth gain change (RMS Approximation)
    if (gainChange_dB < prevGainSmooth_) {
        // attack mode
        gainSmooth = -sqrt(((1 - alphaA) * powf(gainChange_dB,2))
                + (alphaA * powf(prevGainSmooth_, 2)));
    } else {
        // release mode
        gainSmooth = -sqrt(((1 - alphaR) * powf(gainChange_dB,2))
                           + (alphaR * powf(prevGainSmooth_, 2)));
    }

    // Convert back to linear amplitude scalar
    auto lin_a = powf(10, gainSmooth / 20);
    float x_out = lin_a * sample;

    prevGainSmooth_ = gainSmooth;

    // Apply Compression
    return x_out;
}

float Ap_dynamicsAudioProcessor::applyPiecewiseOverdrive(float sample) {
    float x_uni = abs(sample);
    float out = 0;

    if (x_uni <= 1/3)
    {
        out = 2 * sample;
    }
    if (x_uni >= 2/3)
    {
        out = sin(sample);
    }
    else {
        out = sin(sample) * (3 - powf(2 - 3 * x_uni, 2)) / 3;
    }
    return out;
}

void Ap_dynamicsAudioProcessor::fillPlotBuffer(float x_dB, float gain_sc)
{
    if (counter_ < pBufferSize_)
    {
        inputAmps_.set(counter_, x_dB);
        outputAmps_.set(counter_, gain_sc);
    } else {
        counter_ = 0;
        inputAmps_.set(counter_, x_dB);
        outputAmps_.set(counter_, gain_sc);
    }
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

    for (int channel = 0; channel < 2; ++channel) {
        tone_[channel].setCoefficients (juce::IIRCoefficients::makeLowPass (
                getSampleRate(), apvts.getRawParameterValue("TN")->load())
                );
        makeup_[channel].setTargetValue(juce::Decibels::decibelsToGain(
                apvts.getRawParameterValue("MU")->load()
                ));
    }
}

void Ap_dynamicsAudioProcessor::reset()
{
    prevGainSmooth_ = 0;
    y_prev_ = 0;

    for (int channel = 0; channel < 2; ++channel) {
        tone_[channel].reset();
        makeup_[channel].reset(getSampleRate(), 0.050);
    }

    meterLocalMaxVal.store (0.0f);
    meterGlobalMaxVal.store (0.0f);
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
    // **Dynamic Range Processor Type**
    parameters.emplace_back(std::make_unique<juce::AudioParameterChoice>(
            "DRT",
            "Dynamic Range Type",
            juce::StringArray { "Compressor", "Expander" },
            0
            ));
    // **Compression Type**
    parameters.emplace_back(std::make_unique<juce::AudioParameterChoice>(
            "CT",
            "Compression Type",
            juce::StringArray { "Feedforward", "Feedback", "RMS" },
            0
    ));
    // **Makeup Gain Parameter** - in dB
    parameters.emplace_back (std::make_unique<juce::AudioParameterFloat>(
            "MU",
            "Makeup",
            juce::NormalisableRange<float>(-40.0f, 40.0f,0.01f),
            0.0f,
            "dB",
            juce::AudioProcessorParameter::genericParameter,
            valueToTextFunction,
            textToValueFunction
    ));
    // **Low Pass Filter Parameter** - in Hz
    parameters.emplace_back (std::make_unique<juce::AudioParameterFloat>(
            "TN",
            "Tone",
            juce::NormalisableRange<float>(20.0f, 22000.0f, 10.0f, 0.2f),
            22000.0f,
            "Hz",
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
