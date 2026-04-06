<img src="../../docs/corvid.png" width="64" alt="Corvid Audio"/>

# Life

![Life](docs/Life.png)

An analog character plugin that adds **life and warmth** to digital tracks. Built as an AU/VST3 audio plugin with [JUCE](https://juce.com/).

Life models the subtle imperfections of analog hardware: transformer coloration, console noise floor, harmonic distortion, and tube saturation, giving sterile digital mixes the depth and movement of real gear.

## Signal Chain

```
Input → Noise → Harmonics → Tube → Iron → Output
```

## Controls

| Knob | Range | Description |
|------|-------|-------------|
| **Noise** | 0 -- 100% | Console noise floor. A real noise sample captured from an analog mixing console, looped and mixed into the signal. |
| **Harmonics** | 0 -- 100% | Harmonic distortion injection. 70% 2nd harmonic (warm, even) and 30% 3rd harmonic (edge, odd). |
| **Tube** | 0 -- 100% | Tube-style tanh saturation with subtle asymmetry. Drive ranges from unity to 4x. |
| **Iron** | 0 -- 100% | SSL-style console transformer. Gain staging with asymmetric saturation for even harmonics, DC blocking at ~30 Hz, and drive-proportional HF rolloff at ~12 kHz. |

| Toggle | Default | Description |
|--------|---------|-------------|
| **Wide** | On | L/R decorrelation. Applies subtle per-channel random variation to noise level, THD amount, saturation drive, and transformer drive for a wider stereo image. |

## Building

Requires CMake 3.22+, Ninja, and JUCE (expected at `/Users/chris/src/github/JUCE`).

```bash
# Configure
cmake -B build -G Ninja \
    -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0 \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_C_COMPILER=$(xcrun -f clang) \
    -DCMAKE_CXX_COMPILER=$(xcrun -f clang++)

# Build
cmake --build build --config Release
```

Artifacts land in `build/Life_artefacts/Release/`:
- `AU/Life.component`
- `VST3/Life.vst3`
- `Standalone/Life.app`

## Install

```bash
# AU
cp -R build/Life_artefacts/Release/AU/Life.component \
      ~/Library/Audio/Plug-Ins/Components/
codesign --force --sign "-" ~/Library/Audio/Plug-Ins/Components/Life.component

# VST3
cp -R build/Life_artefacts/Release/VST3/Life.vst3 \
      ~/Library/Audio/Plug-Ins/VST3/
codesign --force --sign "-" ~/Library/Audio/Plug-Ins/VST3/Life.vst3
```

Validate the AU with:

```bash
auval -v aufx LIFE CVDA
```

## License

GPL-3.0 -- see [LICENSE](LICENSE).
