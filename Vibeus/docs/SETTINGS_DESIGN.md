# Vibeus — Settings Panel Design Document

> Product Design Research | v0.3.0 Milestone
> Two-tab settings: **Basic** (anyone) and **Advanced** (tinkerers)

---

## Part 1: Persona Analysis

---

### 🌙 Luna — Casual Chill User

**Profile:** Uses Vibeus to decompress after work. Lo-fi beats, dimmed lights, doesn't touch menus once it's running. Wants "launch and forget" simplicity.

**Top Settings She'd Look For:**

| # | Setting | Tab | Control | Default | Range |
|---|---------|-----|---------|---------|-------|
| 1 | **Preset auto-advance timer** | Basic | Slider | 30s | 10s–120s |
| 2 | **Shuffle on/off** | Basic | Toggle | On | — |
| 3 | **Fullscreen** | Basic | Toggle | Off | — |
| 4 | **Volume / gain** | Basic | Slider | 100% | 0–300% |
| 5 | **"Chill Mode" preset filter** | Basic | Toggle | Off | — |

**Luna's voice:** *"I don't want sliders. I want a 'chill' button and a fullscreen button. That's it. If I have to scroll, you've already lost me."*

---

### 🎧 DJ Hex — VJ / Live Performer

**Profile:** Uses Vibeus projected behind DJ booth or on Twitch stream. Needs frame-perfect timing, zero dropped frames, manual preset control. Will eventually want MIDI.

**Top Settings They'd Look For:**

| # | Setting | Tab | Control | Default | Range |
|---|---------|-----|---------|---------|-------|
| 1 | **Transition duration** | Basic | Slider | 3.0s | 0.0s–10.0s |
| 2 | **Hard cut sensitivity** | Advanced | Slider | 1.0 | 0.0–5.0 |
| 3 | **FPS cap / VSync** | Advanced | Dropdown + Toggle | VSync On / 60 | 30/60/120/144/165/240/Uncapped |
| 4 | **Beat sensitivity** | Basic | Slider | 1.0 | 0.0–5.0 |
| 5 | **Transition style** | Advanced | Dropdown | Soft Cut | Soft Cut / Hard Cut / Freeze-Fade / Instant |

**DJ Hex's voice:** *"I need the transition to be controlled by ME, not some random timer. Let me set it to 0s for instant cuts on beat drops. And for the love of god, don't drop frames — give me a performance panel with FPS counter."*

---

### ⚡ Sparks — ADHD Hyperfocus Tinkerer

**Profile:** Will map out every shader parameter. Builds custom preset packs. Wants to override per-preset settings. Reads this design doc for fun.

**Top Settings They'd Look For:**

| # | Setting | Tab | Control | Default | Range |
|---|---------|-----|---------|---------|-------|
| 1 | **FFT size** | Advanced | Dropdown | 1024 | 256/512/1024/2048/4096/8192 |
| 2 | **Resolution scale** | Advanced | Slider | 100% | 25%–200% |
| 3 | **Color temperature** | Advanced | Slider | 6500K | 2700K–10000K |
| 4 | **Per-preset overrides** | Advanced | Sub-panel | — | Speed, Gain, Beat Sens per preset |
| 5 | **Audio smoothing** | Advanced | Slider | 0.7 | 0.0–0.99 |

**Sparks' voice:** *"Wait, I can change the FFT window size? And color temperature? AND per-preset speed overrides? I'm never closing this settings panel. *chef's kiss*"*

---

### 🎮 Cory — Steam Deck Couch User

**Profile:** Docked to TV via Steam Deck dock, or handheld on couch. Everything via controller. Needs large touch targets, readable text, battery awareness.

**Top Settings They'd Look For:**

| # | Setting | Tab | Control | Default | Range |
|---|---------|-----|---------|---------|-------|
| 1 | **UI scale** | Basic | Slider | 100% | 75%–200% |
| 2 | **Performance mode / Quality mode** | Basic | Dropdown | Quality | Battery Saver / Balanced / Quality |
| 3 | **Auto-advance on/off** | Basic | Toggle | On | — |
| 4 | **Gamepad deadzone** | Advanced | Slider | 24% | 5%–50% |
| 5 | **Controller vibration** | Advanced | Toggle | Off | — |

**Cory's voice:** *"Battery Saver mode? Yes please. Also the text is way too small on my Deck. And let me adjust the stick deadzone — my left stick drifts."*

---

### ♿ Sage — Accessibility-Focused User

**Profile:** Photosensitive. May have epilepsy. Needs hard limits on flashing, brightness, and speed. Also wants font scaling and high-contrast option.

**Top Settings They'd Look For:**

| # | Setting | Tab | Control | Default | Range |
|---|---------|-----|---------|---------|-------|
| 1 | **Flash/strobe limiter** | Basic | Toggle | Off | — |
| 2 | **Max brightness cap** | Advanced | Slider | 100% | 10%–100% |
| 3 | **Reduced motion** | Basic | Toggle | Off | — |
| 4 | **Font scale** | Basic | Slider | 100% | 75%–200% |
| 5 | **High-contrast UI** | Advanced | Toggle | Off | — |

**Sage's voice:** *"The epilepsy warning at startup was great. Now let me actually prevent the flashing. And please, PLEASE, let me scale up the font — I shouldn't need a magnifying glass to read tooltips."*

---

## Part 2: Master Settings — BASIC TAB

> Clean, friendly, ~12 settings. Anyone can understand at a glance.
> Layout: Single scrollable column. Large labels. Grouped by affordance.

---

### 🔊 Audio

| Setting | Description (Tooltip) | Control | Range | Default | Primary Persona |
|---------|----------------------|---------|-------|---------|----------------|
| **Audio Gain** | How strongly visuals react to your music. Higher = more reactive. | Slider | 0%–300% | 100% | Luna, DJ Hex |
| **Beat Sensitivity** | How sensitive visuals are to bass hits and beats. | Slider | 0.0–5.0 | 1.0 | DJ Hex, Luna |

### 🎬 Presets

| Setting | Description (Tooltip) | Control | Range | Default | Primary Persona |
|---------|----------------------|---------|-------|---------|----------------|
| **Auto-Advance** | Automatically switch to a new preset after a timer. | Toggle | On/Off | On | Luna, Cory |
| **Preset Duration** | Seconds before auto-advancing to the next preset. | Slider (shown when Auto-Advance=On) | 10s–120s | 30s | Luna |
| **Shuffle** | Randomize preset order instead of playing sequentially. | Toggle | On/Off | On | Luna, Cory |
| **Transition Time** | Duration of the visual crossfade between presets. | Slider | 0.0s–10.0s | 3.0s | DJ Hex |

### 🖥️ Display

| Setting | Description (Tooltip) | Control | Range | Default | Primary Persona |
|---------|----------------------|---------|-------|---------|----------------|
| **Fullscreen** | Toggle between windowed and borderless fullscreen. | Toggle | On/Off | Off | Everyone |
| **UI Scale** | Scale all menus and text. Useful for TV or handheld displays. | Slider | 75%–200% | 100% | Cory, Sage |
| **Performance Mode** | Balance between visual quality and battery/GPU usage. | Dropdown | Battery Saver / Balanced / Quality | Quality | Cory |

### ♿ Safety

| Setting | Description (Tooltip) | Control | Range | Default | Primary Persona |
|---------|----------------------|---------|-------|---------|----------------|
| **Flash Limiter** | Reduce rapid brightness changes that could trigger photosensitive reactions. Clamps frame-to-frame luminance delta. | Toggle | On/Off | Off | Sage |
| **Reduced Motion** | Caps animation speed and disables hard cuts. Makes visuals gentler. | Toggle | On/Off | Off | Sage |
| **Font Scale** | Make all UI text larger or smaller. | Slider | 75%–200% | 100% | Sage, Cory |

**Basic Tab Total: 12 settings** — three per group, clean four-section layout.

---

## Part 3: Master Settings — ADVANCED TAB

> The tinkerer's playground. Collapsible sections. Each section closed by default.
> Every setting has a tooltip via `(?)` icon. Hold gamepad `Y` for tooltip on console.

---

### 🎨 Section 1: Visual Tuning

| Setting | Description (Tooltip) | Control | Range | Default | Primary Persona |
|---------|----------------------|---------|-------|---------|----------------|
| **Brightness** | Global brightness multiplier applied to final render output. | Slider | 0%–200% | 100% | Sage, Sparks |
| **Contrast** | Increase or decrease difference between light and dark areas. | Slider | 50%–200% | 100% | Sparks |
| **Saturation** | Color intensity. 0% = grayscale, 200% = vivid. | Slider | 0%–200% | 100% | Sparks, Sage |
| **Gamma** | Non-linear brightness curve. Lower = darker shadows, higher = lifted darks. | Slider | 0.5–2.5 | 1.0 | Sparks |
| **Color Temperature** | Shift the color spectrum. Low = warm amber, High = cool blue. | Slider (with color preview strip) | 2700K–10000K | 6500K | Sparks, Luna |
| **Max Brightness Cap** | Hard limit on pixel brightness. Prevents any preset from blinding you. | Slider | 10%–100% | 100% | Sage |
| **Hue Rotation** | Rotate the entire color palette by degrees. | Slider | 0°–360° | 0° | Sparks |
| **Vignette** | Darken edges of the screen. Creates a focus tunnel effect. | Slider | 0%–100% | 0% | Luna, Sparks |
| **Film Grain** | Subtle noise overlay for an analog/retro aesthetic. | Slider | 0%–50% | 0% | Sparks |

### 🌊 Section 2: Motion & Animation

| Setting | Description (Tooltip) | Control | Range | Default | Primary Persona |
|---------|----------------------|---------|-------|---------|----------------|
| **Animation Speed** | Global speed multiplier for all preset animations. | Slider | 0.05x–5.0x | 1.0x | DJ Hex, Sparks |
| **Transition Style** | How presets blend into each other. | Dropdown | Soft Cut (crossfade) / Hard Cut (instant) / Freeze-Fade (pause→melt) | Soft Cut | DJ Hex |
| **Hard Cut Sensitivity** | How aggressively bass hits trigger instant preset changes. 0 = never. | Slider | 0.0–5.0 | 1.0 | DJ Hex |
| **Hard Cut Cooldown** | Minimum seconds between bass-triggered hard cuts. Prevents rapid-fire switches. | Slider | 1s–60s | 15s | DJ Hex, Sage |
| **Motion Smoothing** | How much animation frames are interpolated for smoother motion. Higher = more buttery but laggier feel. | Slider | 0% (none)–100% (max) | 50% | Sparks |
| **Max Animation Speed** | Hard cap on speed when Flash Limiter is on. Overrides global speed if exceeded. | Slider | 0.1x–2.0x | 1.5x | Sage |

### 🔊 Section 3: Audio Processing

| Setting | Description (Tooltip) | Control | Range | Default | Primary Persona |
|---------|----------------------|---------|-------|---------|----------------|
| **FFT Size** | Frequency analysis resolution. Higher = more frequency detail, more CPU. Lower = snappier beat response. | Dropdown | 256 / 512 / 1024 / 2048 / 4096 / 8192 | 1024 | Sparks |
| **Audio Smoothing** | Temporal smoothing of the audio spectrum. Higher = less jittery, more mellow. | Slider | 0.0–0.99 | 0.70 | Sparks, Luna |
| **Noise Gate** | Audio level below which input is treated as silence. Prevents idle-noise jitter. | Slider | -80dB to -20dB | -60dB | DJ Hex, Sparks |
| **Bass Boost** | Extra amplification of low frequencies (20-250Hz) before feeding visuals. | Slider | 0.0x–3.0x | 1.0x | DJ Hex |
| **Treble Boost** | Extra amplification of high frequencies (4kHz-20kHz) before feeding visuals. | Slider | 0.0x–3.0x | 1.0x | Sparks |
| **Beat Detection Algorithm** | Method used to detect musical beats. | Dropdown | Energy (simple) / Spectral Flux (accurate) / Onset (percussive) | Energy | Sparks |
| **Audio Latency Offset** | Manually compensate for audio-visual sync delay (ms). Negative = visuals earlier. | Slider | -100ms to +100ms | 0ms | DJ Hex |

### ⚡ Section 4: Performance

| Setting | Description (Tooltip) | Control | Range | Default | Primary Persona |
|---------|----------------------|---------|-------|---------|----------------|
| **VSync** | Synchronize frame rate to monitor refresh rate. Prevents tearing. | Toggle | On/Off | On | DJ Hex |
| **FPS Cap** | Maximum frames per second. Only applies when VSync is off. | Dropdown | 30 / 60 / 120 / 144 / 165 / 240 / Uncapped | 60 | DJ Hex, Cory |
| **Resolution Scale** | Render at lower/higher resolution than display, then scale. Lower = faster. | Slider | 25%–200% | 100% | Cory, Sparks |
| **GPU Power Target** | Limit GPU utilization for heat/battery. Auto = let GPU decide. | Dropdown | Low (30%) / Medium (50%) / High (75%) / Auto (100%) | Auto | Cory |
| **Show FPS Counter** | Display real-time frame rate in corner. | Toggle | On/Off | Off | DJ Hex, Sparks |
| **Debug Overlay** | Show audio levels, preset info, frame timings. | Toggle | On/Off | Off | Sparks |
| **Texture Quality** | Quality of preset textures. Lower = less VRAM usage. | Dropdown | Low / Medium / High | High | Cory |
| **Mesh Resolution** | Complexity of the Milkdrop waveform mesh grid. Higher = smoother but heavier. | Dropdown | Low (32×24) / Medium (64×48) / High (96×72) / Ultra (128×96) | Medium | Sparks |

### ♿ Section 5: Accessibility

| Setting | Description (Tooltip) | Control | Range | Default | Primary Persona |
|---------|----------------------|---------|-------|---------|----------------|
| **Flash Intensity Limit** | Maximum allowed luminance change per frame. Lower = softer transitions. | Slider | 1% (extremely safe)–100% (no limit) | 100% | Sage |
| **Strobe Frequency Cap** | Block presets from flashing faster than N Hz. W3C guideline: <3Hz. | Slider | 1Hz–30Hz | 3Hz (when Flash Limiter is on) | Sage |
| **Color Filter** | Limit or remap color palette for color vision deficiency. | Dropdown | None / Protanopia / Deuteranopia / Tritanopia / Monochrome | None | Sage |
| **High-Contrast UI** | High-contrast theme for menus and overlays. White text on black backgrounds. | Toggle | On/Off | Off | Sage |
| **Epilepsy-Safe Preset Filter** | Only play presets that have been tested and flagged as photosensitive-safe. | Toggle | On/Off | Off | Sage |
| **Reduced Motion (Advanced)** | Maximum animation speed cap + disable waveform oscillation. | Slider | 0.1x–1.0x | 0.5x (when Reduced Motion is on) | Sage |
| **UI Animation Speed** | Speed of menu transitions, button hover effects. 0% = instant (no animations). | Slider | 0%–100% | 100% | Sage |

### 🖥️ Section 6: Display

| Setting | Description (Tooltip) | Control | Range | Default | Primary Persona |
|---------|----------------------|---------|-------|---------|----------------|
| **Window Mode** | Full display control. | Dropdown | Windowed / Borderless Fullscreen / Exclusive Fullscreen | Borderless Fullscreen | Everyone |
| **Monitor** | Choose which display to run on (multi-monitor setups). | Dropdown | (auto-detect list) | Primary | DJ Hex |
| **Aspect Ratio** | Force specific aspect ratio, or match monitor. | Dropdown | Auto / 16:9 / 21:9 / 4:3 / 1:1 | Auto | DJ Hex |
| **UI Opacity** | Transparency of menu overlays when visible. | Slider | 20%–100% | 92% | Sparks, Luna |
| **HUD Position** | Where speed/gain/preset info displays. | Dropdown | Top-Left / Top-Right / Bottom-Left / Bottom-Right / Hidden | Top-Left | DJ Hex |
| **HUD Auto-Hide** | Hide the HUD after N seconds of no input. 0 = always visible. | Slider | 0s–30s | 5s | Luna |
| **Background During Menus** | Keep visuals running behind menus or show static/black. | Dropdown | Live Visuals / Frozen Frame / Black | Live Visuals | Luna, Sparks |

### 🎮 Section 7: Input & Controls

| Setting | Description (Tooltip) | Control | Range | Default | Primary Persona |
|---------|----------------------|---------|-------|---------|----------------|
| **Gamepad Deadzone** | Ignore analog stick input below this threshold. Fix for stick drift. | Slider | 5%–50% | 24% | Cory |
| **Gamepad Vibration** | Haptic feedback on beat hits and preset changes. | Toggle + Intensity Slider | Off / Low / Medium / High | Off | Cory |
| **Mouse Sensitivity (Flow Mode)** | How aggressively mouse position affects speed in Flow Mode. | Slider | 0.1x–3.0x | 1.0x | DJ Hex |
| **Flow Mode Speed Range** | Min/max speed that Flow Mode maps to horizontal mouse position. | Dual Slider | 0.05x–5.0x | 0.2x–3.0x | DJ Hex, Sparks |
| **Scroll Wheel Action** | What the scroll wheel does during visualization. | Dropdown | Nothing / Speed / Gain / Preset Next/Prev / Zoom | Speed | Sparks |
| **Double-Click Action** | What double-clicking does during visualization. | Dropdown | Nothing / Toggle Fullscreen / Pause / Next Preset | Toggle Fullscreen | Luna |
| **Touch Waveform Persistence** | How long touch waveforms linger before fading. | Slider | 0.5s–30s | 5s | Sparks |
| **Touch Waveform Default Type** | Default waveform shape when clicking/touching the screen. | Dropdown | Random / Circle / Radial Blob / Derivative Line / Double Line / (all 9 types) | Random | Sparks |

### 💾 Section 8: Data & Profiles

| Setting | Description (Tooltip) | Control | Range | Default | Primary Persona |
|---------|----------------------|---------|-------|---------|----------------|
| **Settings Profile** | Save/load named settings configurations. | Dropdown + Save/Load/Delete buttons | (user-created list) | Default | DJ Hex, Sparks |
| **Per-Preset Overrides** | Enable per-preset speed, gain, and beat sensitivity. Stored with favorites. | Toggle | On/Off | Off | Sparks |
| **Export Settings** | Export current settings to a JSON file for sharing or backup. | Button | — | — | Sparks |
| **Import Settings** | Load a settings JSON file. | Button | — | — | Sparks |
| **Reset to Defaults** | Reset ALL settings to factory defaults. | Button (with confirmation) | — | — | Everyone |

---

## Part 4: Unique Selling Point Settings

> Things no other music visualizer offers. Stuff that makes people say *"holy shit, this app thinks of everything."*

---

### 🏆 USP #1: Mood Presets (One-Click Vibes)

**Setting:** `Mood` — a single dropdown at the top of Basic tab.

| Value | What It Does |
|-------|-------------|
| **Chill** | Speed 0.7x, gain 80%, beat sens 0.5, soft cuts only, warm color temp (4000K), transition 5s |
| **Party** | Speed 1.5x, gain 150%, beat sens 2.0, hard cuts enabled, cool temp (7500K), transition 1.5s |
| **Focus** | Speed 0.4x, gain 60%, beat sens 0.3, no hard cuts, neutral temp, transition 8s, reduced motion |
| **Psychedelic** | Speed 1.2x, gain 120%, beat sens 1.5, medium hard cuts, hue rotation +45°, transition 3s |
| **Custom** | Manual (shows rest of settings as normal) |

**Why it's killer:** No other visualizer offers one-word mood selection that tunes 8+ parameters at once. Luna picks "Chill" and never touches another setting. DJ Hex picks "Party" as a starting point and tweaks from there.

---

### 🏆 USP #2: Per-Preset Overrides

**Concept:** When favoriting a preset, optionally save custom speed / gain / beat sensitivity / color temperature values that auto-apply when that preset plays.

**Why it's killer:** Sparks can make slow presets fast and loud presets quiet. DJ Hex can build a "set" where each preset is tuned for their specific mix. No existing visualizer supports this.

---

### 🏆 USP #3: Vibe Lock

**Setting:** `Vibe Lock` — toggle in Basic tab.

When enabled, disables all keyboard/mouse/gamepad input EXCEPT Escape (to unlock). The app becomes a "digital lava lamp" — no accidental input disrupts the experience.

**Why it's killer:** Luna puts it on before leaving the room. Cory activates it so little kids can watch without accidentally exiting. Party hosts lock it behind the DJ booth.

---

### 🏆 USP #4: Audio-Reactive Accessibility

Instead of just limiting flashes, the **Flash Limiter analyzes each rendered frame** and dynamically clamps brightness delta. This means:
- Presets that are naturally smooth play at full fidelity
- Only dangerous frames get tone-mapped down
- The experience degrades gracefully, not uniformly

No competitor does frame-level photosensitivity filtering. W3C guidelines say <3 flashes/sec — Vibeus actually enforces it per-pixel.

---

### 🏆 USP #5: Gamepad Haptic Beat Sync

**Setting:** `Gamepad Vibration` in Advanced → Input.

Maps bass drum hits to controller rumble. Intensity scales with beat energy. For Steam Deck and DualSense users, this makes you *feel* the music through the controller.

**Why it's killer:** This is a "didn't know I wanted this" feature. Synesthesia through touch. Nobody does this.

---

### 🏆 USP #6: Session Analytics (Debug Overlay)

The Debug Overlay shows:
- Current FPS / frame time graph (last 120 frames)
- Audio waveform + spectrum mini-display
- GPU memory usage
- Current preset name + time remaining
- Smoothed BPM estimate
- Audio latency measurement

**Why it's killer:** DJ Hex uses this to verify sync. Sparks uses this to diagnose why a preset looks different at different speeds. Nobody else surfaces this data.

---

### 🏆 USP #7: Scroll Wheel as Universal Modifier

The scroll wheel becomes a context-aware universal dial:
- Default: speed control (smooth, proportional)
- Hold Shift: audio gain
- Hold Ctrl: beat sensitivity
- Hold Alt: transition time

**Why it's killer:** Mouse users get instant parameter tuning without ever opening a menu. Zero-friction workflow for DJ Hex's live sets.

---

## Part 5: UI/UX Notes

### Basic Tab Layout
```
┌────────────────────────────────────────┐
│  ⚙ SETTINGS                    [Basic][Advanced] │
├────────────────────────────────────────┤
│                                        │
│  🎭 Mood:  [Chill ▾]                  │
│                                        │
│  ── Audio ──────────────────────────── │
│  Audio Gain        [====●========] 100%│
│  Beat Sensitivity  [==●==========] 1.0 │
│                                        │
│  ── Presets ────────────────────────── │
│  Auto-Advance      [●]  ON            │
│  Preset Duration   [======●====]  30s │
│  Shuffle           [●]  ON            │
│  Transition Time   [===●=======] 3.0s │
│                                        │
│  ── Display ────────────────────────── │
│  Fullscreen        [●]  ON            │
│  UI Scale          [====●======] 100% │
│  Performance       [Quality ▾]         │
│                                        │
│  ── Safety ─────────────────────────── │
│  Flash Limiter     [ ]  OFF            │
│  Reduced Motion    [ ]  OFF            │
│  Font Scale        [====●======] 100% │
│                                        │
│  [🔒 Vibe Lock]                        │
│                                        │
│           [Reset to Defaults]          │
└────────────────────────────────────────┘
```

### Advanced Tab Layout
```
┌────────────────────────────────────────┐
│  ⚙ SETTINGS                    [Basic][Advanced] │
├────────────────────────────────────────┤
│                                        │
│  ▸ Visual Tuning (9 settings)          │
│  ▸ Motion & Animation (6 settings)     │
│  ▸ Audio Processing (7 settings)       │
│  ▸ Performance (8 settings)            │
│  ▸ Accessibility (7 settings)          │
│  ▸ Display (7 settings)                │
│  ▸ Input & Controls (8 settings)       │
│  ▸ Data & Profiles (5 settings)        │
│                                        │
│  Click to expand a section.            │
│  Hold (Y) for tooltip on any setting.  │
│                                        │
│  ── Settings Profile ──────────────── │
│  [Default ▾] [Save] [Load] [Delete]   │
│                                        │
│           [Reset to Defaults]          │
└────────────────────────────────────────┘
```

### Interaction Rules

1. **Gamepad Navigation:** D-pad navigates settings. A = interact with control. B = back/close. Y (hold) = show tooltip. Start = close settings.
2. **Sliders:** Left/right on D-pad or left stick. Hold for accelerated seek. Click/tap value text to type exact number.
3. **Tooltips:** On hover (mouse), on Y-hold (gamepad), or on long-press (touch). Displayed as floating card below the setting.
4. **Persistence:** All settings auto-save to `vibeus_config.json` on change. No explicit "Save" button needed for general settings.
5. **Undo:** Ctrl+Z in settings panel reverts last change. "Reset to Defaults" requires confirmation dialog.
6. **Search (Advanced tab):** Type-to-filter search box at top of Advanced tab. Filters across all sections.

### Gamepad-Friendly Design Targets

- Minimum touch target: 48px height
- Minimum font size: 16px at 100% scale
- Focus indicator: 2px glowing ring in accent color
- Settings entries: one per row, no horizontal grouping of controls
- Scrolling: smooth, with analog stick proportional speed

---

## Part 6: Implementation Priority

### Phase 1 — Ship with v0.3.0 (Must-Have)

All 12 Basic tab settings + config save/load:
- Audio Gain, Beat Sensitivity
- Auto-Advance, Preset Duration, Shuffle, Transition Time
- Fullscreen, UI Scale, Performance Mode
- Flash Limiter, Reduced Motion, Font Scale
- Mood presets (Chill / Party / Focus / Psychedelic / Custom)
- Vibe Lock

### Phase 2 — Ship with v0.4.0 (High Value)

Priority Advanced sections:
- Performance (VSync, FPS cap, resolution scale, FPS counter)
- Motion & Animation (speed, transition style, hard cut controls)
- Input & Controls (deadzone, flow mode tuning, scroll wheel actions)
- Display (window mode, monitor selection, HUD controls)
- Settings Profiles (save/load/export/import)

### Phase 3 — Ship with v0.5.0 (Differentiation)

Remaining Advanced sections:
- Visual Tuning (brightness, contrast, saturation, gamma, color temp, vignette, film grain)
- Audio Processing (FFT size, smoothing, noise gate, bass/treble boost, beat detection, latency offset)
- Accessibility (full suite: flash intensity, strobe cap, color filters, epilepsy-safe filter)
- Per-Preset Overrides
- Gamepad Haptic Beat Sync
- Session Analytics / Debug Overlay

---

## Settings Count Summary

| Tab | Section | Count |
|-----|---------|-------|
| **Basic** | Audio | 2 |
| **Basic** | Presets | 4 |
| **Basic** | Display | 3 |
| **Basic** | Safety | 3 |
| **Basic** | *+ Mood dropdown + Vibe Lock* | 2 |
| | **Basic Total** | **14** |
| **Advanced** | Visual Tuning | 9 |
| **Advanced** | Motion & Animation | 6 |
| **Advanced** | Audio Processing | 7 |
| **Advanced** | Performance | 8 |
| **Advanced** | Accessibility | 7 |
| **Advanced** | Display | 7 |
| **Advanced** | Input & Controls | 8 |
| **Advanced** | Data & Profiles | 5 |
| | **Advanced Total** | **57** |
| | **Grand Total** | **71** |
