# 🍺 BeerCrusher

> A VST3 audio plugin that crushes, saturates, and clips your signal — served with a cold, animated pint.

---

## What Is BeerCrusher?

BeerCrusher is a creative VST3 effect plugin built with [JUCE](https://juce.com/).  
It chains three classic lo-fi / saturation processes:

1. **Bitcrusher** — reduces bit depth for crunchy, digital grit.  
2. **Tube Saturation** — smooth harmonic drive modelled on a soft-knee transfer curve.  
3. **Peak Clipper** — hard-clipping stage with foam overflow indication.

The GUI features a **live-animated beer glass** that reacts to your audio in real time:  
the liquid level, bubbles, foam head, and wave surface all respond to the signal.

---

## Features

| Feature | Details |
|---|---|
| 🎛️ Bitcrusher | 1–16 bit depth reduction |
| 🔥 Tube Saturation | Soft-knee harmonic drive |
| ✂️ Peak Clipper | Hard-clip with foam overflow indicator |
| 🍺 Animated Beer Glass | 60 fps — bubbles, wave physics, foam, glass shine |
| 🎨 Custom Knobs | Copper/brass rotary knobs with amber glow arc |
| 💾 State Saving | Full preset recall via APVTS |

---

## Parameters

| Parameter | ID | Range | Default | Description |
|---|---|---|---|---|
| **Crush** | `crush` | 0 – 1 | 0.0 | Bitcrusher depth (0 = off, 1 = max crunch) |
| **Pour** | `drive` | 0 – 1 | 0.5 | Tube saturation drive amount |
| **Foam** | `foam` | 0 – 1 | 0.0 | Peak clipper threshold (more foam = lower threshold) |
| **Mix** | `wetdry` | 0 – 1 | 0.5 | Wet/dry blend (0 = dry, 1 = fully wet) |

---

## Building

### Prerequisites

- **CMake** ≥ 3.22  
- **JUCE** (included as a Git submodule at `libs/JUCE`)  
- **Visual Studio 2022** (Windows) or **Xcode** (macOS)

### Clone

```bash
git clone --recurse-submodules https://github.com/yourname/beercrusher.git
cd beercrusher
```

### Build locally

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel
```

The compiled `.vst3` bundle will appear inside `build/`.

### GitHub Actions (Recommended)

Every push to any branch automatically triggers a build.  
Download the compiled plugin from the **Actions** tab:

1. Navigate to **Actions** → select the latest workflow run.  
2. Download the **BeerCrusher-VST3-Windows** artifact.  
3. Extract and copy `BeerCrusher.vst3` to your VST3 folder:  
   `C:\Program Files\Common Files\VST3\`

---

## Project Structure

```
beercrusher/
├── CMakeLists.txt
├── Source/
│   ├── BeerDSP.h / .cpp          # DSP chain (bitcrusher, drive, clipper)
│   ├── BeerGlassComponent.h/.cpp # Animated beer glass UI
│   ├── BubbleParticle.h/.cpp     # Bubble particle physics
│   ├── KnobLookAndFeel.h/.cpp    # Custom copper knob renderer
│   ├── PluginProcessor.h/.cpp    # AudioProcessor + APVTS
│   └── PluginEditor.h/.cpp       # AudioProcessorEditor
├── libs/
│   └── JUCE/                     # JUCE submodule
└── .github/
    └── workflows/
        └── build.yml             # CI/CD — builds VST3 on Windows
```

---

## License

MIT © 2024 — do whatever you like with it. Cheers! 🍻
