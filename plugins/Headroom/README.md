<img src="../../docs/corvid.png" width="64" alt="Corvid Audio"/>

# Headroom

![Headroom](docs/Headroom.png)

A **hard clipper** with a single threshold control. Start at 100% for transparent passthrough, then pull back until the clip LED lights — the signal is cut at exactly that ceiling with no added colour.

## Controls

| Control | Range | Description |
|---------|-------|-------------|
| **Threshold** | 0 – 100% | Clip ceiling. 100% = full scale (no clipping). Pull back to reduce headroom and introduce hard clipping. |
| **LED** | — | Lights red when the signal exceeds the threshold. Stays lit for ~500 ms after the last clipped sample. |

## Signal Chain

```
Input → Hard Clip (±threshold) → Output
```

## Building

Requires CMake 3.22+, Ninja, and JUCE (expected at `~/src/github/JUCE`).

```bash
# Configure (from repo root)
cmake -B build -G Ninja \
    -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0 \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_C_COMPILER=$(xcrun -f clang) \
    -DCMAKE_CXX_COMPILER=$(xcrun -f clang++)

# Build
cmake --build build --config Release --target Headroom
```

Artifacts land in `build/plugins/Headroom/Headroom_artefacts/Release/`:
- `AU/Headroom.component`
- `VST3/Headroom.vst3`
- `Standalone/Headroom.app`

## Install

```bash
# AU
cp -R build/plugins/Headroom/Headroom_artefacts/Release/AU/Headroom.component \
      ~/Library/Audio/Plug-Ins/Components/
codesign --force --sign "-" ~/Library/Audio/Plug-Ins/Components/Headroom.component

# VST3
cp -R build/plugins/Headroom/Headroom_artefacts/Release/VST3/Headroom.vst3 \
      ~/Library/Audio/Plug-Ins/VST3/
codesign --force --sign "-" ~/Library/Audio/Plug-Ins/VST3/Headroom.vst3
```

Validate the AU with:

```bash
auval -v aufx HDRM CVDA
```

## License

GPL-3.0 — see [LICENSE](LICENSE).
