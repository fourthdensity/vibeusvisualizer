# Vibeus Roadmap

## Phase 2 — Core UX Polish

- [ ] **Menu / HUD overlay**
  - Pause button (freeze frame + pause audio feed)
  - Preset browser (list view, search, favorites)
  - Settings panel (speed, gain, beat sensitivity sliders)
  - Semi-transparent overlay, toggle with `M` or `Tab`
  - ESC closes menu before quitting app

- [ ] **Pause functionality**
  - Freeze visualization (stop feeding frames/audio)
  - `Space` to toggle pause
  - Visual indicator (dim overlay or "PAUSED" text)

- [ ] **Fix mouse/touch waveforms**
  - Current bug: waveforms destroyed immediately on mouse-up (feels like nothing happens)
  - Fix: let waveforms persist after mouse-up, add `C` key to clear all (`projectm_touch_destroy_all`)
  - Test all touch types: circle, radial blob, blob2/3, derivative line, blob5, line, double line
  - Consider right-click to cycle touch types
  - Add scroll wheel to adjust touch "pressure" parameter

## Phase 3 — Visual Enhancements

- [ ] **Custom transitions**
  - Melt, implode, absorb effects between presets
  - Configurable transition duration

- [ ] **Parallax / depth effects**
  - Layer-based depth on waveforms

- [ ] **Custom artwork / user sprites**
  - Import images via `user_sprites.h` API
  - Overlay logos, album art, custom graphics

## Phase 4 — Mobile ("Pocket Vibes")

- [ ] **Platform research**
  - Android: OpenGL ES 3.0, Oboe audio capture, SDL2 Android backend
  - iOS: OpenGL ES 3.0 / Metal, Core Audio, SDL2 iOS backend
  - projectM already supports GLES — check `cmake/gles/FindOpenGL.cmake`

- [ ] **Audio input on mobile**
  - Android: microphone capture (Oboe/AAudio) since loopback needs root
  - iOS: AVAudioSession microphone input
  - Consider Bluetooth audio latency

- [ ] **Touch-native controls**
  - Swipe left/right for preset navigation
  - Pinch to zoom
  - Two-finger drag for settings
  - Native touch waveform interaction (already in projectM API)

- [ ] **App packaging**
  - Android: Gradle + NDK build, APK/AAB
  - iOS: Xcode project, App Store distribution
  - Shared C++ core, platform-specific audio/input layers

## Backlog

- [ ] Preset favorites / rating system
- [ ] Audio source selection (specific app, mic, line-in)
- [ ] Recording / screenshot capture
- [ ] MIDI controller support
- [ ] Streaming integration (Spotify metadata overlay)
- [ ] Multi-monitor support
