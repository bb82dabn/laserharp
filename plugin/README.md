# LaserHarp VST3

Native VST3-only port of the browser LaserHarp instrument.

## Build

Requirements:

- CMake 3.22+
- C++17 compiler
- Internet access on the first configure so CMake FetchContent can clone JUCE

```bash
cmake -S plugin -B plugin/build -DCMAKE_BUILD_TYPE=Release
cmake --build plugin/build --config Release
```

Override the JUCE version with `-DJUCE_GIT_TAG=8.0.0` if you need a different commit.

## Controls

- 13 laser strings mapped to mouse X position.
- Click and drag with the left mouse to play. MIDI notes also trigger strings.
- Hold the right mouse button while the left button is held for modulation.
- Waveform, scale, key, filter cutoff, reverb, delay, and output gain are DAW parameters.
