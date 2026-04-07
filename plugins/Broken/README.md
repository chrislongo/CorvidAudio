<img src="../../docs/corvid.png" width="64" alt="Corvid Audio"/>

# Broken

![Broken](docs/Broken.png)

A circuit failure emulator AU/VST3 plugin, built with [JUCE](https://juce.com).

Models the behavior of a transistor-based circuit operating outside its design envelope: as supply voltage drops, the transistor loses its stable operating point and begins oscillating between cutoff and saturation on every audio cycle. The result ranges from asymmetric clipping at full voltage down to sputtering, gated disintegration as the circuit collapses.

## Parameters

| Parameter | Range | Description |
|-----------|-------|-------------|
| Drive | 0–100 | Input gain into the circuit: audio taper, 1× to 10× |
| Starve | 0–100 | Supply voltage. 0 = nominal operation; as it rises the circuit loses its bias point and breaks apart |
| Output | -20–+6 dB | Output level trim |

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
  CorvidAudio/
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
cmake --build build --config Release --target Broken
```

### Install

```bash
# AU
cp -R build/plugins/Broken/Broken_artefacts/Release/AU/Broken.component \
      ~/Library/Audio/Plug-Ins/Components/
codesign --force --sign - ~/Library/Audio/Plug-Ins/Components/Broken.component
auval -v aufx BRKN CVDA

# VST3
cp -R build/plugins/Broken/Broken_artefacts/Release/VST3/Broken.vst3 \
      ~/Library/Audio/Plug-Ins/VST3/
codesign --force --sign - ~/Library/Audio/Plug-Ins/VST3/Broken.vst3
```

## Formats

| Format | AU Type | Manufacturer | Plugin Code | Bundle ID |
|--------|---------|--------------|-------------|-----------|
| AU Effect | `aufx` | `CVDA` | `BRKN` | `com.CorvidAudio.Broken` |
| VST3 | - | - | - | `com.CorvidAudio.Broken` |

## License

Copyright 2026 Corvid Audio

This project is licensed under the **GNU General Public License v3.0**, see [LICENSE](LICENSE) for the full text.

JUCE is used under its [GPL-3.0 open-source licence](https://github.com/juce-framework/JUCE/blob/master/LICENSE.md).
