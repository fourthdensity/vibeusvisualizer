# Vibeus Settings Implementation Guide

> Technical implementation reference for the settings overhaul
> Companion to SETTINGS_DESIGN.md

---

## Current State vs. Target State

### What's Implemented (v0.3.x)

**VibeusConfig fields (23):**
- ✅ audioGain, beatSensitivity
- ✅ autoAdvance, presetDuration, shuffle, transitionTime
- ✅ hardCutEnabled, hardCutSensitivity, hardCutDuration
- ✅ fullscreen, uiScale, showFps, perfMode
- ✅ flashLimiter, reducedMotion, fontScale
- ✅ mood, vibeLock
- ✅ speedMultiplier
- ✅ gamepadDeadzone, flowMode, touchEnabled
- ✅ overlayOpacity

**projectM API calls in use:**
- ✅ `projectm_set_beat_sensitivity()`
- ✅ `projectm_set_preset_duration()`
- ✅ `projectm_set_soft_cut_duration()`
- ✅ `projectm_set_hard_cut_enabled()`
- ✅ `projectm_set_hard_cut_sensitivity()`
- ✅ `projectm_set_hard_cut_duration()`
- ✅ `projectm_set_preset_locked()`
- ✅ `projectm_set_mesh_size()` — hardcoded to 64×48
- ✅ `projectm_set_window_size()` — auto on resize
- ✅ `projectm_set_frame_time()` — speed multiplier applied
- ✅ `projectm_playlist_set_shuffle()`

### What's NOT Connected

**projectM API functions available but not exposed:**

| Function | Purpose | Difficulty |
|----------|---------|------------|
| `projectm_set_aspect_correction(bool)` | Fix ultrawide/portrait distortion | Easy |
| `projectm_set_easter_egg(float)` | Random duration variance | Easy |
| `projectm_set_fps(int32_t)` | Target frame rate | Easy |
| `projectm_set_preset_start_clean(bool)` | Black canvas between presets | Easy |
| `projectm_set_mesh_size(w, h)` | Mesh quality (currently hardcoded) | Easy |
| `projectm_set_texel_offset(x, y)` | Warp shader alignment | Medium |

**PerfMode is defined but not applied:**
```cpp
// In config.h - exists but doesn't do anything yet
enum class PerfMode { BatterySaver, Balanced, Quality };
```

---

## Implementation Roadmap

### Phase 1: Quick Wins (2-3 hours)

**Goal:** Wire up existing API functions to new config fields

#### 1.1 Add New Config Fields

```cpp
// config.h - add to VibeusConfig struct

// ── Visual Quality ──
int   meshQuality        = 64;       // 8–200 (mesh width, height = width * 3/4)
bool  aspectCorrection   = true;     // fix distortion on ultrawide/portrait
bool  cleanTransitions   = false;    // black canvas between presets

// ── Preset Timing ──  
float easterEgg          = 0.0f;     // 0.0–2.0 (random duration variance)

// ── Performance ──
int   targetFps          = 60;       // 30, 60, 120, or 0 (unlimited)
bool  vsyncEnabled       = true;     // vertical sync
```

#### 1.2 Update applyConfig()

```cpp
// main.cpp - applyConfig() additions

static void applyConfig(const VibeusConfig& cfg)
{
    // ... existing code ...
    
    if (g_pm) {
        // NEW: Visual quality
        projectm_set_mesh_size(g_pm, cfg.meshQuality, cfg.meshQuality * 3 / 4);
        projectm_set_aspect_correction(g_pm, cfg.aspectCorrection);
        projectm_set_preset_start_clean(g_pm, cfg.cleanTransitions);
        
        // NEW: Easter egg (random duration variance)
        projectm_set_easter_egg(g_pm, cfg.easterEgg);
        
        // NEW: FPS target (projectM internal)
        projectm_set_fps(g_pm, cfg.targetFps == 0 ? 1000 : cfg.targetFps);
    }
    
    // NEW: VSync
    if (g_vsyncState != cfg.vsyncEnabled) {
        SDL_GL_SetSwapInterval(cfg.vsyncEnabled ? 1 : 0);
        g_vsyncState = cfg.vsyncEnabled;
    }
}
```

#### 1.3 Apply PerfMode Presets

```cpp
// config.cpp - new function

void applyPerfMode(VibeusConfig& cfg, PerfMode mode)
{
    switch (mode) {
        case PerfMode::BatterySaver:
            cfg.meshQuality = 24;
            cfg.targetFps = 30;
            cfg.vsyncEnabled = true;
            break;
        case PerfMode::Balanced:
            cfg.meshQuality = 48;
            cfg.targetFps = 60;
            cfg.vsyncEnabled = true;
            break;
        case PerfMode::Quality:
            cfg.meshQuality = 64;
            cfg.targetFps = 60;
            cfg.vsyncEnabled = true;
            break;
    }
}
```

#### 1.4 Update JSON Serialization

```cpp
// config.cpp - loadConfig() additions

cfg.meshQuality = j.value("meshQuality", 64);
cfg.aspectCorrection = j.value("aspectCorrection", true);
cfg.cleanTransitions = j.value("cleanTransitions", false);
cfg.easterEgg = j.value("easterEgg", 0.0f);
cfg.targetFps = j.value("targetFps", 60);
cfg.vsyncEnabled = j.value("vsyncEnabled", true);

// config.cpp - saveConfig() additions

j["meshQuality"] = cfg.meshQuality;
j["aspectCorrection"] = cfg.aspectCorrection;
j["cleanTransitions"] = cfg.cleanTransitions;
j["easterEgg"] = cfg.easterEgg;
j["targetFps"] = cfg.targetFps;
j["vsyncEnabled"] = cfg.vsyncEnabled;
```

---

### Phase 2: UI Additions (4-6 hours)

**Goal:** Expose new settings in menu_overlay.cpp

#### 2.1 Display Tab - Visual Quality Section

```cpp
// menu_overlay.cpp - in renderSettings(), Display section

ImGui::SeparatorText("Visual Quality");
{
    // Mesh Detail slider (quality vs performance)
    ImGui::SetNextItemWidth(contentW * 0.65f);
    if (ImGui::SliderInt("Mesh Detail", &m_config->meshQuality, 8, 200)) {
        changed = true;
    }
    ImGui::SameLine(); ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Resolution of the warp effect grid.\n"
                          "Higher = smoother warps, more GPU.\n"
                          "8 = fast, 64 = balanced, 200 = smooth.");

    // Aspect Correction checkbox
    if (ImGui::Checkbox("Aspect Correction", &m_config->aspectCorrection))
        changed = true;
    ImGui::SameLine(); ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Fix visual stretching on ultrawide\n"
                          "or portrait displays.");

    // VSync checkbox
    if (ImGui::Checkbox("VSync", &m_config->vsyncEnabled))
        changed = true;
    ImGui::SameLine(); ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Synchronize with monitor refresh rate.\n"
                          "Prevents tearing but may add input lag.");

    // FPS Target dropdown
    const char* fpsOptions[] = { "30", "60", "120", "Unlimited" };
    int fpsIdx = (m_config->targetFps == 30) ? 0 :
                 (m_config->targetFps == 60) ? 1 :
                 (m_config->targetFps == 120) ? 2 : 3;
    ImGui::SetNextItemWidth(contentW * 0.4f);
    if (ImGui::Combo("Target FPS", &fpsIdx, fpsOptions, 4)) {
        m_config->targetFps = (fpsIdx == 0) ? 30 :
                              (fpsIdx == 1) ? 60 :
                              (fpsIdx == 2) ? 120 : 0;
        changed = true;
    }
    ImGui::SameLine(); ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Maximum frames per second.\n"
                          "Lower = better battery life.\n"
                          "Unlimited = as fast as GPU allows.");
}
```

#### 2.2 Presets Tab - New Options

```cpp
// menu_overlay.cpp - in Presets section

// Easter Egg slider
ImGui::SetNextItemWidth(contentW * 0.65f);
if (ImGui::SliderFloat("Duration Variance", &m_config->easterEgg, 0.0f, 2.0f, "%.1f")) {
    changed = true;
}
ImGui::SameLine(); ImGui::TextDisabled("(?)");
if (ImGui::IsItemHovered())
    ImGui::SetTooltip("Add random variance to preset duration.\n"
                      "0 = exact duration, 2 = very random.\n"
                      "Creates unpredictable timing.");

// Clean Transitions checkbox
if (ImGui::Checkbox("Clean Transitions", &m_config->cleanTransitions))
    changed = true;
ImGui::SameLine(); ImGui::TextDisabled("(?)");
if (ImGui::IsItemHovered())
    ImGui::SetTooltip("Start each new preset from a black canvas\n"
                      "instead of blending with the previous frame.");
```

---

### Phase 3: PerfMode Integration (2 hours)

**Goal:** Make Performance Mode dropdown actually apply settings

```cpp
// menu_overlay.cpp - update PerfMode combo

const char* perfModes[] = { "Battery Saver", "Balanced", "Quality" };
int pmIdx = static_cast<int>(m_config->perfMode);
ImGui::SetNextItemWidth(contentW * 0.5f);
if (ImGui::Combo("Performance", &pmIdx, perfModes, 3)) {
    m_config->perfMode = static_cast<PerfMode>(pmIdx);
    applyPerfMode(*m_config, m_config->perfMode);  // NEW: actually apply!
    changed = true;
}
ImGui::SameLine(); ImGui::TextDisabled("(?)");
if (ImGui::IsItemHovered())
    ImGui::SetTooltip("Battery Saver: 30 FPS, low mesh (24).\n"
                      "Balanced: 60 FPS, medium mesh (48).\n"
                      "Quality: 60 FPS, high mesh (64).");
```

---

## projectM API Reference

### Complete Function List (from projectM-4/parameters.h)

```cpp
// ═══════════════════════════════════════════════════════════════════
// BEAT & TIMING
// ═══════════════════════════════════════════════════════════════════
void projectm_set_beat_sensitivity(projectm_handle, float);       // 0.0-5.0
float projectm_get_beat_sensitivity(projectm_handle);

void projectm_set_hard_cut_duration(projectm_handle, double);     // seconds before hard cut
double projectm_get_hard_cut_duration(projectm_handle);

void projectm_set_hard_cut_enabled(projectm_handle, bool);
bool projectm_get_hard_cut_enabled(projectm_handle);

void projectm_set_hard_cut_sensitivity(projectm_handle, float);   // 0.5-4.0
float projectm_get_hard_cut_sensitivity(projectm_handle);

void projectm_set_soft_cut_duration(projectm_handle, double);     // crossfade seconds
double projectm_get_soft_cut_duration(projectm_handle);

void projectm_set_preset_duration(projectm_handle, double);       // seconds per preset
double projectm_get_preset_duration(projectm_handle);

void projectm_set_fps(projectm_handle, int32_t);                  // target FPS
int32_t projectm_get_fps(projectm_handle);

void projectm_set_frame_time(projectm_handle, double);            // manual time control
double projectm_get_last_frame_time(projectm_handle);

// ═══════════════════════════════════════════════════════════════════
// VISUAL MESH
// ═══════════════════════════════════════════════════════════════════
void projectm_set_mesh_size(projectm_handle, size_t width, size_t height);
void projectm_get_mesh_size(projectm_handle, size_t* width, size_t* height);
// Range: 8-300 per dimension, default 32x24
// Higher = smoother warps, more GPU

void projectm_set_aspect_correction(projectm_handle, bool);       // fix ultrawide
bool projectm_get_aspect_correction(projectm_handle);

void projectm_set_texel_offset(projectm_handle, float x, float y);
void projectm_get_texel_offset(projectm_handle, float* x, float* y);
// Default: 0.5, 0.5 (MilkDrop standard)

// ═══════════════════════════════════════════════════════════════════
// DISPLAY
// ═══════════════════════════════════════════════════════════════════
void projectm_set_window_size(projectm_handle, size_t width, size_t height);
void projectm_get_window_size(projectm_handle, size_t* width, size_t* height);

void projectm_set_preset_start_clean(projectm_handle, bool);      // black canvas
bool projectm_get_preset_start_clean(projectm_handle);

// ═══════════════════════════════════════════════════════════════════
// PRESET CONTROL
// ═══════════════════════════════════════════════════════════════════
void projectm_set_preset_locked(projectm_handle, bool);
bool projectm_get_preset_locked(projectm_handle);

void projectm_set_easter_egg(projectm_handle, float);             // random duration sigma
float projectm_get_easter_egg(projectm_handle);
// 0.0 = off, 0.5 = subtle, 2.0 = chaotic
```

---

## Settings → API Mapping Table

| Config Field | projectM Function | SDL Function | Notes |
|--------------|-------------------|--------------|-------|
| `beatSensitivity` | `projectm_set_beat_sensitivity()` | — | 0.0–5.0 |
| `audioGain` | — (manual PCM scaling) | — | Applied in feedAudio() |
| `presetDuration` | `projectm_set_preset_duration()` | — | 10–120 sec |
| `transitionTime` | `projectm_set_soft_cut_duration()` | — | 0.0–10.0 sec |
| `hardCutEnabled` | `projectm_set_hard_cut_enabled()` | — | — |
| `hardCutSensitivity` | `projectm_set_hard_cut_sensitivity()` | — | 0.5–4.0 |
| `hardCutDuration` | `projectm_set_hard_cut_duration()` | — | 5–60 sec |
| `vibeLock` | `projectm_set_preset_locked()` | — | — |
| `shuffle` | `projectm_playlist_set_shuffle()` | — | — |
| `speedMultiplier` | `projectm_set_frame_time()` | — | multiplier |
| **`meshQuality`** | `projectm_set_mesh_size()` | — | **NEW** 8–200 |
| **`aspectCorrection`** | `projectm_set_aspect_correction()` | — | **NEW** |
| **`cleanTransitions`** | `projectm_set_preset_start_clean()` | — | **NEW** |
| **`easterEgg`** | `projectm_set_easter_egg()` | — | **NEW** 0.0–2.0 |
| **`targetFps`** | `projectm_set_fps()` | — | **NEW** 30/60/120/0 |
| **`vsyncEnabled`** | — | `SDL_GL_SetSwapInterval()` | **NEW** |
| `fullscreen` | — | `SDL_SetWindowFullscreen()` | — |

---

## Testing Checklist

### Phase 1 Tests

- [ ] **Mesh Quality:** Slide from 8 to 200, observe warp smoothness
- [ ] **Aspect Correction:** Toggle on ultrawide monitor, verify fix
- [ ] **Clean Transitions:** Enable, advance preset, see black flash
- [ ] **Easter Egg:** Set to 2.0, presets switch at random times
- [ ] **Target FPS:** Set to 30, verify frame rate drops
- [ ] **VSync:** Disable, verify tearing (if monitor allows)

### Phase 2 Tests

- [ ] **Settings persist:** Change mesh, close app, reopen, verify saved
- [ ] **PerfMode applies:** Select "Battery Saver", verify mesh=24, fps=30
- [ ] **Tooltips accurate:** Hover each new setting, verify description
- [ ] **Sliders clamp:** Can't set mesh below 8 or above 200

### Phase 3 Tests

- [ ] **Mood presets:** Select "Chill", verify mesh doesn't change (only audio/speed)
- [ ] **Reset to Defaults:** Click button, verify new fields reset
- [ ] **Migration:** Old config file without new fields loads with defaults

---

## File Changes Summary

| File | Changes |
|------|---------|
| `config.h` | Add 6 new fields to VibeusConfig |
| `config.cpp` | Update loadConfig(), saveConfig(), add applyPerfMode() |
| `main.cpp` | Update applyConfig() with new API calls |
| `menu_overlay.cpp` | Add UI controls for new settings |

---

## Not Implementing (Out of Scope)

These features from SETTINGS_DESIGN.md require projectM changes or are post-v0.4.0:

| Feature | Reason |
|---------|--------|
| FFT Size | projectM doesn't expose FFT window size |
| Audio Smoothing | No projectM API |
| Bass/Treble Boost | Requires audio filter implementation |
| Color Temperature | Requires post-processing shader |
| Brightness/Contrast | Requires post-processing shader |
| Per-Preset Overrides | Architecture change needed |
| Resolution Scaling | SDL/OpenGL framebuffer work |
| Beat Detection Algorithm | projectM internal |

---

## Quick Reference: New Config Defaults

```cpp
struct VibeusConfig {
    // ... existing fields ...
    
    // NEW: Visual Quality
    int   meshQuality        = 64;       // 8–200
    bool  aspectCorrection   = true;
    bool  cleanTransitions   = false;
    
    // NEW: Timing
    float easterEgg          = 0.0f;     // 0.0–2.0
    
    // NEW: Performance
    int   targetFps          = 60;       // 30, 60, 120, 0
    bool  vsyncEnabled       = true;
};
```
