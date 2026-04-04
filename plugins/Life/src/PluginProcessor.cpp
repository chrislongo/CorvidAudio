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

    const float baseNoise = noise / 100.0f;
    const float baseThd   = (thd / 100.0f) * 0.5f;
    const float baseXfmr  = xfmr / 100.0f;

    const int numChannels = buffer.getNumChannels();
    const int numSamples  = buffer.getNumSamples();

    // Wide mode: R channel (ch 1) gets fixed component-tolerance offsets.
    // L channel always uses nominal values. When Wide is off, both are nominal.
    const bool applyWide = wideOn && numChannels >= 2;

    // Noise sample channel count (may be mono or stereo)
    const int noiseCh = noiseSample.getNumChannels();

    // Per-channel harmonic ratios: L uses 70/30, R uses 62/38 when wide
    const float h2Ratio[2] = { 0.70f, applyWide ? kWideThdH2Ratio : 0.70f };
    const float h3Ratio[2] = { 0.30f, applyWide ? (1.0f - kWideThdH2Ratio) : 0.30f };

    for (int ch = 0; ch < numChannels && ch < 2; ++ch)
    {
        const auto chi = static_cast<size_t> (ch);

        // L = nominal, R = tolerance-offset when wide
        const bool isOffsetCh = applyWide && ch == 1;

        const float chNoise = baseNoise * (isOffsetCh ? kWideNoiseScale : 1.0f);
        const float chThd   = baseThd   * (isOffsetCh ? kWideThdScale   : 1.0f);
        // Only scale the variable drive portion so sat=0 stays at exactly 1.0 on both channels
        const float chSat   = 1.0f + (sat / 100.0f) * 3.0f * (isOffsetCh ? kWideSatScale : 1.0f);
        const float chXfmr  = baseXfmr  * (isOffsetCh ? kWideXfmrScale  : 1.0f);

        noiseLevelSmoothed[chi].setTargetValue (chNoise);
        thdLevelSmoothed[chi].setTargetValue (chThd);
        satLevelSmoothed[chi].setTargetValue (chSat);
        xfmrDriveSmoothed[chi].setTargetValue (chXfmr);

        float* data = buffer.getWritePointer (ch);

        // Pick the noise channel: use matching channel if stereo, else mono
        const float* noiseData = (noiseSampleLen > 0)
            ? noiseSample.getReadPointer (juce::jmin (ch, noiseCh - 1))
            : nullptr;

        const float chH2 = h2Ratio[ch < 2 ? ch : 0];
        const float chH3 = h3Ratio[ch < 2 ? ch : 0];
        const float chAsymmetry = isOffsetCh ? kWideSatAsymmetry : 0.0f;

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
            // L: 70% 2nd / 30% 3rd.  R (wide): 62% 2nd / 38% 3rd.
            if (tl > 1.0e-7f)
            {
                const float h2 = x * x * (x >= 0.0f ? 1.0f : -1.0f);
                const float h3 = x * x * x;
                x += tl * (chH2 * h2 + chH3 * h3);
            }

            // --- 3. Tube saturation ---
            // R (wide): slightly different bias point + extra asymmetry
            if (sl > 1.001f)
            {
                x = std::tanh (x * sl) / sl;
                x += (0.02f + chAsymmetry) * (sl - 1.0f) * x * x;
            }

            // --- 4. SSL-style console transformer ---
            // Always run the full chain to keep filter states warm.
            // Blend dry->wet by drive so the DC blocker is never suddenly applied.
            {
                const float g = 1.0f + drive * 2.0f;
                float wet = x * g;

                // Asymmetric saturation (even harmonic generation)
                wet += drive * 0.2f * wet * std::abs (wet);

                // Soft saturation, normalized for unity small-signal gain
                wet = std::tanh (wet) / g;

                // DC blocking HPF (~30 Hz)
                xfmrDcState[chi] += xfmrDcCoeff * (wet - xfmrDcState[chi]);
                wet -= xfmrDcState[chi];

                // HF rolloff (transformer inductance, increases with drive)
                xfmrLpfState[chi] += xfmrLpfCoeff * (wet - xfmrLpfState[chi]);
                const float lpfMix = drive * 0.3f;
                wet = wet * (1.0f - lpfMix) + xfmrLpfState[chi] * lpfMix;

                // At drive=0: pure dry (x). At drive=1: pure transformer (wet).
                x += drive * (wet - x);
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
