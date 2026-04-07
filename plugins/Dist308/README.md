<img src="../../docs/corvid.png" width="64" alt="Corvid Audio"/>

# Dist308

![Dist308](docs/Dist308.png)

A ProCo RAT-inspired distortion AU/VST3 plugin, built with [JUCE](https://juce.com).

The signal chain models the key stages of the classic RAT circuit: a high-pass filter removes bass bloom before clipping, a pre-clip low-pass filter models the LM308 op-amp's gain-bandwidth product, `tanh` saturation emulates the anti-parallel 1N914 diode clipping, and a post-clip one-pole low-pass filter provides the tone control.

## Parameters

| Parameter | Range | Description |
|-----------|-------|-------------|
| Distortion | 0–100 | Drive amount: exponential gain curve (47×–1047×); pre-clip LPF tracks gain to model LM308 GBW rolloff |
| Filter | 0–100 | Tone control: CCW = dark (475 Hz), CW = bright (22 kHz) |
| Volume | 0–100 | Output level: quadratic taper |

Hold Shift while dragging any knob for fine-tune resolution.

## Building

### Requirements

- macOS 11.0+, arm64 or x86_64
- Xcode Command Line Tools (`xcode-select --install`)
- CMake >= 3.22 and Ninja (`brew install cmake ninja`)
- [JUCE](https://github.com/juce-framework/JUCE) source

### Setup

Clone this repo and JUCE as siblings:

```
parent/
  JUCE/
  Dist308/
```

Or pass a custom path with `-DJUCE_DIR=...`.

### Build

```bash
cmake -B build -G Ninja \
    -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0 \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_C_COMPILER=$(xcrun -f clang) \
    -DCMAKE_CXX_COMPILER=$(xcrun -f clang++)
cmake --build build --config Release
```

### Install

```bash
# AU
cp -R build/Dist308_artefacts/Release/AU/Dist308.component \
      ~/Library/Audio/Plug-Ins/Components/
codesign --force --sign - ~/Library/Audio/Plug-Ins/Components/Dist308.component
auval -v aufx D308 CVDA

# VST3
cp -R build/Dist308_artefacts/Release/VST3/Dist308.vst3 \
      ~/Library/Audio/Plug-Ins/VST3/
codesign --force --sign - ~/Library/Audio/Plug-Ins/VST3/Dist308.vst3
```

### Package (sign, notarize, DMG)

The `scripts/package.sh` script builds, code-signs, creates a DMG, and notarizes it in one step. It requires an Apple Developer ID certificate and a notarytool keychain profile (see the script header for setup).

```bash
./scripts/package.sh
```

## Formats

| Format | AU Type | Manufacturer | Plugin Code | Bundle ID |
|--------|---------|--------------|-------------|-----------|
| AU Effect | `aufx` | `CVDA` | `D308` | `com.CorvidAudio.Dist308` |
| VST3 | - | - | - | `com.CorvidAudio.Dist308` |

## License

Copyright 2026 Corvid Audio

This project is licensed under the **GNU General Public License v3.0**, see [LICENSE](LICENSE) for the full text.

JUCE is used under its [GPL-3.0 open-source licence](https://github.com/juce-framework/JUCE/blob/master/LICENSE.md).
