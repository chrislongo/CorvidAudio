#include "PluginEditor.h"

//==============================================================================
// Dist308AudioProcessorEditor
//==============================================================================

Dist308AudioProcessorEditor::Dist308AudioProcessorEditor (Dist308AudioProcessor& p)
    : AudioProcessorEditor (&p),
      distAttachment   (p.apvts, "distortion", distKnob),
      filterAttachment (p.apvts, "filter",     filterKnob),
      volumeAttachment (p.apvts, "volume",     volumeKnob)
{
    setSize (320, 230);

    juce::Slider* knobs[] = { &distKnob, &filterKnob, &volumeKnob };
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

    setupLabel (distLabel,   "Distortion");
    setupLabel (filterLabel, "Filter");
    setupLabel (volumeLabel, "Volume");
}

Dist308AudioProcessorEditor::~Dist308AudioProcessorEditor()
{
    juce::Slider* knobs[] = { &distKnob, &filterKnob, &volumeKnob };
    for (auto* knob : knobs)
        knob->setLookAndFeel (nullptr);
}

void Dist308AudioProcessorEditor::paint (juce::Graphics& g)
{
    corvid::paintPanelBackground (g, getWidth(), getHeight());

    g.setColour (juce::Colour (0xff111111));
    g.setFont (juce::Font (juce::FontOptions().withHeight (20.0f).withStyle ("Bold")));
    g.drawText ("Dist308", 12, 4, 150, 24, juce::Justification::bottomLeft);
}

void Dist308AudioProcessorEditor::resized()
{
    constexpr int cx[3]  = { 53, 160, 267 };
    constexpr int r      = 48;  // 36 knob body + 12 tick margin each side
    constexpr int gap    = 4;
    constexpr int labelH = 15;
    constexpr int labelW = 100;

    const int groupH = r * 2 + gap + labelH;
    const int cy     = (getHeight() - groupH) / 2 + r;

    juce::Slider* knobs[]  = { &distKnob,  &filterKnob,  &volumeKnob  };
    juce::Label*  labels[] = { &distLabel, &filterLabel, &volumeLabel };

    for (int i = 0; i < 3; ++i)
    {
        knobs[i] ->setBounds (cx[i] - r,        cy - r,        r * 2,  r * 2);
        labels[i]->setBounds (cx[i] - labelW/2, cy + r + gap,  labelW, labelH);
    }
}
