# Changelog

## v0.5.0

- Fix Iron pop on engagement: transformer chain always runs with dry/wet blend (drive=0 is 100% dry), eliminating the DC blocker step-discontinuity that caused the click

## v0.4.2

- Fix Wide mode level imbalance: saturation offset no longer engages at zero
- Fix Iron knob snap-on: normalize transformer output by drive gain (not tanh(g)) for smooth engagement from zero

## v0.4.1

- Use 15s mono noise sample. 

## v0.4.0

- Merged IT/OT into single "Iron" transformer knob (SSL-style console transformer with asymmetric saturation, DC blocking, and drive-proportional HF rolloff)
- Reordered signal chain: Noise → Harmonics → Tube → Transformer
- Fixed Wide mode clicks/pops: replaced periodic step-changes with smooth mean-reverting random walk
- All knob defaults set to zero (clean passthrough)
- UI: 4 knobs (Noise, Harmonics, Tube, Iron) + Wide pill

## v0.3.0

- Add README with screenshot and Corvid Audio logo

## v0.2.0

- IT/OT changed from on/off toggles to drive knobs (0-100) with drive-scaled saturation and always-warm HPF state
- Noise stage now uses embedded console recording (resource/Noise.aif) instead of algorithmic pink noise
- Wide mode: pill toggle in top right, default on, slow-updating random variations to prevent crackle
- THD max bumped to 50%, renamed label to THD
- Tube saturation label
- Rotary sweep set to exact 270 degrees for horizontal 3/9 o'clock ticks

## v0.1.0

- Initial release: Life AU/VST3 plugin
