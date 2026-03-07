# Vibeus

A real-time music visualizer that reacts to whatever audio is playing on your system. Powered by the [projectM](https://github.com/projectM-visualizer/projectm) rendering engine and thousands of community-created visual presets.

## Features

- **Live audio capture** — WASAPI loopback grabs system audio (no microphone needed)
- **9,800+ presets** — uses the [cream-of-the-crop](https://github.com/projectM-visualizer/presets-cream-of-the-crop) preset collection
- **Speed control** — slow down or speed up visualizations (0.05x – 4.0x)
- **Audio gain** — boost or dampen audio reactivity (0% – 300%)
- **Touch waveforms** — click and drag to spawn interactive waveforms
- **Debug mode** — FPS counter, preset info, and projectM log output

## Controls

| Key | Action |
|-----|--------|
| `N` / `→` | Next preset |
| `P` / `←` | Previous preset |
| `R` | Random preset (hard cut) |
| `H` | Go back in history |
| `S` | Toggle shuffle |
| `F` / `F11` | Toggle fullscreen |
| `↑` / `↓` | Adjust beat sensitivity |
| `[` / `]` | Slow down / speed up |
| `Backspace` | Reset speed to 1.0x |
| `-` / `=` | Audio gain down / up |
| `0` | Reset audio gain to 100% |
| `D` | Toggle debug overlay |
| Mouse click/drag | Spawn touch waveforms |
| `Q` / `Esc` | Quit |

## Building

### Prerequisites

- **Windows 10/11** (WASAPI audio capture)
- **CMake 3.21+**
- **Visual Studio 2022** (or Build Tools with MSVC)
- **vcpkg** (for SDL2)
- **projectM 4.x** — built from source

### Steps

1. **Clone and build projectM**:
   ```bash
   git clone --recurse-submodules https://github.com/projectM-visualizer/projectm.git
   cd projectm
   cmake -B build -G "Visual Studio 17 2022" -A x64
   cmake --build build --config Release
   ```

2. **Install SDL2 via vcpkg**:
   ```bash
   vcpkg install sdl2:x64-windows
   ```

3. **Build Vibeus**:
   ```bash
   cd Vibeus
   cmake -B build -G "Visual Studio 17 2022" -A x64 \
     -DCMAKE_TOOLCHAIN_FILE="<path-to-vcpkg>/scripts/buildsystems/vcpkg.cmake" \
     -DPROJECTM_ROOT="<path-to-projectm-source>"
   cmake --build build --config Release
   ```

4. **Set up presets** — place (or symlink) a preset folder at `build/Release/presets/`:
   ```bash
   git clone https://github.com/projectM-visualizer/presets-cream-of-the-crop.git
   mklink /J build\Release\presets presets-cream-of-the-crop
   ```

5. **Run**:
   ```bash
   build\Release\Vibeus.exe
   build\Release\Vibeus.exe --debug   # with debug overlay
   ```

## Project Structure

```
Vibeus/
  CMakeLists.txt          # Build configuration
  src/
    main.cpp              # SDL2 window, render loop, input handling
    audio_capture.h/cpp   # WASAPI loopback audio capture
    preset_manager.h/cpp  # Playlist wrapper around projectM playlist API
```

## License

This frontend is an independent project. [projectM](https://github.com/projectM-visualizer/projectm) is licensed under LGPL-2.1.

## Acknowledgments

- [projectM](https://github.com/projectM-visualizer/projectm) — the open-source Milkdrop-compatible music visualization library
- [Cream of the Crop presets](https://github.com/projectM-visualizer/presets-cream-of-the-crop) — curated preset collection
