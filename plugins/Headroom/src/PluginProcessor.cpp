#include "PluginProcessor.h"
#include "PluginEditor.h"

HeadroomAudioProcessor::HeadroomAudioProcessor()
    : AudioProcessor (BusesProperties()
                        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "Parameters", createParameterLayout())
{}

juce::AudioProcessorValueTreeState::ParameterLayout
HeadroomAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // -30 to 0 dBFS, default 0 (no clipping). Skewed so mid-knob ≈ -12 dBFS.
    juce::NormalisableRange<float> threshRange (-30.0f, 0.0f, 0.1f);
    threshRange.setSkewForCentre (-12.0f);

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "threshold", 1 }, "Threshold", threshRange, 0.0f,
        juce::AudioParameterFloatAttributes().withLabel (" dB")));

    return layout;
}

void HeadroomAudioProcessor::prepareToPlay (double sampleRate, int)
{
    thresholdSmoothed.reset (sampleRate, 0.02);
    const float initThresh = juce::Decibels::decibelsToGain (apvts.getRawParameterValue ("threshold")->load());
    thresholdSmoothed.setCurrentAndTargetValue (initThresh);
}

bool HeadroomAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    return layouts.getMainInputChannelSet() == layouts.getMainOutputChannelSet();
}

void HeadroomAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                             juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const float targetThresh = juce::Decibels::decibelsToGain (apvts.getRawParameterValue ("threshold")->load());
    thresholdSmoothed.setTargetValue (targetThresh);

    const int numChannels = buffer.getNumChannels();
    const int numSamples  = buffer.getNumSamples();

    for (int i = 0; i < numSamples; ++i)
    {
        const float t = thresholdSmoothed.getNextValue();

        for (int ch = 0; ch < numChannels; ++ch)
        {
            float& x = buffer.getWritePointer (ch)[i];

            if (x > t)
            {
                x = t;
                clipping.store (true, std::memory_order_relaxed);
            }
            else if (x < -t)
            {
                x = -t;
                clipping.store (true, std::memory_order_relaxed);
            }
        }
    }
}

juce::AudioProcessorEditor* HeadroomAudioProcessor::createEditor()
{
    return new HeadroomAudioProcessorEditor (*this);
}

void HeadroomAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    if (auto xml = state.createXml())
        copyXmlToBinary (*xml, destData);
}

void HeadroomAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new HeadroomAudioProcessor();
}
