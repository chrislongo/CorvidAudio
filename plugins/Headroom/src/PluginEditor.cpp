#include "PluginEditor.h"

HeadroomAudioProcessorEditor::HeadroomAudioProcessorEditor (HeadroomAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    addAndMakeVisible (thresholdKnob);
    thresholdKnob.setSliderStyle (juce::Slider::RotaryVerticalDrag);
    thresholdKnob.setTextBoxStyle (juce::Slider::NoTextBox, true, 0, 0);
    thresholdKnob.setRotaryParameters (juce::MathConstants<float>::pi * 1.25f,
                                        juce::MathConstants<float>::pi * 2.75f,
                                        true);
    thresholdKnob.setLookAndFeel (&knobLAF);

    thresholdAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        processor.apvts, "threshold", thresholdKnob);

    thresholdLabel.setText ("Threshold", juce::dontSendNotification);
    thresholdLabel.setFont (juce::Font (juce::FontOptions().withHeight (12.0f).withStyle ("Bold")));
    thresholdLabel.setColour (juce::Label::textColourId, juce::Colour (0xff111111));
    thresholdLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (thresholdLabel);

    setSize (200, 190);
    startTimerHz (30);
}

HeadroomAudioProcessorEditor::~HeadroomAudioProcessorEditor()
{
    thresholdKnob.setLookAndFeel (nullptr);
    stopTimer();
}

void HeadroomAudioProcessorEditor::paint (juce::Graphics& g)
{
    corvid::paintPanelBackground (g, getWidth(), getHeight());

    // Plugin name — same treatment as other Corvid plugins
    g.setColour (juce::Colour (0xff111111));
    g.setFont (juce::Font (juce::FontOptions().withHeight (20.0f).withStyle ("Bold")));
    g.drawText ("Headroom", 12, 4, 150, 24, juce::Justification::bottomLeft);

    // Clip LED — flat fill, colour interpolated for smooth fade
    constexpr float kLedDiam = 10.0f;
    const float ledX = (getWidth() - kLedDiam) * 0.5f;
    constexpr float kLedY = 168.0f;
    const juce::Rectangle<float> led (ledX, kLedY, kLedDiam, kLedDiam);

    const auto offColour = juce::Colour (55, 12, 12);
    const auto onColour  = juce::Colour (220, 30, 30);
    g.setColour (offColour.interpolatedWith (onColour, ledAlpha));
    g.fillEllipse (led);

    g.setColour (juce::Colour (20, 6, 6));
    g.drawEllipse (led, 1.0f);
}

void HeadroomAudioProcessorEditor::resized()
{
    constexpr int kKnobSize = 96;
    constexpr int kGap      = 4;
    constexpr int kLabelH   = 15;

    const int knobX = (getWidth() - kKnobSize) / 2;
    thresholdKnob.setBounds (knobX, 36, kKnobSize, kKnobSize);
    thresholdLabel.setBounds (0, 36 + kKnobSize + kGap, getWidth(), kLabelH);
}

void HeadroomAudioProcessorEditor::timerCallback()
{
    if (processor.clipping.exchange (false, std::memory_order_relaxed))
        clipHoldTicks = 15;  // ~500 ms at 30 Hz

    const float target = (clipHoldTicks > 0) ? 1.0f : 0.0f;
    if (clipHoldTicks > 0)
        --clipHoldTicks;

    const float newAlpha = ledAlpha + (target - ledAlpha) * 0.55f;

    if (std::abs (newAlpha - ledAlpha) > 0.004f)
    {
        ledAlpha = newAlpha;
        repaint();
    }
    else if (ledAlpha != target)
    {
        ledAlpha = target;
        repaint();
    }
}
