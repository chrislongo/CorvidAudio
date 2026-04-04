#pragma once
#include <JuceHeader.h>
#include <CorvidLookAndFeel.h>
#include "PluginProcessor.h"

class HeadroomAudioProcessorEditor : public juce::AudioProcessorEditor,
                                      private juce::Timer
{
public:
    explicit HeadroomAudioProcessorEditor (HeadroomAudioProcessor&);
    ~HeadroomAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

    HeadroomAudioProcessor& processor;

    corvid::BlackKnobLookAndFeel knobLAF;
    juce::Slider thresholdKnob;
    juce::Label  thresholdLabel;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> thresholdAttachment;

    int   clipHoldTicks = 0;
    float ledAlpha      = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HeadroomAudioProcessorEditor)
};
