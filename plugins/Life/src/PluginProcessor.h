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

    // Wide mode: fixed per-channel component tolerance offsets.
    // Models two slightly different analog channel strips.
    // L = reference channel, R = offset channel.
    static constexpr float kWideNoiseScale   = 1.03f;   // R preamp noise floor +3%
    static constexpr float kWideThdScale     = 1.04f;   // R harmonic level +4%
    static constexpr float kWideThdH2Ratio   = 0.62f;   // R: 62/38 2nd/3rd (vs L: 70/30)
    static constexpr float kWideSatScale     = 1.025f;  // R tube bias +2.5%
    static constexpr float kWideSatAsymmetry = 0.035f;  // R extra even-harmonic asymmetry
    static constexpr float kWideXfmrScale    = 1.03f;   // R transformer gain +3%

    float piOverSr = 0.0f;

    void loadNoiseSample (double targetSampleRate);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LifeAudioProcessor)
};
