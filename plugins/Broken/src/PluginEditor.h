#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include <CorvidLookAndFeel.h>

class BrokenAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit BrokenAudioProcessorEditor (BrokenAudioProcessor&);
    ~BrokenAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    corvid::BlackKnobLookAndFeel knobLAF;

    juce::Slider driveKnob, starveKnob, mixKnob, outputKnob;
    juce::Label  driveLabel, starveLabel, mixLabel, outputLabel;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> driveAttachment,
                                                                           starveAttachment,
                                                                           mixAttachment,
                                                                           outputAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BrokenAudioProcessorEditor)
};
