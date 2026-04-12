#include "PluginEditor.h"


//==============================================================================
// Geometry
//==============================================================================
namespace {
    // ── FM slider columns: Ratio, Index, Feedback, Sub ─────────────────────
    // Evenly distributed; Sub aligns with Output knob (col 5).
    constexpr int kFMColX[]   = { 100, 251, 403, 556 };

    // ── Knob columns: Attack, Decay, Color, Output ─────────────────────────
    constexpr int kKnobColX[] = { 100, 252, 404, 556 };

    // ── FM slider geometry ─────────────────────────────────────────────────
    constexpr int kTrackTop    = 114;
    constexpr int kTrackBot    = 306;
    constexpr int kTrackHeight = kTrackBot - kTrackTop;   // 192
    constexpr int kTrackWidth  = 6;

    constexpr int kThumbW      = 50;
    constexpr int kThumbH      = 16;
    constexpr int kTickCount   = 10;

    constexpr int kSliderW     = 53;
    constexpr int kSliderTop   = kTrackTop - kThumbH / 2;            // 106
    constexpr int kSliderH     = kTrackBot - kSliderTop + kThumbH / 2; // 208

    // FM label
    constexpr int kFMLabelY    = 325;
    constexpr int kFMLabelH    = 17;
    constexpr int kFMLabelW    = 96;

    // ── Knob geometry ──────────────────────────────────────────────────────
    constexpr int   kKnobR     = 31;
    constexpr int   kKnobCY    = 446;
    constexpr int   kKnobW     = kKnobR * 2 + 24;  // 86 — extra margin for tick marks

    // Rotary sweep: −135° to +135° from 12 o'clock (270° total).
    constexpr float kRotaryStart = juce::MathConstants<float>::pi * 1.25f;
    constexpr float kRotaryEnd   = juce::MathConstants<float>::pi * 2.75f;

    // Knob label
    constexpr int kKnobLabelY  = 494;
    constexpr int kKnobLabelH  = 17;
    constexpr int kKnobLabelW  = 96;

    // ── LPG section header ─────────────────────────────────────────────────
    constexpr int kLPGHeaderY  = 365;
    constexpr int kLPGHeaderH  = 17;

    // ── Ping pill ──────────────────────────────────────────────────────────
    constexpr int kPillW       = 46;
    constexpr int kPillH       = 13;
    // bottomLeft label text visual center sits ~11px into the 17px header rect;
    // offset pill so its centre matches.
    constexpr int kPillYOffset = 11 - kPillH / 2;  // = 5
}

//==============================================================================
// FMSliderLookAndFeel
//==============================================================================

FMSliderLookAndFeel::FMSliderLookAndFeel()
{
    setColour (juce::Slider::textBoxTextColourId,       juce::Colour (0xff333333));
    setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
    setColour (juce::Slider::textBoxOutlineColourId,    juce::Colours::transparentBlack);
}

void FMSliderLookAndFeel::drawLinearSlider (juce::Graphics& g,
                                             int /*x*/, int /*y*/, int /*width*/, int /*height*/,
                                             float sliderPos,
                                             float /*minSliderPos*/,
                                             float /*maxSliderPos*/,
                                             juce::Slider::SliderStyle,
                                             juce::Slider& slider)
{
    const float localTrackTop = (float) (kTrackTop - kSliderTop);
    const float localTrackBot = (float) (kTrackBot - kSliderTop);
    const float trackH        = localTrackBot - localTrackTop;
    const float cx            = slider.getWidth() * 0.5f;

    // ── Tick marks (skip top and bottom endpoints) ───────────────────────────
    g.setColour (juce::Colour (0xff888888));
    for (int i = 1; i < kTickCount - 1; ++i)
    {
        const float ty = localTrackTop + (float) i * (trackH / (float) (kTickCount - 1));
        g.drawLine (cx - 17.0f, ty, cx - 5.5f, ty, 1.0f);
        g.drawLine (cx + 5.5f, ty, cx + 17.0f, ty, 1.0f);
    }

    // ── Track ────────────────────────────────────────────────────────────────
    g.setColour (juce::Colour (0xff2a2a2a));
    g.fillRect (cx - kTrackWidth * 0.5f, localTrackTop, (float) kTrackWidth, trackH);

    // ── Thumb ────────────────────────────────────────────────────────────────
    const float thumbCy = sliderPos;
    const float thumbX  = cx - kThumbW * 0.5f;
    const float thumbY  = thumbCy - kThumbH * 0.5f;

    g.setColour (juce::Colour (0xff111111));
    g.fillRect (thumbX, thumbY, (float) kThumbW, (float) kThumbH);

    g.setColour (juce::Colours::white);
    g.drawLine (thumbX, thumbCy, thumbX + kThumbW, thumbCy, 1.0f);
}

//==============================================================================
// ADSRKnobLookAndFeel
//==============================================================================

void ADSRKnobLookAndFeel::drawRotarySlider (juce::Graphics& g,
                                             int x, int y, int width, int height,
                                             float sliderPos,
                                             float rotaryStartAngle,
                                             float rotaryEndAngle,
                                             juce::Slider&)
{
    const float cx = x + width  * 0.5f;
    const float cy = y + height * 0.5f;
    const float r  = (float) kKnobR;

    // ── Tick marks ───────────────────────────────────────────────────────────
    constexpr int kNumTicks = 7;
    g.setColour (juce::Colour (0xff888888));
    for (int i = 0; i < kNumTicks; ++i)
    {
        const float t     = (float) i / (float) (kNumTicks - 1);
        const float angle = rotaryStartAngle + t * (rotaryEndAngle - rotaryStartAngle);
        const float sinA  = std::sin (angle);
        const float cosA  = std::cos (angle);
        g.drawLine (cx + sinA * (r + 5.0f),  cy - cosA * (r + 5.0f),
                    cx + sinA * (r + 12.0f), cy - cosA * (r + 12.0f),
                    1.0f);
    }

    // ── Body ─────────────────────────────────────────────────────────────────
    g.setColour (juce::Colour (0xff111111));
    g.fillEllipse (cx - r, cy - r, r * 2.0f, r * 2.0f);

    // ── Indicator line ───────────────────────────────────────────────────────
    const float angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    const float sinA  = std::sin (angle);
    const float cosA  = std::cos (angle);

    juce::Path indicator;
    indicator.startNewSubPath (cx + sinA * 11.6f,  cy - cosA * 11.6f);
    indicator.lineTo           (cx + sinA * 27.9f,  cy - cosA * 27.9f);

    g.setColour (juce::Colours::white);
    g.strokePath (indicator,
                  juce::PathStrokeType (4.0f,
                                        juce::PathStrokeType::curved,
                                        juce::PathStrokeType::rounded));
}

//==============================================================================
// PillLookAndFeel
//==============================================================================

void PillLookAndFeel::drawToggleButton (juce::Graphics& g, juce::ToggleButton& b,
                                        bool, bool)
{
    const float w      = (float) b.getWidth();
    const float h      = (float) b.getHeight();
    const float corner = h * 0.42f;

    if (b.getToggleState())
    {
        // PING: filled matte-black pill
        g.setColour (juce::Colour (0xff111111));
        g.fillRoundedRectangle (0.0f, 0.0f, w, h, corner);
        g.setColour (juce::Colours::white);
    }
    else
    {
        // GATE: outlined pill on panel background
        g.setColour (juce::Colour (0xff888888));
        g.drawRoundedRectangle (0.5f, 0.5f, w - 1.0f, h - 1.0f, corner, 1.0f);
        g.setColour (juce::Colour (0xff444444));
    }

    g.setFont (juce::Font (juce::FontOptions().withHeight (9.0f).withStyle ("Bold")));
    g.drawText ("PING",
                0, 0, (int) w, (int) h, juce::Justification::centred);
}

//==============================================================================
// TwoOpFMAudioProcessorEditor
//==============================================================================

TwoOpFMAudioProcessorEditor::TwoOpFMAudioProcessorEditor (TwoOpFMAudioProcessor& p)
    : AudioProcessorEditor (&p),
      ratioAttach_   (p.apvts, "ratio",    ratioSlider_),
      indexAttach_   (p.apvts, "index",    indexSlider_),
      feedbackAttach_(p.apvts, "feedback", feedbackSlider_),
      subAttach_     (p.apvts, "sub",      subSlider_),
      attackAttach_  (p.apvts, "attack",   attackSlider_),
      decayAttach_   (p.apvts, "decay",    decaySlider_),
      colorAttach_   (p.apvts, "color",    colorSlider_),
      outputAttach_  (p.apvts, "output",   outputSlider_),
      pingAttach_    (p.apvts, "ping",     pingButton_)
{
    setSize (654, 546);

    setupSlider (ratioSlider_,    ratioLabel_,    "Ratio");
    setupSlider (indexSlider_,    indexLabel_,    "Index");
    setupSlider (feedbackSlider_, feedbackLabel_, "Feedback");
    setupSlider (subSlider_,      subLabel_,      "Sub");

    setupKnob (attackSlider_, attackLabel_, "Attack");
    setupKnob (decaySlider_,  decayLabel_,  "Decay");
    setupKnob (colorSlider_, colorLabel_, "Color");
    setupKnob (outputSlider_, outputLabel_, "Output");

    pingButton_.setClickingTogglesState (true);
    pingButton_.setLookAndFeel (&pillLAF_);
    addAndMakeVisible (pingButton_);
}

TwoOpFMAudioProcessorEditor::~TwoOpFMAudioProcessorEditor()
{
    juce::Slider* all[] = { &ratioSlider_, &indexSlider_, &feedbackSlider_, &subSlider_,
                             &attackSlider_, &decaySlider_, &colorSlider_, &outputSlider_ };
    for (auto* s : all)
        s->setLookAndFeel (nullptr);
    pingButton_.setLookAndFeel (nullptr);
}

void TwoOpFMAudioProcessorEditor::setupSlider (juce::Slider& s, juce::Label& nameLabel,
                                                const juce::String& name)
{
    s.setSliderStyle (juce::Slider::LinearVertical);
    s.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    s.setLookAndFeel (&sliderLAF_);
    addAndMakeVisible (s);

    nameLabel.setText (name, juce::dontSendNotification);
    nameLabel.setFont (juce::Font (juce::FontOptions().withHeight (12.0f).withStyle ("Bold")));
    nameLabel.setColour (juce::Label::textColourId, juce::Colour (0xff444444));
    nameLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (nameLabel);
}

void TwoOpFMAudioProcessorEditor::setupKnob (juce::Slider& s, juce::Label& nameLabel,
                                              const juce::String& name)
{
    s.setSliderStyle (juce::Slider::RotaryVerticalDrag);
    s.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    s.setRotaryParameters (kRotaryStart, kRotaryEnd, true);
    s.setLookAndFeel (&knobLAF_);
    addAndMakeVisible (s);

    nameLabel.setText (name, juce::dontSendNotification);
    nameLabel.setFont (juce::Font (juce::FontOptions().withHeight (12.0f).withStyle ("Bold")));
    nameLabel.setColour (juce::Label::textColourId, juce::Colour (0xff444444));
    nameLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (nameLabel);
}

void TwoOpFMAudioProcessorEditor::paint (juce::Graphics& g)
{
    // ── Panel background ─────────────────────────────────────────────────────
    g.setColour (juce::Colour (0xffd8d8d8));
    g.fillAll();

    juce::ColourGradient overlay (juce::Colour (0x18ffffff), 0.0f, 0.0f,
                                  juce::Colour (0x18000000), 0.0f, (float) getHeight(), false);
    g.setGradientFill (overlay);
    g.fillAll();

    // ── Title strip ──────────────────────────────────────────────────────────
    g.setColour (juce::Colour (0xff111111));
    g.setFont (juce::Font (juce::FontOptions().withHeight (36.0f).withStyle ("Bold")));
    g.drawText ("2-OP", 26, 11, 180, 44, juce::Justification::bottomLeft);

    g.setColour (juce::Colour (0xff666666));
    g.setFont (juce::Font (juce::FontOptions().withHeight (9.0f).withStyle ("Bold")));
    g.drawText ("FM SYNTHESIZER", 0, 17, getWidth() - 28, 17, juce::Justification::bottomRight);

    g.setColour (juce::Colour (0xff777777));
    g.setFont (juce::Font (juce::FontOptions().withHeight (9.0f)));
    g.drawText ("OPERATOR MODEL: 2-OP / FEEDBACK", 0, 34, getWidth() - 28, 17, juce::Justification::bottomRight);

    // ── FM OPERATORS section box ─────────────────────────────────────────────
    const juce::Rectangle<float> fmBox (17.0f, 72.0f, 620.0f, 274.0f);
    g.setColour (juce::Colour (0xffd0d0d0));
    g.fillRoundedRectangle (fmBox, 10.0f);
    g.setColour (juce::Colour (0xff888888));
    g.drawRoundedRectangle (fmBox, 10.0f, 1.5f);

    g.setColour (juce::Colour (0xff555555));
    g.setFont (juce::Font (juce::FontOptions().withHeight (10.0f).withStyle ("Bold")));
    g.drawText ("FM CONTROLS", 34, 82, 240, 17, juce::Justification::bottomLeft);

    // ── ENVELOPE + OUTPUT section box ────────────────────────────────────────
    const juce::Rectangle<float> envBox (17.0f, 355.0f, 620.0f, 179.0f);
    g.setColour (juce::Colour (0xffd0d0d0));
    g.fillRoundedRectangle (envBox, 10.0f);
    g.setColour (juce::Colour (0xff888888));
    g.drawRoundedRectangle (envBox, 10.0f, 1.5f);

    g.setColour (juce::Colour (0xff555555));
    g.setFont (juce::Font (juce::FontOptions().withHeight (10.0f).withStyle ("Bold")));
    g.drawText ("LPG + OUTPUT", 34, kLPGHeaderY, 300, kLPGHeaderH, juce::Justification::bottomLeft);

}

void TwoOpFMAudioProcessorEditor::resized()
{
    static_assert (std::size (kFMColX)   == 4, "");
    static_assert (std::size (kKnobColX) == 4, "");

    juce::Slider* fmSliders[] = { &ratioSlider_, &indexSlider_, &feedbackSlider_, &subSlider_ };
    juce::Label*  fmNames[]   = { &ratioLabel_,  &indexLabel_,  &feedbackLabel_,  &subLabel_ };

    for (int i = 0; i < 4; ++i)
    {
        const int cx = kFMColX[i];
        fmSliders[i]->setBounds (cx - kSliderW / 2, kSliderTop, kSliderW, kSliderH);
        fmNames  [i]->setBounds (cx - kFMLabelW / 2, kFMLabelY, kFMLabelW, kFMLabelH);
    }

    juce::Slider* knobs[]     = { &attackSlider_, &decaySlider_, &colorSlider_, &outputSlider_ };
    juce::Label*  knobNames[] = { &attackLabel_,  &decayLabel_,  &colorLabel_,  &outputLabel_ };

    for (int i = 0; i < 4; ++i)
    {
        const int cx = kKnobColX[i];
        knobs    [i]->setBounds (cx - kKnobW / 2, kKnobCY - kKnobW / 2, kKnobW, kKnobW);
        knobNames[i]->setBounds (cx - kKnobLabelW / 2, kKnobLabelY, kKnobLabelW, kKnobLabelH);
    }

    // Pill toggle: centered on Decay knob column, vertically centered in label row.
    pingButton_.setBounds (kKnobColX[1] - kPillW / 2, kLPGHeaderY + kPillYOffset, kPillW, kPillH);
}
