#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include <CorvidLookAndFeel.h>

//==============================================================================
class LocBoxAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit LocBoxAudioProcessorEditor (LocBoxAudioProcessor&);
    ~LocBoxAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    corvid::BlackKnobLookAndFeel blackKnobLAF;

    juce::Slider inputKnob, limitKnob, outputKnob;
    juce::Label  inputLabel, limitLabel, outputLabel;

    juce::AudioProcessorValueTreeState::SliderAttachment inputAttachment,
                                                         limitAttachment,
                                                         outputAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LocBoxAudioProcessorEditor)
};
