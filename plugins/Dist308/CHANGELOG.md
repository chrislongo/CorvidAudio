# Dist308 Changelog

### v1.2.0 -2026-04-06
- Add plugin name to UI
- Panel height increased to 230px to match suite standard
- Label font size increased to 12pt to match suite standard

### v1.1.0 -2026-03-30
- Add VST3 build target
- Use JUCE_DIR cache variable instead of hardcoded path (overridable via `-DJUCE_DIR=...`)
- Add package script for signing, notarizing, and DMG creation

### v1.0.0 -2026-03-07
- Compute pre-clip LPF coefficient per-sample for accurate gain tracking at all distortion levels
- Fix stereo channel index bug
- Switch to exponential gain curve (47×–1047×); clean output at zero distortion
- Invert Filter knob direction: CCW = darkest (475 Hz), CW = brightest (22 kHz); default 50%
- Overhaul DSP signal chain for accurate RAT circuit emulation
  - HPF at 180 Hz removes bass bloom before clipping
  - Pre-clip LPF models LM308 GBW (effective 5 MHz, calibrated against real RAT recording)
  - tanh saturation models anti-parallel 1N914 diodes in op-amp feedback
  - Post-clip LPF controlled by Filter knob
- Initial release: ProCo Rat-inspired distortion AU effect
- Parameters: Distortion, Filter, Volume
- Manufacturer code CVDA, plugin code D308, bundle `com.CorvidAudio.Dist308`
