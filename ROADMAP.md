# Vibeus Roadmap

## v0.2.0 — Done ✅

- [x] **Epilepsy warning splash screen**
- [x] **Main menu** (glassmorphism UI — Start, Browse Presets, Settings, Exit)
- [x] **Pause menu** (Resume, Browse, Settings, Record, Exit)
- [x] **Preset browser** (search, favorites, double-click to play, save/load favorites)
- [x] **App state machine** (Splash → Main Menu → Visualizer)
- [x] **Background visualization** behind menu screens
- [x] **Dear ImGui integration** (SDL2 + OpenGL3 backend)
- [x] **Gamepad support** (A/B/X/Y presets, Start=pause, analog sticks, D-pad, triggers)
- [x] **Flow mode** (Tab — mouse position controls speed + generates waveforms)
- [x] **Pause/resume fixes** (event isolation, render limiting, delta time cap)
- [x] **Touch waveforms** (persist after release, type cycling, pressure, clear)

## Phase 3 — Polish & Settings

- [ ] **Settings panel**
  - Speed slider (0.1x–5x)
  - Audio gain slider (0–5x)
  - Beat sensitivity slider
  - Transition duration slider
  - Fullscreen toggle
  - Audio device selection (specific app, mic, line-in)
  - Save/load user preferences to config file

- [ ] **Recording & screenshots**
  - Screenshot capture (PNG export)
  - Video recording (encode to MP4 / GIF)
  - Configurable output directory

- [ ] **Visual enhancements**
  - Custom transitions (melt, implode, absorb between presets)
  - Parallax / depth effects on waveforms
  - Custom artwork / user sprites via `user_sprites.h` API
  - Overlay logos, album art, custom graphics

- [ ] **Preset flow / playlists**
  - User-created ordered preset sequences ("flows")
  - Timed auto-advance or manual stepping
  - Export/import flows as shareable files

## Phase 4 — Steam Release (Windows + macOS)

- [ ] **Steam distribution**
  - Create Steamworks developer account ($100 fee)
  - Price point: $0.99–$2.99 (impulse buy territory)
  - Steam store page: screenshots, trailer video, description
  - Steam achievements (e.g., "Browsed 100 presets", "Created first flow", "10 hours visualizing")
  - Steam Workshop integration for community preset sharing
  - Steam Cloud for syncing favorites/flows across machines

- [ ] **macOS port**
  - SDL2 + OpenGL 3.3 already cross-platform
  - Audio capture: Core Audio loopback (replace WASAPI)
  - CMake build for macOS (Xcode generator or Makefiles)
  - Code-sign and notarize for macOS distribution
  - Universal binary (Intel + Apple Silicon)

- [ ] **Windows polish for release**
  - Installer (NSIS or WiX) or portable zip
  - Bundled preset packs (cream-of-the-crop, community favorites)
  - App icon and branding assets
  - First-run setup wizard (audio device, preset directory)

- [ ] **Steamworks integration**
  - Steam overlay compatibility (OpenGL hook)
  - Rich presence ("Listening to: [song] with preset: [name]")
  - Steam Input API (replaces raw SDL gamepad — lets users remap)
  - Automatic updates via Steam

- [ ] **Content & presets**
  - Bundle curated preset packs as DLC or included
  - Community preset sharing via Steam Workshop
  - Preset rating/voting system
  - Featured preset of the week

## Phase 5 — Mobile ("Pocket Vibes")

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
  - Android: Gradle + NDK build, APK/AAB → Google Play
  - iOS: Xcode project → App Store
  - Shared C++ core, platform-specific audio/input layers

## Phase 6 — Future Vision

- [ ] **Hand tracking / gesture control**
  - Camera-based hand tracking (MediaPipe or OpenCV)
  - "Orchestra conductor" mode — wave hands to control visuals
  - Depth sensing with webcam for Z-axis control

- [ ] **MIDI controller support**
  - Map knobs/faders to speed, gain, beat sensitivity
  - Pad triggers for preset switching
  - DJ controller integration

- [ ] **Streaming integration**
  - Spotify / system media metadata overlay
  - Song title, artist, album art display
  - Auto-tag presets to genres/moods

- [ ] **Multi-monitor support**
  - Span visualization across displays
  - Different presets per monitor
  - Control panel on one screen, visuals on another

- [ ] **VR mode**
  - OpenXR / SteamVR integration
  - 360° immersive visualization
  - Hand controller interaction in VR space
