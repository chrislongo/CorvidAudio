<img src="../../docs/corvid.png" width="64" alt="Corvid Audio"/>

# Broken

A **dying battery fuzz** inspired by the fuzz face bias pot. Turn Bias down past 9 o'clock and the circuit starves: the transistor oscillates between cutoff and saturation on every audio cycle, producing the sputtering velcro texture of a pedal running on a nearly-dead battery.

## Controls

| Control | Range | Description |
|---------|-------|-------------|
| **Drive** | 0 – 100% | Input gain. Higher drive pushes more signal into the clipping/velcro zone. |
| **Bias** | 0 – 100% | Transistor operating point. 100% = properly biased (asymmetric fuzz). Below ~9 o'clock the circuit really breaks up into velcro and sputter. |
| **Output** | −20 – +6 dB | Output level trim. Low bias settings drop volume significantly; use Output to compensate. |

## Signal Chain

```
Input → Drive → Asymmetric Dead-Zone Clipper → DC Block → Output Trim
```

The clipper models a transistor near cutoff: the positive side snaps hard from silence to saturation at the threshold boundary; the negative side backs out of saturation more gently. The dead zone between both thresholds widens as Bias drops, and audio-rate oscillation through this boundary is the velcro texture.

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
cmake --build build --config Release --target Broken
```

Artifacts land in `build/plugins/Broken/Broken_artefacts/Release/`:
- `AU/Broken.component`
- `VST3/Broken.vst3`
- `Standalone/Broken.app`

## Install

```bash
# AU
cp -R build/plugins/Broken/Broken_artefacts/Release/AU/Broken.component \
      ~/Library/Audio/Plug-Ins/Components/
codesign --force --sign "-" ~/Library/Audio/Plug-Ins/Components/Broken.component

# VST3
cp -R build/plugins/Broken/Broken_artefacts/Release/VST3/Broken.vst3 \
      ~/Library/Audio/Plug-Ins/VST3/
codesign --force --sign "-" ~/Library/Audio/Plug-Ins/VST3/Broken.vst3
```

Validate the AU with:

```bash
auval -v aufx BRKN CVDA
```

## License

GPL-3.0, see [LICENSE](LICENSE).
