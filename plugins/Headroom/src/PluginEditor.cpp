#include "PluginEditor.h"

HeadroomAudioProcessorEditor::HeadroomAudioProcessorEditor (HeadroomAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    addAndMakeVisible (thresholdKnob);
    thresholdKnob.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    thresholdKnob.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    thresholdKnob.setLookAndFeel (&knobLAF);

    thresholdAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        processor.apvts, "threshold", thresholdKnob);

    setSize (200, 210);
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

    const auto labelColour = juce::Colour (0x60, 0x60, 0x60);

    // Plugin name
    g.setFont (juce::Font (juce::FontOptions().withHeight (13.0f).withStyle ("Bold")));
    g.setColour (labelColour);
    g.drawText ("HEADROOM", 0, 8, getWidth(), 20, juce::Justification::centred);

    // Knob label
    g.setFont (juce::Font (juce::FontOptions().withHeight (11.0f).withStyle ("Bold")));
    g.drawText ("THRESHOLD", 0, 158, getWidth(), 18, juce::Justification::centred);

    // Clip LED
    constexpr float kLedDiam = 10.0f;
    const float ledX = (getWidth() - kLedDiam) * 0.5f;
    constexpr float kLedY = 185.0f;
    const juce::Rectangle<float> led (ledX, kLedY, kLedDiam, kLedDiam);

    if (ledOn)
    {
        // Outer glow
        g.setColour (juce::Colour (255, 40, 40).withAlpha (0.25f));
        g.fillEllipse (led.expanded (4.0f));
        // Body
        g.setColour (juce::Colour (255, 40, 40));
        g.fillEllipse (led);
        // Specular highlight
        g.setColour (juce::Colours::white.withAlpha (0.45f));
        g.fillEllipse (led.reduced (3.0f).translated (0.0f, -0.8f));
    }
    else
    {
        g.setColour (juce::Colour (55, 12, 12));
        g.fillEllipse (led);
    }

    g.setColour (juce::Colour (20, 6, 6));
    g.drawEllipse (led, 1.0f);
}

void HeadroomAudioProcessorEditor::resized()
{
    thresholdKnob.setBounds (40, 30, 120, 120);
}

void HeadroomAudioProcessorEditor::timerCallback()
{
    if (processor.clipping.exchange (false, std::memory_order_relaxed))
        clipHoldTicks = 15;  // ~500 ms at 30 Hz

    const bool newLed = (clipHoldTicks > 0);
    if (clipHoldTicks > 0)
        --clipHoldTicks;

    if (newLed != ledOn)
    {
        ledOn = newLed;
        repaint();
    }
}
