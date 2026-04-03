#include "PluginEditor.h"

//==============================================================================
// LifeAudioProcessorEditor
//==============================================================================

LifeAudioProcessorEditor::LifeAudioProcessorEditor (LifeAudioProcessor& p)
    : AudioProcessorEditor (&p),
      noiseAttachment (p.apvts, "noise",       noiseKnob),
      thdAttachment   (p.apvts, "thd",         thdKnob),
      satAttachment   (p.apvts, "saturation",  satKnob),
      xfmrAttachment  (p.apvts, "transformer", xfmrKnob),
      wideToggle      (p.apvts, "wide",        "Wide")
{
    setSize (550, 230);

    // Knobs
    juce::Slider* knobs[] = { &noiseKnob, &thdKnob, &satKnob, &xfmrKnob };
    for (auto* knob : knobs)
    {
        knob->setSliderStyle (juce::Slider::RotaryVerticalDrag);
        knob->setTextBoxStyle (juce::Slider::NoTextBox, true, 0, 0);
        knob->setRotaryParameters (juce::MathConstants<float>::pi * 1.25f,
                                    juce::MathConstants<float>::pi * 2.75f,
                                    true);
        knob->setLookAndFeel (&blackKnobLAF);
        addAndMakeVisible (knob);
    }

    // Labels
    auto setupLabel = [this] (juce::Label& label, const char* text)
    {
        label.setText (text, juce::dontSendNotification);
        label.setFont (juce::Font (juce::FontOptions().withHeight (12.0f).withStyle ("Bold")));
        label.setColour (juce::Label::textColourId, juce::Colour (0xff111111));
        label.setJustificationType (juce::Justification::centred);
        addAndMakeVisible (label);
    };

    setupLabel (noiseLabel, "Noise");
    setupLabel (thdLabel,   "Harmonics");
    setupLabel (satLabel,   "Tube");
    setupLabel (xfmrLabel,  "Iron");

    // Wide pill
    addAndMakeVisible (wideToggle);
}

LifeAudioProcessorEditor::~LifeAudioProcessorEditor()
{
    juce::Slider* knobs[] = { &noiseKnob, &thdKnob, &satKnob, &xfmrKnob };
    for (auto* knob : knobs)
        knob->setLookAndFeel (nullptr);
}

void LifeAudioProcessorEditor::paint (juce::Graphics& g)
{
    corvid::paintPanelBackground (g, getWidth(), getHeight());

    // Plugin name
    g.setColour (juce::Colour (0xff111111));
    g.setFont (juce::Font (juce::FontOptions().withHeight (20.0f).withStyle ("Bold")));
    g.drawText ("Life", 12, 4, 150, 24, juce::Justification::bottomLeft);
}

void LifeAudioProcessorEditor::resized()
{
    // 4 knobs evenly spaced horizontally
    constexpr int numKnobs = 4;
    constexpr int knobR    = 48;   // 36 body + 12 tick margin
    constexpr int gap      = 4;
    constexpr int labelH   = 15;
    constexpr int labelW   = 100;

    const int knobGroupH = knobR * 2 + gap + labelH;
    const int knobCY     = (getHeight() - knobGroupH) / 2 + knobR - 8;

    constexpr int cx[numKnobs] = { 83, 220, 358, 495 };

    juce::Slider* knobs[]  = { &noiseKnob,  &thdKnob,  &satKnob,  &xfmrKnob  };
    juce::Label*  labels[] = { &noiseLabel, &thdLabel, &satLabel, &xfmrLabel };

    for (int i = 0; i < numKnobs; ++i)
    {
        knobs[i] ->setBounds (cx[i] - knobR,        knobCY - knobR,        knobR * 2,  knobR * 2);
        labels[i]->setBounds (cx[i] - labelW / 2,   knobCY + knobR + gap,  labelW,     labelH);
    }

    // Wide pill: top right
    wideToggle.setBounds (getWidth() - 60, 8, 48, 20);
}
