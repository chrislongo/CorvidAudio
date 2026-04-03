# CLAUDE.md

Monorepo for Corvid Audio JUCE plugins.

## Build

All plugins build from the repo root with a single CMake invocation.

**Configure** (once, or after CMakeLists.txt changes):
```bash
cmake -B build -G Ninja \
    -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0 \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_C_COMPILER=$(xcrun -f clang) \
    -DCMAKE_CXX_COMPILER=$(xcrun -f clang++)
```

**Build all plugins:**
```bash
cmake --build build --config Release
```

**Build a single plugin:**
```bash
cmake --build build --config Release --target TwoOpFM   # 2-OP
cmake --build build --config Release --target Dist308    # Dist308
cmake --build build --config Release --target Life       # Life
cmake --build build --config Release --target LocBox     # Loc-Box
```

JUCE and eurorack must be siblings of this repo (`../JUCE`, `../eurorack`), or override with `-DJUCE_DIR=... -DEURORACK_DIR=...`.

## Testing

No automated tests. `auval` is the primary correctness check:
```bash
auval -v aumu TWOP CVDA   # 2-OP (instrument)
auval -v aufx D308 CVDA   # Dist308
auval -v aufx LIFE CVDA   # Life
auval -v aufx LBOX CVDA   # Loc-Box
```

## Plugins

| Plugin | CMake target | Type | Description |
|--------|-------------|------|-------------|
| **2-OP** | `TwoOpFM` | Instrument | 2-operator FM synth (Plaits engine) |
| **Dist308** | `Dist308` | Effect | ProCo Rat-inspired distortion |
| **Life** | `Life` | Effect | Analog character & warmth |
| **Loc-Box** | `LocBox` | Effect | Shure Level Loc brickwall limiter |

All plugins: CorvidAudio manufacturer, AU + VST3 + Standalone formats, macOS 11.0+ universal binary.

## Repo structure

```
CorvidAudio/
├── CMakeLists.txt              # Top-level: loads JUCE, adds all plugins
├── shared/
│   ├── CMakeLists.txt          # INTERFACE library (header-only)
│   └── CorvidLookAndFeel.h     # BlackKnobLookAndFeel, PillToggle, paintPanelBackground
└── plugins/
    ├── 2-OP/                   # FM synth (own LAFs, depends on eurorack)
    ├── Dist308/                # Distortion (own BlackKnobLookAndFeel variant)
    ├── Life/                   # Analog character (uses shared LAF + PillToggle)
    └── Loc-Box/                # Limiter (uses shared LAF)
```

## Shared code (`shared/CorvidLookAndFeel.h`)

- `corvid::paintPanelBackground()` — silver-grey #d8d8d8 + gradient overlay
- `corvid::BlackKnobLookAndFeel` — matte-black knob with 7 tick marks + white indicator
- `corvid::PillToggle` — pill-shaped APVTS bool toggle component

Life and Loc-Box use the shared LAF. Dist308 has its own variant (drop shadow, no ticks). 2-OP has unique LAFs (FMSliderLookAndFeel, ADSRKnobLookAndFeel, PillLookAndFeel).

## Design language

- **Panel**: silver-grey `#d8d8d8`, subtle top→bottom gradient overlay, flat — no bevels or shadows
- **Controls**: matte-black `#111111` with white indicator lines
- **Labels**: 10–13pt bold, dark grey, all-caps for parameter names

## JUCE conventions

- `juce_generate_juce_header(<Target>)` is required — without it `<JuceHeader.h>` is missing.
- Use `juce::Font(juce::FontOptions().withHeight(n))` — the `Font(float)` constructor is deprecated.
- `AudioProcessorEditor` already has a `processor` member; don't shadow it.
- Index `std::array<SmoothedValue>` with `static_cast<size_t>(ch)` to avoid `-Wsign-conversion`.
