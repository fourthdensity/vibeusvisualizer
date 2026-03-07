# Vibeus — Road to Launch

> Target: Steam Early Access → Full Release (Windows + macOS)
> Price: $1.99–$2.99 | Niche: Interactive music visualization for enthusiasts

---

## Competitor Intelligence — What Users Are Saying

### projectM (Steam — Free, Overwhelmingly Positive, 753 reviews)
**What they love:** "Limitless presets", "psychedelic visuals", Milkdrop legacy
**What they hate:**
- "60% of presets simply don't work — black screen, white screen, or overstimulating static"
- Freezes/deadlocks with certain presets (GitHub #408 — open since 2020)
- "Way too chaotic, even with beat sensitivity turned down" — no speed/intensity control
- Audio capture confusion (doesn't react to music on Android TV, loopback issues)
- No preset curation — 40,000+ presets with no way to filter good from bad
- No favorites, no playlists, no preset management of any kind
- Bare-bones UI — "press F1 for help" is the onboarding
- No controller/gamepad support
- No Steam Deck verification

### Plane9 (Steam — $13.99)
**What they love:** 3D scenes, VR support, scene-based variety
**What they hate:**
- Single developer (Joakim Dahl) = slow updates
- No community preset/scene sharing ecosystem
- Limited interactivity — it's a screensaver, not a tool
- High price for what it offers vs free alternatives

### General Market Sentiment (Reddit threads 2024-2025)
- "What happened to music visualizers?" — massive nostalgia for Winamp/Milkdrop era
- "Given graphics tech is amazing these days — why is nobody making these anymore?"
- "Surely there'd be enough of a market for people to make something great and modern?"
- Xbox 360 Visualize Sound "undefeated even in 2024" — people WANT this experience
- "Music is not a spreadsheet. Humans love watching something while listening to music."
- ADHD community: "I bought a music visualizer and now I can't get any work done cause I'm hypnotized"
- Couples/chill use case: "Lights off, just the music, visuals and us. Trippy but really calming."

### Vibeus Competitive Advantages (what NOBODY else offers)
| Feature | projectM Steam | Plane9 | Vibeus |
|---------|---------------|--------|--------|
| Preset browser + search | No | No | **Yes** |
| Favorites system | No | No | **Yes** |
| Gamepad/controller | No | No | **Yes** |
| Flow mode (mouse = visuals) | No | No | **Yes** |
| Speed control | No | Limited | **Yes** |
| Audio gain control | No | No | **Yes** |
| Pause + frozen frame | No | No | **Yes** |
| Epilepsy warning | No | No | **Yes** |
| Glassmorphism UI | No | No | **Yes** |
| Touch waveforms | Basic | No | **Advanced** |
| Curated preset packs | No | Scene-based | **Planned** |

---

## What's Done — v0.2.0 ✅

- [x] Epilepsy warning splash screen
- [x] Main menu (glassmorphism UI — Start, Browse Presets, Settings, Exit)
- [x] Pause menu (Resume, Browse, Settings, Record, Exit)
- [x] Preset browser (search, favorites, double-click to play, save/load)
- [x] App state machine (Splash → Main Menu → Visualizer)
- [x] Background visualization behind menu screens
- [x] Dear ImGui integration (SDL2 + OpenGL3)
- [x] Gamepad support (A/B/X/Y, Start=pause, analog sticks, D-pad, triggers)
- [x] Flow mode (Tab — mouse position controls speed + waveforms)
- [x] Pause/resume fixes (event isolation, render limiting, delta time cap)
- [x] Touch waveforms (persist, type cycling, pressure, clear)

---

## Milestone 1 — "Playable Product" (v0.3.0)
> Goal: Something you'd be proud to show anyone

- [ ] **Settings panel** _(HIGH PRIORITY — users complain about NO controls)_
  - Speed slider (0.1x–5x)
  - Audio gain slider (0–5x)
  - Beat sensitivity slider (addresses "way too chaotic" complaint)
  - Transition duration / auto-advance timer
  - Fullscreen toggle
  - Save/load user config to JSON file
- [ ] **Preset quality curation** _(addresses "60% don't work" complaint)_
  - Test-load presets on startup, flag broken ones (black/white/crash)
  - "Verified" badge for tested presets in browser
  - Default to curated subset, option to show all
- [ ] **Credits / Licenses / About screen**
  - Third-party license notices (LGPL, MIT, zlib, Apache — see audit)
  - Version info, GitHub link
  - Preset community credit
- [ ] **Keyboard shortcut overlay / help screen**
  - Show all controls in a styled overlay (F1 or ?)
  - Gamepad control diagram
- [ ] _Open slot_
- [ ] _Open slot_

## Milestone 2 — "Feature Complete" (v0.4.0)
> Goal: Match or beat every competitor feature

- [ ] **Preset flows / playlists** _(addresses "no playlist" gap)_
  - User-created ordered preset sequences
  - Timed auto-advance or manual stepping
  - Export/import flows as shareable JSON files
- [ ] **Screenshot & recording**
  - Screenshot capture (PNG, one keypress)
  - Video recording (encode to MP4 or GIF)
  - Share-ready output sizes
- [ ] **Audio source selection** _(addresses "doesn't react to music" complaints)_
  - Choose specific audio device / app
  - Microphone input option
  - Visual indicator of audio level
- [ ] **Smooth preset transitions**
  - Crossfade between presets (configurable duration)
  - Optional hard cut mode
- [ ] **Steam Deck compatibility** _(no competitor is verified)_
  - Full gamepad-only navigation (already mostly there)
  - UI scaling for handheld resolution
  - Verify controller layout in Steam Input
- [ ] _Open slot_
- [ ] _Open slot_

## Milestone 3 — "Steam Ready" (v0.5.0)
> Goal: Ship to Steam Early Access

- [ ] **Steamworks integration**
  - Developer account setup ($100)
  - Steam overlay compatibility (OpenGL hook testing)
  - Steam Cloud for favorites/flows/settings sync
  - Steam Input API (user-remappable controls)
  - Rich presence ("Vibing to preset: [name]")
- [ ] **Store page assets**
  - 5+ high-quality screenshots
  - 30-60 second trailer video
  - Store description, tags, categories
  - Capsule art / logo / branding
- [ ] **Bundled content**
  - Curated "Vibeus Essentials" preset pack (100-200 handpicked)
  - Full cream-of-the-crop pack as optional content
  - Organized into mood/genre folders
- [ ] **Installer / packaging**
  - Windows: NSIS installer or portable zip
  - Include all DLLs, presets, config defaults
  - Auto-detect audio devices on first run
- [ ] **QA & polish**
  - Multi-monitor testing
  - GPU compatibility sweep (Intel/AMD/NVIDIA)
  - Memory leak / long-session stability testing
  - Clean shutdown (save state on exit)
- [ ] _Open slot_
- [ ] _Open slot_

## Milestone 4 — "Full Release" (v1.0.0)
> Goal: Leave Early Access, macOS launch, Workshop

- [ ] **macOS port**
  - Core Audio loopback (replace WASAPI)
  - CMake macOS build (Xcode generator)
  - Code-sign + notarize
  - Universal binary (Intel + Apple Silicon)
- [ ] **Steam Workshop**
  - Upload/download community presets
  - Preset rating & voting
  - "Featured preset of the week"
  - Browse by tag/mood/genre
- [ ] **Steam achievements**
  - "First Vibe" — run visualizer for 1 minute
  - "Curator" — favorite 50 presets
  - "Flow Master" — create a flow with 10+ presets
  - "Night Owl" — 10 cumulative hours
  - "Hypnotized" — 100 cumulative hours
- [ ] **Advanced visual features**
  - Custom transitions (melt, implode, absorb)
  - User sprites / artwork overlay via API
  - Album art display (if metadata available)
- [ ] _Open slot_
- [ ] _Open slot_

---

## Post-Launch Roadmap

### DLC / Content Packs
- [ ] Themed preset packs (Chill, Psychedelic, Minimal, Party, Lo-fi)
- [ ] Artist collaboration packs
- [ ] Holiday/seasonal packs

### Mobile — "Pocket Vibes"
- [ ] Android: OpenGL ES 3.0 + Oboe audio + SDL2
- [ ] iOS: OpenGL ES 3.0 / Metal + Core Audio + SDL2
- [ ] Touch-native controls (swipe, pinch, two-finger drag)
- [ ] Google Play / App Store distribution

### Future Vision
- [ ] Hand tracking / gesture control (MediaPipe / webcam)
- [ ] MIDI controller support (knobs → parameters)
- [ ] Streaming integration (Spotify / system media metadata)
- [ ] Multi-monitor support (different presets per display)
- [ ] VR mode (OpenXR / SteamVR — 360° immersion)

---

## License Compliance Checklist (must complete before selling)

- [ ] Ship LGPL-2.1 license text (projectM) — include in NOTICES file
- [ ] Ship MIT license text (Dear ImGui, GLM, hlslparser, projectm-eval, stb_image)
- [ ] Ship zlib license text (SDL2) — include copyright notice
- [ ] Ship Apache-2.0 notice (GLAD/Khronos headers)
- [ ] Add "About / Credits" screen in Settings with all attributions
- [ ] Credit Milkdrop/projectM community for presets
- [ ] Confirm dynamic linking only (DLLs) — no static LGPL linking
- [ ] Review final binary with dependency walker before release

---

_Last updated: March 7, 2026_
_Roadmap maintained at: https://github.com/fourthdensity/vibeusvisualizer_
