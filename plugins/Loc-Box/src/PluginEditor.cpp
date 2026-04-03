#include "PluginEditor.h"

//==============================================================================
// LocBoxAudioProcessorEditor
//==============================================================================

LocBoxAudioProcessorEditor::LocBoxAudioProcessorEditor (LocBoxAudioProcessor& p)
    : AudioProcessorEditor (&p),
      inputAttachment  (p.apvts, "input",  inputKnob),
      limitAttachment  (p.apvts, "limit",  limitKnob),
      outputAttachment (p.apvts, "output", outputKnob)
{
    setSize (320, 230);

    juce::Slider* knobs[] = { &inputKnob, &limitKnob, &outputKnob };
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

    auto setupLabel = [this] (juce::Label& label, const char* text)
    {
        label.setText (text, juce::dontSendNotification);
        label.setFont (juce::Font (juce::FontOptions().withHeight (12.0f).withStyle ("Bold")));
        label.setColour (juce::Label::textColourId, juce::Colour (0xff111111));
        label.setJustificationType (juce::Justification::centred);
        addAndMakeVisible (label);
    };

    setupLabel (inputLabel,  "Input");
    setupLabel (limitLabel,  "Limit");
    setupLabel (outputLabel, "Output");
}

LocBoxAudioProcessorEditor::~LocBoxAudioProcessorEditor()
{
    juce::Slider* knobs[] = { &inputKnob, &limitKnob, &outputKnob };
    for (auto* knob : knobs)
        knob->setLookAndFeel (nullptr);
}

void LocBoxAudioProcessorEditor::paint (juce::Graphics& g)
{
    corvid::paintPanelBackground (g, getWidth(), getHeight());

    // Plugin name
    g.setColour (juce::Colour (0xff111111));
    g.setFont (juce::Font (juce::FontOptions().withHeight (20.0f).withStyle ("Bold")));
    g.drawText ("Loc-Box", 12, 4, 150, 24, juce::Justification::bottomLeft);
}

void LocBoxAudioProcessorEditor::resized()
{
    constexpr int cx[3]  = { 53, 160, 267 };
    constexpr int r      = 48;  // 36 knob body + 12 tick margin
    constexpr int gap    = 4;
    constexpr int labelH = 15;
    constexpr int labelW = 100;

    const int groupH = r * 2 + gap + labelH;
    const int cy     = (getHeight() - groupH) / 2 + r;

    juce::Slider* knobs[]  = { &inputKnob,  &limitKnob,  &outputKnob  };
    juce::Label*  labels[] = { &inputLabel, &limitLabel, &outputLabel };

    for (int i = 0; i < 3; ++i)
    {
        knobs[i] ->setBounds (cx[i] - r,        cy - r,        r * 2,  r * 2);
        labels[i]->setBounds (cx[i] - labelW/2, cy + r + gap,  labelW, labelH);
    }
}
