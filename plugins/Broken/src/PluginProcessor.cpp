#include "PluginProcessor.h"
#include "PluginEditor.h"

BrokenAudioProcessor::BrokenAudioProcessor()
    : AudioProcessor (BusesProperties()
                        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "Parameters", createParameterLayout())
{}

juce::AudioProcessorValueTreeState::ParameterLayout
BrokenAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "drive", 1 }, "Drive",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f), 30.0f,
        juce::AudioParameterFloatAttributes().withLabel (" %")));

    // Starve: 0 = clean/properly biased, 100 = fully starved (dying battery, velcro).
    // Skew concentrates the usable action in the lower half of the knob:
    // 9 o'clock (~30%) is where things really break up.
    juce::NormalisableRange<float> biasRange (0.0f, 100.0f, 0.1f);
    biasRange.setSkewForCentre (30.0f);
    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "bias", 1 }, "Starve", biasRange, 0.0f,
        juce::AudioParameterFloatAttributes().withLabel (" %")));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "output", 1 }, "Output",
        juce::NormalisableRange<float> (-20.0f, 6.0f, 0.1f), 0.0f,
        juce::AudioParameterFloatAttributes().withLabel (" dB")));

    return layout;
}

void BrokenAudioProcessor::prepareToPlay (double sampleRate, int)
{
    driveSmoothed.reset  (sampleRate, 0.02);
    starveSmoothed.reset   (sampleRate, 0.02);
    outputSmoothed.reset (sampleRate, 0.02);

    const float initDrive  = apvts.getRawParameterValue ("drive")->load() / 100.0f;
    const float initBias   = apvts.getRawParameterValue ("bias")->load()  / 100.0f;
    const float initOutput = juce::Decibels::decibelsToGain (apvts.getRawParameterValue ("output")->load());
    driveSmoothed.setCurrentAndTargetValue  (initDrive);
    starveSmoothed.setCurrentAndTargetValue   (initBias);
    outputSmoothed.setCurrentAndTargetValue (initOutput);

    // DC block: y[n] = x[n] - x[n-1] + alpha * y[n-1], fc ~30 Hz
    dcAlpha = 1.0f - (juce::MathConstants<float>::twoPi * 30.0f
                       / static_cast<float> (sampleRate));

    for (int ch = 0; ch < 2; ++ch)
    {
        dcXPrev[ch] = 0.0f;
        dcYPrev[ch] = 0.0f;
    }
}

bool BrokenAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    return layouts.getMainInputChannelSet() == layouts.getMainOutputChannelSet();
}

void BrokenAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                          juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    driveSmoothed.setTargetValue  (apvts.getRawParameterValue ("drive")->load() / 100.0f);
    starveSmoothed.setTargetValue   (apvts.getRawParameterValue ("bias")->load()  / 100.0f);
    outputSmoothed.setTargetValue (juce::Decibels::decibelsToGain (
                                       apvts.getRawParameterValue ("output")->load()));

    const int numChannels = buffer.getNumChannels();
    const int numSamples  = buffer.getNumSamples();

    // Maximum dead-zone threshold at bias=0.
    // Positive side: Q2 snapping from cutoff to saturation (hard, velcro character).
    // Negative side: Q2 backing out of saturation (softer response).
    constexpr float kMaxThreshold = 1.2f;
    constexpr float kAsymRatio    = 0.5f;
    constexpr float kSnapPos      = 8.0f;
    constexpr float kSnapNeg      = 3.0f;

    for (int i = 0; i < numSamples; ++i)
    {
        const float d = driveSmoothed.getNextValue();   // 0–1
        const float b = starveSmoothed.getNextValue();    // 0=dying, 1=healthy
        const float v = outputSmoothed.getNextValue();

        // Drive: audio taper, 1× to 10× gain (0% → unity, 100% → +20 dB)
        const float kDrive = 1.0f + d * d * 9.0f;

        const float posThresh = b * kMaxThreshold;
        const float negThresh = posThresh * kAsymRatio;

        for (int ch = 0; ch < numChannels && ch < 2; ++ch)
        {
            float& x = buffer.getWritePointer (ch)[i];
            x *= kDrive;

            float y;
            if (x > posThresh)
            {
                y = std::tanh (kSnapPos * (x - posThresh));
            }
            else if (x < -negThresh)
            {
                y = -std::tanh (kSnapNeg * (-x - negThresh));
            }
            else
            {
                // Dead zone: transistor in cutoff.
                // Audio-rate oscillation through this boundary is the velcro.
                y = 0.0f;
            }

            // DC block removes offset from the asymmetric dead zone
            const float yIn = y;
            y = yIn - dcXPrev[ch] + dcAlpha * dcYPrev[ch];
            dcXPrev[ch] = yIn;
            dcYPrev[ch] = y;

            x = y * v;
        }
    }
}

juce::AudioProcessorEditor* BrokenAudioProcessor::createEditor()
{
    return new BrokenAudioProcessorEditor (*this);
}

void BrokenAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    if (auto xml = state.createXml())
        copyXmlToBinary (*xml, destData);
}

void BrokenAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BrokenAudioProcessor();
}
