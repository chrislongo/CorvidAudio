#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "BinaryData.h"

LifeAudioProcessor::LifeAudioProcessor()
    : AudioProcessor (BusesProperties()
                        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "Parameters", createParameterLayout())
{
}

LifeAudioProcessor::~LifeAudioProcessor() {}

juce::AudioProcessorValueTreeState::ParameterLayout
LifeAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // Noise level
    juce::NormalisableRange<float> noiseRange (0.0f, 100.0f, 0.01f);
    noiseRange.setSkewForCentre (20.0f);
    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "noise", 1 }, "Noise", noiseRange, 0.0f,
        juce::AudioParameterFloatAttributes().withLabel (" %")));

    // Harmonics (THD) level
    juce::NormalisableRange<float> thdRange (0.0f, 100.0f, 0.01f);
    thdRange.setSkewForCentre (20.0f);
    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "thd", 1 }, "Harmonics", thdRange, 0.0f,
        juce::AudioParameterFloatAttributes().withLabel (" %")));

    // Tube saturation level
    juce::NormalisableRange<float> satRange (0.0f, 100.0f, 0.01f);
    satRange.setSkewForCentre (25.0f);
    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "saturation", 1 }, "Tube", satRange, 0.0f,
        juce::AudioParameterFloatAttributes().withLabel (" %")));

    // Transformer drive (SSL-style console transformer)
    juce::NormalisableRange<float> xfmrRange (0.0f, 100.0f, 0.01f);
    xfmrRange.setSkewForCentre (25.0f);
    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "transformer", 1 }, "Iron", xfmrRange, 0.0f,
        juce::AudioParameterFloatAttributes().withLabel (" %")));

    // Wide mode on/off
    layout.add (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { "wide", 1 }, "Wide", true));

    return layout;
}

void LifeAudioProcessor::loadNoiseSample (double targetSampleRate)
{
    juce::AudioFormatManager formatManager;
    formatManager.registerBasicFormats();

    auto* reader = formatManager.createReaderFor (
        std::make_unique<juce::MemoryInputStream> (
            BinaryData::Noise_aif, BinaryData::Noise_aifSize, false));

    if (reader == nullptr)
        return;

    // Read entire file
    juce::AudioBuffer<float> fileBuffer ((int) reader->numChannels,
                                          (int) reader->lengthInSamples);
    reader->read (&fileBuffer, 0, (int) reader->lengthInSamples, 0, true, true);

    const double fileSr = reader->sampleRate;
    delete reader;

    // Resample if needed
    if (std::abs (fileSr - targetSampleRate) > 1.0)
    {
        const double ratio = targetSampleRate / fileSr;
        const int newLen = (int) (fileBuffer.getNumSamples() * ratio);

        juce::AudioBuffer<float> resampled (fileBuffer.getNumChannels(), newLen);

        for (int ch = 0; ch < fileBuffer.getNumChannels(); ++ch)
        {
            juce::LagrangeInterpolator interpolator;
            interpolator.reset();
            interpolator.process (1.0 / ratio,
                                  fileBuffer.getReadPointer (ch),
                                  resampled.getWritePointer (ch),
                                  newLen);
        }

        noiseSample = std::move (resampled);
    }
    else
    {
        noiseSample = std::move (fileBuffer);
    }

    noiseSampleLen = noiseSample.getNumSamples();
}

void LifeAudioProcessor::prepareToPlay (double sampleRate, int)
{
    const float sr = static_cast<float> (sampleRate);
    piOverSr = juce::MathConstants<float>::pi / sr;

    // Transformer DC blocker at ~30 Hz
    const float dcK = std::tan (piOverSr * 30.0f);
    xfmrDcCoeff = dcK / (1.0f + dcK);

    // Transformer HF rolloff at ~12 kHz
    const float lpfK = std::tan (piOverSr * 12000.0f);
    xfmrLpfCoeff = lpfK / (1.0f + lpfK);

    // Load console noise sample
    loadNoiseSample (sampleRate);

    // Offset channel read positions for decorrelation
    noiseReadPos[0] = 0;
    noiseReadPos[1] = noiseSampleLen / 3;

    const float noise = apvts.getRawParameterValue ("noise")->load();
    const float thd   = apvts.getRawParameterValue ("thd")->load();
    const float sat   = apvts.getRawParameterValue ("saturation")->load();
    const float xfmr  = apvts.getRawParameterValue ("transformer")->load();

    const float initNoise = noise / 100.0f;
    const float initThd   = (thd / 100.0f) * 0.5f;
    const float initSat   = 1.0f + (sat / 100.0f) * 3.0f;
    const float initXfmr  = xfmr / 100.0f;

    for (size_t ch = 0; ch < 2; ++ch)
    {
        noiseLevelSmoothed[ch].reset (sampleRate, 0.08);
        noiseLevelSmoothed[ch].setCurrentAndTargetValue (initNoise);

        thdLevelSmoothed[ch].reset (sampleRate, 0.08);
        thdLevelSmoothed[ch].setCurrentAndTargetValue (initThd);

        satLevelSmoothed[ch].reset (sampleRate, 0.08);
        satLevelSmoothed[ch].setCurrentAndTargetValue (initSat);

        xfmrDriveSmoothed[ch].reset (sampleRate, 0.08);
        xfmrDriveSmoothed[ch].setCurrentAndTargetValue (initXfmr);

        xfmrDcState[ch]  = 0.0f;
        xfmrLpfState[ch] = 0.0f;
    }
}

void LifeAudioProcessor::releaseResources() {}

bool LifeAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    return layouts.getMainInputChannelSet() == layouts.getMainOutputChannelSet();
}

void LifeAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                        juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const bool wideOn = apvts.getRawParameterValue ("wide")->load() >= 0.5f;

    const float noise = apvts.getRawParameterValue ("noise")->load();
    const float thd   = apvts.getRawParameterValue ("thd")->load();
    const float sat   = apvts.getRawParameterValue ("saturation")->load();
    const float xfmr  = apvts.getRawParameterValue ("transformer")->load();

    const float targetNoise = noise / 100.0f;
    const float targetThd   = (thd / 100.0f) * 0.5f;
    const float targetSat   = 1.0f + (sat / 100.0f) * 3.0f;
    const float targetXfmr  = xfmr / 100.0f;

    const int numChannels = buffer.getNumChannels();
    const int numSamples  = buffer.getNumSamples();

    // Wide mode: continuous mean-reverting random walk per channel
    if (wideOn && numChannels >= 2)
    {
        constexpr float drift  = 0.002f;  // random nudge per block
        constexpr float revert = 0.005f;  // pull toward 1.0

        auto walk = [&] (float& var, float range) {
            var += (rng.nextFloat() * 2.0f - 1.0f) * drift;
            var += (1.0f - var) * revert;
            var = juce::jlimit (1.0f - range, 1.0f + range, var);
        };

        for (int c = 0; c < 2; ++c)
        {
            walk (wideNoiseVar[c], 0.05f);
            walk (wideThdVar[c],   0.04f);
            walk (wideSatVar[c],   0.03f);
            walk (wideXfmrVar[c],  0.03f);
        }
    }
    else
    {
        // Smooth convergence back to unity
        constexpr float revert = 0.02f;
        for (int c = 0; c < 2; ++c)
        {
            wideNoiseVar[c] += (1.0f - wideNoiseVar[c]) * revert;
            wideThdVar[c]   += (1.0f - wideThdVar[c])   * revert;
            wideSatVar[c]   += (1.0f - wideSatVar[c])    * revert;
            wideXfmrVar[c]  += (1.0f - wideXfmrVar[c])  * revert;
        }
    }

    // Noise sample channel count (may be mono or stereo)
    const int noiseCh = noiseSample.getNumChannels();

    for (int ch = 0; ch < numChannels && ch < 2; ++ch)
    {
        const auto chi = static_cast<size_t> (ch);

        noiseLevelSmoothed[chi].setTargetValue (targetNoise * wideNoiseVar[ch]);
        thdLevelSmoothed[chi].setTargetValue (targetThd * wideThdVar[ch]);
        satLevelSmoothed[chi].setTargetValue (targetSat * wideSatVar[ch]);
        xfmrDriveSmoothed[chi].setTargetValue (targetXfmr * wideXfmrVar[ch]);

        float* data = buffer.getWritePointer (ch);

        // Pick the noise channel: use matching channel if stereo, else mono
        const float* noiseData = (noiseSampleLen > 0)
            ? noiseSample.getReadPointer (juce::jmin (ch, noiseCh - 1))
            : nullptr;

        for (int i = 0; i < numSamples; ++i)
        {
            const float nl    = noiseLevelSmoothed[chi].getNextValue();
            const float tl    = thdLevelSmoothed[chi].getNextValue();
            const float sl    = satLevelSmoothed[chi].getNextValue();
            const float drive = xfmrDriveSmoothed[chi].getNextValue();

            float x = data[i];

            // --- 1. Console noise ---
            if (nl > 1.0e-7f && noiseData != nullptr)
            {
                x += noiseData[noiseReadPos[ch]] * nl;
                noiseReadPos[ch]++;
                if (noiseReadPos[ch] >= noiseSampleLen)
                    noiseReadPos[ch] = 0;
            }

            // --- 2. Harmonics (THD) ---
            if (tl > 1.0e-7f)
            {
                const float h2 = x * x * (x >= 0.0f ? 1.0f : -1.0f);
                const float h3 = x * x * x;
                x += tl * (0.7f * h2 + 0.3f * h3);
            }

            // --- 3. Tube saturation ---
            if (sl > 1.001f)
            {
                x = std::tanh (x * sl) / sl;
                x += 0.02f * (sl - 1.0f) * x * x;
            }

            // --- 4. SSL-style console transformer ---
            if (drive > 1.0e-5f)
            {
                // Gain staging
                const float g = 1.0f + drive * 2.0f;
                x *= g;

                // Asymmetric saturation (even harmonic generation)
                x += drive * 0.2f * x * std::abs (x);

                // Soft saturation (transformer core)
                x = std::tanh (x) / std::tanh (g);

                // DC blocking HPF (~30 Hz)
                xfmrDcState[chi] += xfmrDcCoeff * (x - xfmrDcState[chi]);
                x -= xfmrDcState[chi];

                // HF rolloff (transformer inductance, increases with drive)
                xfmrLpfState[chi] += xfmrLpfCoeff * (x - xfmrLpfState[chi]);
                const float lpfMix = drive * 0.3f;
                x = x * (1.0f - lpfMix) + xfmrLpfState[chi] * lpfMix;
            }
            else
            {
                // Keep filter states tracking to avoid clicks when engaging
                xfmrDcState[chi]  += xfmrDcCoeff  * (x - xfmrDcState[chi]);
                xfmrLpfState[chi] += xfmrLpfCoeff * (x - xfmrLpfState[chi]);
            }

            data[i] = x;
        }
    }
}

juce::AudioProcessorEditor* LifeAudioProcessor::createEditor()
{
    return new LifeAudioProcessorEditor (*this);
}

void LifeAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    if (auto xml = state.createXml())
        copyXmlToBinary (*xml, destData);
}

void LifeAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new LifeAudioProcessor();
}
