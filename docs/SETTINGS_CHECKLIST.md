# Vibeus Settings — Implementation Priority Checklist

> Task tracker for the settings overhaul implementation
> Reference: SETTINGS_DESIGN.md, SETTINGS_IMPLEMENTATION.md, SETTINGS_ARCHITECTURE.md

---

## Phase 1: Wire Up Existing API (Est. 2-3 hours)

**Goal:** Connect 6 unexposed projectM API functions to new config fields

### config.h Changes
- [ ] Add `int meshQuality = 64;` (8–200)
- [ ] Add `bool aspectCorrection = true;`
- [ ] Add `bool cleanTransitions = false;`
- [ ] Add `float easterEgg = 0.0f;` (0.0–2.0)
- [ ] Add `int targetFps = 60;` (30/60/120/0)
- [ ] Add `bool vsyncEnabled = true;`

### config.cpp Changes
- [ ] Add new fields to `loadConfig()` with `j.value()` defaults
- [ ] Add new fields to `saveConfig()` JSON serialization
- [ ] Create `applyPerfMode(VibeusConfig&, PerfMode)` function

### main.cpp Changes
- [ ] Add `projectm_set_mesh_size(g_pm, cfg.meshQuality, cfg.meshQuality * 3 / 4);`
- [ ] Add `projectm_set_aspect_correction(g_pm, cfg.aspectCorrection);`
- [ ] Add `projectm_set_preset_start_clean(g_pm, cfg.cleanTransitions);`
- [ ] Add `projectm_set_easter_egg(g_pm, cfg.easterEgg);`
- [ ] Add `projectm_set_fps(g_pm, cfg.targetFps == 0 ? 1000 : cfg.targetFps);`
- [ ] Add `SDL_GL_SetSwapInterval(cfg.vsyncEnabled ? 1 : 0);`
- [ ] Add static `g_vsyncState` to track vsync changes

### Testing
- [ ] Verify mesh slider changes warp smoothness visually
- [ ] Verify aspect correction fixes ultrawide distortion
- [ ] Verify easter egg randomizes preset durations
- [ ] Verify clean transitions show black between presets
- [ ] Verify FPS target affects frame rate
- [ ] Verify VSync toggle prevents/allows tearing

---

## Phase 2: UI Controls (Est. 4-6 hours)

**Goal:** Add ImGui controls for new settings

### Display Tab — Visual Quality Section
- [ ] Add "Mesh Detail" SliderInt (8–200)
- [ ] Add "Aspect Correction" Checkbox
- [ ] Add "VSync" Checkbox  
- [ ] Add "Target FPS" Dropdown (30/60/120/Unlimited)
- [ ] Add tooltips for all new controls

### Presets Tab — New Options
- [ ] Add "Duration Variance" SliderFloat (0.0–2.0) for easterEgg
- [ ] Add "Clean Transitions" Checkbox
- [ ] Add tooltips for both

### PerfMode Integration
- [ ] Make Performance dropdown call `applyPerfMode()` when changed
- [ ] Update tooltip to show actual values applied
- [ ] Consider adding "Ultra" quality tier (mesh=128, fps=120)

### Testing
- [ ] All new sliders persist to JSON on exit
- [ ] All new sliders apply in real-time
- [ ] Tooltips display correctly
- [ ] Sliders respect min/max bounds

---

## Phase 3: UI Reorganization (Est. 3-4 hours)

**Goal:** Split into 4 tabs for better organization

### Tab Restructure
- [ ] Create "Vibes" tab with: Mood, Audio Reactivity, Motion
- [ ] Create "Presets" tab with: Playback, Transitions
- [ ] Create "Display" tab with: Performance, Visual Quality, Window, Accessibility
- [ ] Create "Advanced" tab with: Input, Rendering, Developer

### Content Migration
- [ ] Move Animation Speed from Advanced → Vibes (Motion section)
- [ ] Move Flash Limiter & Reduced Motion to Vibes (they affect visuals)
- [ ] Move UI Scale & Font Scale to Display (Accessibility section)
- [ ] Keep Input section in Advanced

### Visual Polish
- [ ] Add section headers with `ImGui::SeparatorText()`
- [ ] Consistent slider widths (65% of content width)
- [ ] Add (?) tooltip icons on same line as all controls
- [ ] Add "Reset to Defaults" at bottom of each tab or globally

---

## Phase 4: Polish & Quality of Life (Est. 2-3 hours)

**Goal:** Enhance user experience

### Improvements
- [ ] Add visual indicator when settings differ from defaults
- [ ] Add "Reset Section" buttons for each section
- [ ] Add search/filter in Advanced tab
- [ ] Show current mesh quality as "Low/Med/High/Ultra" label

### Performance Mode Presets Table
- [ ] Document values in tooltip:
  - Battery Saver: mesh=24, fps=30, vsync=ON
  - Balanced: mesh=48, fps=60, vsync=ON
  - Quality: mesh=64, fps=60, vsync=ON
  - Ultra: mesh=128, fps=120, vsync=OFF

### Mood Presets Update
- [ ] Review mood presets to include new settings
- [ ] Consider if moods should affect mesh quality (probably not)

---

## Future Phases (Post-v0.4.0)

### Requires Post-Processing Shader Work
- [ ] Brightness adjustment
- [ ] Contrast adjustment
- [ ] Saturation adjustment
- [ ] Color temperature
- [ ] Vignette effect
- [ ] Film grain overlay

### Requires Audio Processing Work
- [ ] FFT size selection
- [ ] Audio smoothing
- [ ] Bass/Treble boost
- [ ] Noise gate

### Requires Architecture Changes
- [ ] Per-preset overrides
- [ ] Settings profiles (save/load named configs)
- [ ] Resolution scaling (render at lower res, upscale)

---

## Quick Reference: API Calls

```cpp
// NEW calls to add in applyConfig()
projectm_set_mesh_size(g_pm, cfg.meshQuality, cfg.meshQuality * 3 / 4);
projectm_set_aspect_correction(g_pm, cfg.aspectCorrection);
projectm_set_preset_start_clean(g_pm, cfg.cleanTransitions);
projectm_set_easter_egg(g_pm, cfg.easterEgg);
projectm_set_fps(g_pm, cfg.targetFps == 0 ? 1000 : cfg.targetFps);
SDL_GL_SetSwapInterval(cfg.vsyncEnabled ? 1 : 0);
```

---

## Definition of Done

### Phase 1 Complete When:
- [ ] All 6 new config fields added and serializing
- [ ] All 6 projectM API calls working
- [ ] Settings persist across app restart
- [ ] Visual changes observable when adjusting settings

### Phase 2 Complete When:
- [ ] All new UI controls visible and functional
- [ ] PerfMode dropdown actually changes mesh/fps
- [ ] Tooltips explain each setting clearly
- [ ] No regressions in existing settings

### Phase 3 Complete When:
- [ ] 4-tab layout implemented
- [ ] Settings logically grouped
- [ ] Tab navigation works with keyboard/gamepad
- [ ] Scroll works within each tab

---

## Notes

- Keep backward compatibility: old config files should load with new defaults
- Test on both windowed and fullscreen modes
- Test with gamepad navigation
- Verify settings panel performance (no frame drops while open)
