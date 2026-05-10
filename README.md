# Vibeus

**A fully custom real-time music visualizer** that reacts to system audio using WASAPI loopback. Features a modern OpenGL-based rendering engine with multiple visual modes, beat-reactive effects, and a flexible abstraction layer.

## Highlights

- **VibeusVisualizer v0.5.0** with 4 visual modes:
  - Classic: Particles + spectrum + waveform + pulsing core + network connections
  - Nebula: Volumetric clouds, aurora spectrum, floating orbs
  - Tunnel: Receding rings, motion lines, flying particles
  - Symmetry: Kaleidoscopic mirrored bursts
- Dynamic shockwave rings and twinkling starfield on beats
- Advanced FFT spectrum analysis and multi-band beat detection
- Full IVisualizer interface — default custom backend, optional ProjectM wrapper
- Complete audio capture, preset system, touch interactions, debug tools

## Quick Start (Windows)

```bash
# Build
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release

# Run
build\Release\Vibeus.exe
```

Default: Custom Vibeus backend. Use `-DUSE_PROJECTM=ON` in CMake for legacy support.

## Controls

See in-app or previous docs for full list. Core: N/P next/prev, R random, speed/gain adjustments, mouse touch, D debug, I indicator.

## Cleanup Note

Development branches (migration/main-abstraction, overhaul, etc.) and log files have been cleaned. Main now contains the final polished codebase.

Built with ❤️ for music visualization enthusiasts.