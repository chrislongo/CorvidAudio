#include "PluginEditor.h"

//==============================================================================
// BlackKnobLookAndFeel
//==============================================================================

BlackKnobLookAndFeel::BlackKnobLookAndFeel() {}

void BlackKnobLookAndFeel::drawRotarySlider (juce::Graphics& g,
                                              int x, int y, int width, int height,
                                              float sliderPosProportional,
                                              float rotaryStartAngle,
                                              float rotaryEndAngle,
                                              juce::Slider&)
{
    const float cx = x + width  * 0.5f;
    const float cy = y + height * 0.5f;
    const float r  = juce::jmin (width, height) * 0.5f - 2.0f;

    // Drop shadow
    {
        juce::ColourGradient shadow (juce::Colour (0x50000000), cx, cy + r * 0.1f,
                                     juce::Colours::transparentBlack, cx, cy + r * 1.3f, true);
        g.setGradientFill (shadow);
        g.fillEllipse (cx - r - 3.0f, cy - r + 4.0f, (r + 3.0f) * 2.0f, (r + 3.0f) * 2.0f);
    }

    // Black body
    g.setColour (juce::Colour (0xff111111));
    g.fillEllipse (cx - r, cy - r, r * 2.0f, r * 2.0f);

    // Subtle top-edge highlight
    {
        juce::ColourGradient highlight (juce::Colour (0x25ffffff), cx, cy - r,
                                        juce::Colours::transparentBlack, cx, cy, false);
        g.setGradientFill (highlight);
        g.fillEllipse (cx - r, cy - r, r * 2.0f, r * 2.0f);
    }

    // White indicator line
    {
        const float angle = rotaryStartAngle
                          + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);
        const float sinA  = std::sin (angle);
        const float cosA  = -std::cos (angle);

        const float innerR = r * 0.30f;
        const float outerR = r * 0.78f;

        juce::Path line;
        line.startNewSubPath (cx + sinA * innerR, cy + cosA * innerR);
        line.lineTo          (cx + sinA * outerR, cy + cosA * outerR);

        g.setColour (juce::Colours::white);
        g.strokePath (line, juce::PathStrokeType (2.2f));
    }
}

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

    setupLabel (distLabel,   "DISTORTION");
    setupLabel (filterLabel, "FILTER");
    setupLabel (volumeLabel, "VOLUME");
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
    constexpr int r      = 36;
    constexpr int gap    = 4;
    constexpr int labelH = 15;
    constexpr int labelW = 100;

    // Centre the knob+gap+label group vertically in the panel
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
