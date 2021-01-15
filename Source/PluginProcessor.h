/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
*/
class Ap_dynamicsAudioProcessor  : public juce::AudioProcessor,
                                   public juce::ValueTree::Listener
{
public:
    //==============================================================================
    Ap_dynamicsAudioProcessor();
    ~Ap_dynamicsAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

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
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    enum ValType {
        sampleVal,
        gainsc
    };

    // ValueTree
    juce::AudioProcessorValueTreeState apvts;
    std::atomic<float> meterLocalMaxVal, meterGlobalMaxVal; // std::atomic to make value thread safe

    // Getters
    juce::Array<float> getInputBuffer() { return inputAmps_; }
    juce::Array<float> getOutputBuffer() { return outputAmps_; }
    int getMinDB() const { return mindB_; }
    float getCurrXdB() { return curr_xdB_; }
    float getCurrGainSC() { return curr_gainsc_; }
    bool getOverdrive() { return isOverdrived_; }
    juce::AudioBuffer<float>& getWaveForm() { return waveform_; };

    // Setters
    void setOverdrive (bool isOverdrived) { isOverdrived_ = isOverdrived; }
    void setCompTypeID (int id) { compTypeID_ = id; }

    // TODO: Lots of repeated code, refactor!
    float applyFFCompression (float sample, ValType type = ValType::sampleVal);
    float applyFBCompression (float sample, ValType type = ValType::sampleVal);
    float applyRMSCompression (float sample, ValType type = ValType::sampleVal);
    float applyPiecewiseOverdrive (float sample);
    void fillPlotBuffer (float x_dB, float gain_sc);
    void fillCCurve();
    // Passes the sample rate and buffer size to DSP
    void prepare (double sampleRate, int samplesPerBlock);
    // Updates DSP when user changes parameters
    void update();
    // Overrides AudioProcessor reset, reset DSP parameters
    void reset() override;
    // Create parameter layout for apvts
    juce::AudioProcessorValueTreeState::ParameterLayout createParameters();

private:
    bool mustUpdateProcessing_ { false }, isActive_ { false };

    juce::AudioBuffer<float> waveform_;
    juce::Array<float> inputAmps_, outputAmps_;
    int pBufferSize_ = 2048;
    float curr_xdB_, curr_gainsc_;

    const juce::StringArray styleChoices_ = { "Clean", "Dirty", "Dirtier" };
    const juce::StringArray compTypeChoices_ = { "Feedforward", "Feedback", "RMS" };

    float threshold_, ratio_, kneeWidth_, attack_, release_ = 0;
//    juce::LinearSmoothedValue<float> inputGain_ [2] { 0.0f };
    juce::LinearSmoothedValue<float> makeup_ [2];
    juce::IIRFilter tone_ [2];
    bool isOverdrived_ { false };
    int compTypeID_ { 0 };
    int mindB_ = -144;
    float prevGainSmooth_ = 0;
    float y_prev_ = 0;

    // Callback for DSP parameter changes
    void valueTreePropertyChanged (juce::ValueTree& treeWhosePropertyChanged, const juce::Identifier& property) override
    {
        mustUpdateProcessing_ = true;
    };
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Ap_dynamicsAudioProcessor)
};
