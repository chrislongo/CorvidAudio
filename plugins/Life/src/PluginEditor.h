#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include <CorvidLookAndFeel.h>

//==============================================================================
class LifeAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit LifeAudioProcessorEditor (LifeAudioProcessor&);
    ~LifeAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    corvid::BlackKnobLookAndFeel blackKnobLAF;

    // 4 knobs in a row
    juce::Slider noiseKnob, thdKnob, satKnob, xfmrKnob;
    juce::Label  noiseLabel, thdLabel, satLabel, xfmrLabel;

    juce::AudioProcessorValueTreeState::SliderAttachment noiseAttachment,
                                                          thdAttachment,
                                                          satAttachment,
                                                          xfmrAttachment;

    // Wide pill toggle
    corvid::PillToggle wideToggle;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LifeAudioProcessorEditor)
};
