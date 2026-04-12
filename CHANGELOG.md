# Corvid Audio Suite Changelog

## v1.3.1 -2026-04-11

**2-OP v1.3.0**
- Scale UI 1.2x: panel 545x455 → 654x546, labels 10pt → 12pt, knobs and sliders proportionally larger

---

## v1.3.0 -2026-04-09

**All plugins**
- Shift+drag fine-tune on all knobs and sliders (1/10 sensitivity); hold Shift while dragging for precise adjustments

---

## v1.2.1 -2026-04-08

**Broken v0.3.0**
- Add Mix knob (dry/wet blend, default 100% wet)
- Panel width increased to 430px to accommodate fourth knob

---

## v1.2.0 -2026-04-06

**Broken v0.2.0**
- Rename Bias knob to Starve; invert direction (0 = nominal, 100 = fully collapsed)
- Panel height increased to 230px to match suite standard
- Label font size increased to 12pt to match suite standard
- Knob tick marks added

**Dist308 v1.2.0**
- Add plugin name to UI
- Panel height increased to 230px to match suite standard
- Label font size increased to 12pt to match suite standard
- Knob tick marks added
- Mixed-case parameter labels (Distortion, Filter, Volume)

---

## v1.1.0 -2026-04-01

**Broken v0.1.0** (new)
- Circuit failure emulator: asymmetric dead-zone clipper with DC block HPF
- Drive, Starve, and Output controls

**Loc-Box v0.3.1**
- Fix knob tick marks at 3 and 9 o'clock not being straight

**Loc-Box v0.3.0**
- Add input and output transformer models
- Add sidechain HPF (2 uF AC-coupling cap)
- Increase compression ratio from 7:1 to 20:1
- Attack time now varies with limit setting
- Lower default limit from 50 to 25

**2-OP v1.1.0**
- Add VST3 format target

---

## v1.0.0 -2026-03-31

**Loc-Box v0.2.0**
- Fix input/output gain structure: unity gain at 12 o'clock
- Add plugin name label, tick marks, title-case knob labels

**Loc-Box v0.1.0** (new)
- Shure Level Loc brickwall limiter emulation
- JFET gain element, peak envelope detector, soft saturation
- Input, Limit, Output controls

**Life v0.5.0**
- Fix Iron pop on engagement

**Life v0.4.2**
- Fix Wide mode level imbalance and Iron knob snap-on

**Life v0.4.1**
- Use 15s mono noise sample

**Life v0.4.0**
- Merge IT/OT into single Iron knob (SSL-style transformer)
- Reorder signal chain: Noise, Harmonics, Tube, Iron
- Fix Wide mode clicks; all knob defaults set to zero

**Dist308 v1.1.0**
- Add VST3 build target
- Add package script for signing, notarizing, and DMG creation

---

## v0.2.0 -2026-03-14

**2-OP v1.0.0**
- Replace ADSR with Plaits LPG (Decay, Color, Output, Ping toggle)
- Gate and Ping modes
- Shift-key fine-tune on all FM sliders

**Headroom v0.2.0**
- Threshold range changed to -30–0 dBFS
- Faster LED fade in/out

**Life v0.3.0**
- Add README with screenshot

**Life v0.2.0**
- IT/OT changed to drive knobs
- Noise stage uses embedded console recording
- Wide mode pill toggle added

---

## v0.1.0 -2026-03-02

**2-OP v0.1.0** (new)
- 2-operator FM synthesizer; wraps Plaits FMEngine
- Ratio, Index, Feedback, Sub, ADSR controls

**Dist308 v1.0.0** (new)
- ProCo RAT-inspired distortion
- Full DSP overhaul: HPF, LM308 GBW model, tanh clipping, tone filter

**Headroom v0.1.0** (new)
- Hard clipper with threshold knob and clip LED

**Life v0.1.0** (new)
- Analog character plugin: Noise, Harmonics, Tube, Iron, Wide
