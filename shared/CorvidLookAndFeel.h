#pragma once

#include <JuceHeader.h>

namespace corvid
{

//==============================================================================
// Shared panel background: silver-grey #d8d8d8 with subtle gradient overlay.
inline void paintPanelBackground (juce::Graphics& g, int width, int height)
{
    g.setColour (juce::Colour (0xffd8d8d8));
    g.fillAll();

    juce::ColourGradient overlay (juce::Colour (0x18ffffff), 0.0f, 0.0f,
                                   juce::Colour (0x18000000), 0.0f, static_cast<float> (height), false);
    g.setGradientFill (overlay);
    g.fillAll();
}

//==============================================================================
// Matte-black rotary knob with 7 tick marks and white indicator line.
// Used by Life and Loc-Box.
class BlackKnobLookAndFeel : public juce::LookAndFeel_V4
{
public:
    BlackKnobLookAndFeel() {}

    void drawRotarySlider (juce::Graphics& g,
                           int x, int y, int width, int height,
                           float sliderPosProportional,
                           float rotaryStartAngle,
                           float rotaryEndAngle,
                           juce::Slider&) override
    {
        const float cx = x + width  * 0.5f;
        const float cy = y + height * 0.5f;
        const float r  = juce::jmin (width, height) * 0.5f - 12.0f;

        // Tick marks
        constexpr int kNumTicks = 7;
        g.setColour (juce::Colour (0xff888888));
        for (int i = 0; i < kNumTicks; ++i)
        {
            const float t     = (float) i / (float) (kNumTicks - 1);
            const float angle = rotaryStartAngle + t * (rotaryEndAngle - rotaryStartAngle);
            const float sinA  = std::sin (angle);
            const float cosA  = std::cos (angle);
            g.drawLine (cx + sinA * (r + 4.0f),  cy - cosA * (r + 4.0f),
                        cx + sinA * (r + 10.0f), cy - cosA * (r + 10.0f),
                        1.0f);
        }

        // Black body
        g.setColour (juce::Colour (0xff111111));
        g.fillEllipse (cx - r, cy - r, r * 2.0f, r * 2.0f);

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
};

//==============================================================================
// Pill-shaped toggle button bound to an APVTS bool parameter.
// Used by Life (Wide toggle). Reusable for any on/off pill.
class PillToggle : public juce::Component
{
public:
    PillToggle (juce::AudioProcessorValueTreeState& apvts,
                const juce::String& paramID,
                const juce::String& text)
        : displayText (text)
    {
        addChildComponent (hiddenButton);
        attachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
            apvts, paramID, hiddenButton);
    }

    ~PillToggle() override {}

    void paint (juce::Graphics& g) override
    {
        const auto bounds = getLocalBounds().toFloat().reduced (1.0f);
        const float cornerSize = bounds.getHeight() * 0.5f;
        const bool engaged = hiddenButton.getToggleState();

        if (engaged)
        {
            g.setColour (juce::Colour (0xff111111));
            g.fillRoundedRectangle (bounds, cornerSize);
            g.setColour (juce::Colour (0xffd8d8d8));
        }
        else
        {
            g.setColour (juce::Colour (0xff111111));
            g.drawRoundedRectangle (bounds, cornerSize, 1.5f);
            g.setColour (juce::Colour (0xff111111));
        }

        g.setFont (juce::Font (juce::FontOptions().withHeight (11.0f).withStyle ("Bold")));
        g.drawText (displayText, getLocalBounds(), juce::Justification::centred);
    }

    void mouseUp (const juce::MouseEvent& event) override
    {
        if (event.mouseWasClicked())
        {
            hiddenButton.setToggleState (!hiddenButton.getToggleState(),
                                         juce::sendNotificationSync);
            repaint();
        }
    }

private:
    juce::String displayText;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> attachment;
    juce::ToggleButton hiddenButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PillToggle)
};

} // namespace corvid
