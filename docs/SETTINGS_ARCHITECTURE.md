# Vibeus Settings Architecture

## System Diagram

```
┌─────────────────────────────────────────────────────────────────────────┐
│                           USER INTERFACE                                 │
│  ┌─────────────────────────────────────────────────────────────────┐   │
│  │                     MenuOverlay (ImGui)                          │   │
│  │  ┌──────────┬──────────┬──────────┬──────────┐                  │   │
│  │  │  Vibes   │ Presets  │ Display  │ Advanced │  ← Tab Bar       │   │
│  │  └──────────┴──────────┴──────────┴──────────┘                  │   │
│  │                                                                  │   │
│  │  ┌─────────────────────────────────────────────────────────┐    │   │
│  │  │  Sliders, Checkboxes, Dropdowns                         │    │   │
│  │  │  • Real-time changes update m_config pointer            │    │   │
│  │  │  • 'changed' flag triggers MenuAction::ApplySettings    │    │   │
│  │  └─────────────────────────────────────────────────────────┘    │   │
│  └─────────────────────────────────────────────────────────────────┘   │
└───────────────────────────────────────────────────────────────────────────┘
                                    │
                                    │ changed = true
                                    │ returns MenuAction::ApplySettings
                                    ▼
┌─────────────────────────────────────────────────────────────────────────┐
│                         MAIN APPLICATION                                 │
│  ┌─────────────────────────────────────────────────────────────────┐   │
│  │                      main.cpp event loop                         │   │
│  │                                                                  │   │
│  │  if (action == MenuAction::ApplySettings)                       │   │
│  │      applyConfig(g_config);   ──────────────────────────────────┼───┼─┐
│  │                                                                  │   │ │
│  │  if (action == MenuAction::BackFromSettings)                    │   │ │
│  │      saveConfig(g_config, g_configPath);  ──────────────────────┼───┼─┼─┐
│  │      applyConfig(g_config);                                     │   │ │ │
│  └─────────────────────────────────────────────────────────────────┘   │ │ │
└───────────────────────────────────────────────────────────────────────────┘ │ │
                                                                             │ │
     ┌───────────────────────────────────────────────────────────────────────┘ │
     │                                                                         │
     ▼                                                                         │
┌────────────────────────────────────────┐        ┌────────────────────────────┘
│         applyConfig()                   │        │
│  ┌────────────────────────────────┐    │        ▼
│  │  projectM API Calls            │    │   ┌──────────────────────────┐
│  │  ├─ set_beat_sensitivity()     │    │   │    config.cpp            │
│  │  ├─ set_preset_duration()      │    │   │  ┌────────────────────┐  │
│  │  ├─ set_soft_cut_duration()    │    │   │  │   saveConfig()     │  │
│  │  ├─ set_hard_cut_*()           │    │   │  │   JSON → disk      │  │
│  │  ├─ set_mesh_size()        NEW │    │   │  └────────────────────┘  │
│  │  ├─ set_aspect_correction() NEW│    │   │  ┌────────────────────┐  │
│  │  ├─ set_easter_egg()       NEW │    │   │  │   loadConfig()     │  │
│  │  ├─ set_preset_start_clean()NEW│    │   │  │   disk → struct    │  │
│  │  └─ set_fps()              NEW │    │   │  └────────────────────┘  │
│  ├────────────────────────────────┤    │   └──────────────────────────┘
│  │  SDL Calls                     │    │
│  │  ├─ SDL_GL_SetSwapInterval()NEW│    │
│  │  └─ SDL_SetWindowFullscreen()  │    │
│  ├────────────────────────────────┤    │
│  │  Global State                  │    │
│  │  ├─ g_audioGain                │    │
│  │  ├─ g_speedMultiplier          │    │
│  │  ├─ g_flashLimiter             │    │
│  │  └─ g_reducedMotion            │    │
│  └────────────────────────────────┘    │
└────────────────────────────────────────┘
             │
             │ Settings Applied
             ▼
┌─────────────────────────────────────────────────────────────────────────┐
│                          projectM Engine                                 │
│  ┌─────────────────────────────────────────────────────────────────┐   │
│  │  Internal State (set by API calls)                               │   │
│  │  ├─ m_beatSensitivity                                           │   │
│  │  ├─ m_presetDuration                                            │   │
│  │  ├─ m_softCutDuration                                           │   │
│  │  ├─ m_hardCut* settings                                         │   │
│  │  ├─ m_meshWidth, m_meshHeight                                   │   │
│  │  ├─ m_aspectCorrection                                          │   │
│  │  ├─ m_easterEgg                                                 │   │
│  │  └─ m_fps                                                        │   │
│  └─────────────────────────────────────────────────────────────────┘   │
│                                                                         │
│  Render Loop: projectm_opengl_render_frame() ───────────────────────►  │
│                                                                         │
└─────────────────────────────────────────────────────────────────────────┘
```

## Data Flow

```
┌──────────────┐    User Input    ┌──────────────┐   Real-time    ┌──────────────┐
│   UI Widget  │ ───────────────► │ VibeusConfig │ ─────────────► │  applyConfig │
│   (ImGui)    │                  │   (struct)   │                │  (function)  │
└──────────────┘                  └──────────────┘                └──────────────┘
                                         │                               │
                                         │ On Back/Exit                  │ Immediate
                                         ▼                               ▼
                                  ┌──────────────┐                ┌──────────────┐
                                  │  saveConfig  │                │  projectM    │
                                  │  (to JSON)   │                │  + SDL       │
                                  └──────────────┘                └──────────────┘
```

## Config Field Categories

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                          VibeusConfig Struct                                │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  ┌─ VISUAL INTENSITY ────────────────┐  ┌─ PRESET BEHAVIOR ─────────────┐  │
│  │  • beatSensitivity  → projectM    │  │  • autoAdvance     → local    │  │
│  │  • audioGain        → local       │  │  • presetDuration  → projectM │  │
│  │  • speedMultiplier  → local       │  │  • shuffle         → playlist │  │
│  │  • flashLimiter     → local       │  │  • transitionTime  → projectM │  │
│  │  • reducedMotion    → local       │  │  • vibeLock        → projectM │  │
│  └───────────────────────────────────┘  │  • hardCut*        → projectM │  │
│                                         │  • easterEgg    NEW→ projectM │  │
│  ┌─ VISUAL QUALITY ──────────────────┐  └────────────────────────────────┘  │
│  │  • meshQuality    NEW→ projectM   │                                      │
│  │  • aspectCorrect  NEW→ projectM   │  ┌─ DISPLAY ──────────────────────┐  │
│  │  • cleanTransit   NEW→ projectM   │  │  • fullscreen    → SDL         │  │
│  └───────────────────────────────────┘  │  • showFps       → local       │  │
│                                         │  • uiScale       → ImGui       │  │
│  ┌─ PERFORMANCE ─────────────────────┐  │  • fontScale     → ImGui       │  │
│  │  • perfMode         → meta-preset │  │  • overlayOpacity→ ImGui       │  │
│  │  • targetFps     NEW→ projectM    │  └────────────────────────────────┘  │
│  │  • vsyncEnabled  NEW→ SDL         │                                      │
│  └───────────────────────────────────┘  ┌─ INPUT ─────────────────────────┐ │
│                                         │  • touchEnabled  → local        │ │
│  ┌─ META ────────────────────────────┐  │  • flowMode      → local        │ │
│  │  • mood             → meta-preset │  │  • gamepadDeadzone→ SDL        │ │
│  └───────────────────────────────────┘  └─────────────────────────────────┘ │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘

Legend:
  → projectM   = Applied via projectm_set_*() API
  → local      = Stored in global variables, applied in render loop
  → SDL        = Applied via SDL_* functions
  → ImGui      = Applied to ImGui::GetIO() or style
  → playlist   = Applied via projectm_playlist_*() API
  → meta-preset= Applies multiple other settings
```

## UI Layout (4-Tab Design)

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                              SETTINGS                                       │
├───────────────┬───────────────┬───────────────┬─────────────────────────────┤
│    Vibes ●    │    Presets    │    Display    │         Advanced            │
├───────────────┴───────────────┴───────────────┴─────────────────────────────┤
│                                                                             │
│  ┌─ Mood Preset ────────────────────────────────────────────────────────┐  │
│  │  [ Chill ▼ ]  Party • Focus • Psychedelic • Custom                   │  │
│  └──────────────────────────────────────────────────────────────────────┘  │
│                                                                             │
│  ┌─ Audio Reactivity ───────────────────────────────────────────────────┐  │
│  │                                                                       │  │
│  │  Beat Reactivity   ═══════════════●═══════════════   [1.0]  (?)      │  │
│  │                     mellow                 intense                    │  │
│  │                                                                       │  │
│  │  Audio Boost       ═══════════●═══════════════════   [100%] (?)      │  │
│  │                     quiet              LOUD                           │  │
│  │                                                                       │  │
│  │  [ ] Flash Limiter  (caps reactivity for photosensitivity)   (?)     │  │
│  └──────────────────────────────────────────────────────────────────────┘  │
│                                                                             │
│  ┌─ Motion ─────────────────────────────────────────────────────────────┐  │
│  │                                                                       │  │
│  │  Animation Speed   ═══════════════●═══════════════   [1.0x] (?)      │  │
│  │                     0.1x         normal          4x                   │  │
│  │                                                                       │  │
│  │  [ ] Reduced Motion  (halves animation speed for comfort)    (?)     │  │
│  └──────────────────────────────────────────────────────────────────────┘  │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────────────────┐
│                              SETTINGS                                       │
├───────────────┬───────────────┬───────────────┬─────────────────────────────┤
│     Vibes     │   Presets ●   │    Display    │         Advanced            │
├───────────────┴───────────────┴───────────────┴─────────────────────────────┤
│                                                                             │
│  ┌─ Playback ───────────────────────────────────────────────────────────┐  │
│  │                                                                       │  │
│  │  [✓] Auto-Advance                  [✓] Shuffle                       │  │
│  │                                                                       │  │
│  │  Duration         ═════════════════●═════════════   [30 sec] (?)     │  │
│  │                   10 sec                   2 min                      │  │
│  │                                                                       │  │
│  │  Duration Variance═══●═══════════════════════════   [0.3]    (?)     │  │
│  │                   off   subtle        chaotic                         │  │
│  │                                                                       │  │
│  │  ┌─────────────────────────────────────────────────────────────┐     │  │
│  │  │        >> PRESET LOCKED — Click to Unlock <<                │     │  │
│  │  └─────────────────────────────────────────────────────────────┘     │  │
│  └──────────────────────────────────────────────────────────────────────┘  │
│                                                                             │
│  ┌─ Transitions ────────────────────────────────────────────────────────┐  │
│  │                                                                       │  │
│  │  Crossfade Time   ═══════●═══════════════════════   [3.0 sec] (?)    │  │
│  │                   instant         smooth                              │  │
│  │                                                                       │  │
│  │  [ ] Clean Start  (black canvas between presets)             (?)     │  │
│  │                                                                       │  │
│  │  ── Beat-Reactive Cuts ──────────────────────────────────────        │  │
│  │  [ ] Enable beat-triggered instant transitions               (?)     │  │
│  │                                                                       │  │
│  │      Trigger Sensitivity  ═══════●═══════════════   [2.0]    (?)     │  │
│  │      Minimum Delay        ═══════════●═══════════   [20 sec] (?)     │  │
│  └──────────────────────────────────────────────────────────────────────┘  │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────────────────┐
│                              SETTINGS                                       │
├───────────────┬───────────────┬───────────────┬─────────────────────────────┤
│     Vibes     │    Presets    │   Display ●   │         Advanced            │
├───────────────┴───────────────┴───────────────┴─────────────────────────────┤
│                                                                             │
│  ┌─ Performance ────────────────────────────────────────────────────────┐  │
│  │                                                                       │  │
│  │  Quality Preset   [ Quality ▼ ]  Battery Saver • Balanced • Quality  │  │
│  │                                                                  (?)  │  │
│  └──────────────────────────────────────────────────────────────────────┘  │
│                                                                             │
│  ┌─ Visual Quality ─────────────────────────────────────────────────────┐  │
│  │                                                                       │  │
│  │  Mesh Detail      ═══════════════●═══════════════   [64]     (?)     │  │
│  │                   8 (fast)    48 (balanced)    200 (smooth)           │  │
│  │                                                                       │  │
│  │  [✓] Aspect Correction  (fix stretching on ultrawide)        (?)     │  │
│  │  [✓] VSync              (prevent tearing)                    (?)     │  │
│  │                                                                       │  │
│  │  Target FPS       [ 60 ▼ ]  30 • 60 • 120 • Unlimited        (?)     │  │
│  └──────────────────────────────────────────────────────────────────────┘  │
│                                                                             │
│  ┌─ Window ─────────────────────────────────────────────────────────────┐  │
│  │                                                                       │  │
│  │  [ ] Fullscreen (F11)                                                │  │
│  │  [ ] Show FPS in title bar                                           │  │
│  │                                                                       │  │
│  │  Overlay Opacity  ════════════●══════════════════   [65%]    (?)     │  │
│  │                   see-through        opaque                           │  │
│  └──────────────────────────────────────────────────────────────────────┘  │
│                                                                             │
│  ┌─ Accessibility ──────────────────────────────────────────────────────┐  │
│  │                                                                       │  │
│  │  UI Scale         ═══════════════●═══════════════   [100%]   (?)     │  │
│  │  Font Scale       ═══════════════●═══════════════   [100%]   (?)     │  │
│  └──────────────────────────────────────────────────────────────────────┘  │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────────────────┐
│                              SETTINGS                                       │
├───────────────┬───────────────┬───────────────┬─────────────────────────────┤
│     Vibes     │    Presets    │    Display    │       Advanced ●            │
├───────────────┴───────────────┴───────────────┴─────────────────────────────┤
│                                                                             │
│  ┌─ Input ──────────────────────────────────────────────────────────────┐  │
│  │                                                                       │  │
│  │  [✓] Touch Waveforms  (click/drag creates effects)           (?)     │  │
│  │  [ ] Flow Mode        (mouse position controls speed)        (?)     │  │
│  │                                                                       │  │
│  │  Gamepad Deadzone ═════════●═════════════════════   [8000]   (?)     │  │
│  │                   2000 (sensitive)      16000 (stiff)                 │  │
│  └──────────────────────────────────────────────────────────────────────┘  │
│                                                                             │
│  ┌─ Rendering ──────────────────────────────────────────────────────────┐  │
│  │                                                                       │  │
│  │  Texel Offset X   ═══════════════●═══════════════   [0.50]   (?)     │  │
│  │  Texel Offset Y   ═══════════════●═══════════════   [0.50]   (?)     │  │
│  │                                                                       │  │
│  │  These affect warp shader alignment. MilkDrop default is 0.5.        │  │
│  └──────────────────────────────────────────────────────────────────────┘  │
│                                                                             │
│  ┌─ Developer ──────────────────────────────────────────────────────────┐  │
│  │                                                                       │  │
│  │  [ ] Debug Logging  (verbose console output)                         │  │
│  │  [ ] Show Preset Path  (display filename in overlay)                 │  │
│  │                                                                       │  │
│  │  projectM Version: 4.1.0                                             │  │
│  │  Presets Loaded: 247                                                 │  │
│  └──────────────────────────────────────────────────────────────────────┘  │
│                                                                             │
│                    ┌───────────────────────────────┐                       │
│                    │    Reset All to Defaults      │                       │
│                    └───────────────────────────────┘                       │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘
```

## Priority Matrix

```
                          IMPACT ON USER EXPERIENCE
                    Low ◄────────────────────────► High
                    │                               │
               High │  Texel Offset    ████████████│ Mesh Quality ███████
                    │                  ████████████│ Aspect Correction ██
                    │                               │ PerfMode (working) █
    EASE OF         │  Clean Start ███ ████████████│
    IMPLEMENTATION  │  Easter Egg ████ ████████████│
                    │                               │
                    │                               │
                    │  VSync/FPS ██████████████████│ Beat Reactivity ████
               Low  │                               │ (already works)
                    │                               │
                    └───────────────────────────────┘

Legend:
  ███ = Phase 1 (Quick Wins)
  ███ = Phase 2 (UI Work)
  ███ = Phase 3+ (Future)
```
