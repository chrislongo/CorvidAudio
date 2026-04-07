# Broken Changelog

### v0.2.0 -2026-04-06
- Rename Bias knob to Starve; invert direction (0 = nominal, 100 = fully collapsed)
- Panel height increased to 230px to match suite standard
- Label font size increased to 12pt to match suite standard
- Knob tick marks added

### v0.1.0 -2026-04-01
- Initial release: circuit failure emulator
- Drive knob (input gain, audio taper 1× to 10×)
- Starve knob (supply voltage, skewed taper; circuit collapses around 9 o'clock)
- Output trim (-20 to +6 dB)
- Asymmetric dead-zone clipper: hard snap on positive side, softer on negative
- DC block HPF (~30 Hz) removes offset introduced by asymmetric clipping
- Manufacturer code CVDA, plugin code BRKN, bundle `com.CorvidAudio.Broken`
