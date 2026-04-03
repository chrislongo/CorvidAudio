#pragma once

#include <JuceHeader.h>

class LifeAudioProcessor : public juce::AudioProcessor
{
public:
    LifeAudioProcessor();
    ~LifeAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Life"; }

    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return "Default"; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    juce::AudioProcessorValueTreeState apvts;

private:
    // SSL-style transformer state (per channel)
    float xfmrDcState[2]  { 0.0f, 0.0f };   // DC blocking HPF
    float xfmrLpfState[2] { 0.0f, 0.0f };   // HF rolloff LPF

    // Transformer filter coefficients
    float xfmrDcCoeff  = 0.0f;   // DC blocker (~30 Hz)
    float xfmrLpfCoeff = 0.0f;   // HF rolloff (~12 kHz)

    // Console noise sample (loaded from binary resource)
    juce::AudioBuffer<float> noiseSample;
    int noiseSampleLen = 0;
    int noiseReadPos[2] { 0, 0 };

    // Smoothed parameter values (per channel)
    std::array<juce::SmoothedValue<float>, 2> noiseLevelSmoothed;
    std::array<juce::SmoothedValue<float>, 2> thdLevelSmoothed;
    std::array<juce::SmoothedValue<float>, 2> satLevelSmoothed;
    std::array<juce::SmoothedValue<float>, 2> xfmrDriveSmoothed;

    // Random for wide mode per-channel variation (mean-reverting random walk)
    juce::Random rng;
    float wideNoiseVar[2]  { 1.0f, 1.0f };
    float wideThdVar[2]    { 1.0f, 1.0f };
    float wideSatVar[2]    { 1.0f, 1.0f };
    float wideXfmrVar[2]   { 1.0f, 1.0f };

    float piOverSr = 0.0f;

    void loadNoiseSample (double targetSampleRate);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LifeAudioProcessor)
};
