# VIBEUS OVERHAUL MASTER PLAN
**Created:** 2026-03-22  
**Analysis Duration:** 10 minutes (5 parallel agents)  
**Models Used:** Claude Haiku 4.5 (analysis) + Claude Opus 4.5 (design)

---

## 🎯 **THREE-PART OVERHAUL**

### **PART 1: REMOVE TOUCH FUNCTIONALITY** ✅ Ready
**Status:** Fully analyzed, safe removal strategy designed  
**Effort:** 30 minutes  
**Impact:** Cleaner dev build, removes "breaks stuff" feature

### **PART 2: FIX & ENHANCE SETTINGS** ✅ Ready  
**Status:** Complete design with code snippets created  
**Effort:** 2-3 hours (Phase 1), 4-6 hours (Phase 2), 3-4 hours (Phase 3)  
**Impact:** Settings actually control visuals, more intuitive

### **PART 3: CATEGORIZE PRESETS** ✅ Ready
**Status:** System designed, code written in `src/preset_database.h/cpp`  
**Effort:** 4-6 hours integration  
**Impact:** Browse 9,797 presets by category/stack instead of flat list

---

## 📋 **PART 1: TOUCH REMOVAL** (Quick Win)

### What Gets Disabled:
- ❌ Mouse click/drag waveform spawning
- ❌ Right-click waveform type cycling
- ❌ Scroll wheel pressure adjustment
- ❌ Gamepad right stick touch control
- ❌ Flow mode (Tab key)
- ❌ Ripple ring effects
- ❌ C key (clear waveforms)

### Implementation Strategy:
**Option A: Safe Disable (Recommended for Dev)**
```cpp
// In main.cpp, change these guard conditions to false:
if (false && g_config.touchEnabled) { /* touch code */ }
if (false && g_flowMode) { /* flow mode code */ }
```

**Locations to modify (6 changes):**
1. Line 430 - Gamepad right stick
2. Line 797 - Left mouse click
3. Line 816 - Mouse drag
4. Line 833 - Scroll wheel
5. Line 455 - Flow mode check
6. Lines 653, 659 - C/Tab key handlers

**Option B: Complete Removal**
- Delete all touch functions
- Remove from menu UI
- Remove config persistence
- ~200 lines deleted

### What STAYS Working:
- ✅ All keyboard controls (speed, audio, presets)
- ✅ Gamepad buttons/triggers (everything except right stick)
- ✅ Menu system
- ✅ All visualizer rendering

---

## 🎨 **PART 2: SETTINGS OVERHAUL**

### **PHASE 1: Fix Broken Settings** (2-3 hours)
**Priority: HIGH** - These settings exist but don't work!

#### 1.1 Fix `gamepadDeadzone` (5 min)
**File:** `main.cpp:408`
```cpp
// BEFORE:
if (abs(raw) < STICK_DEADZONE) return 0.0f;

// AFTER:
if (abs(raw) < g_config.gamepadDeadzone) return 0.0f;
```

#### 1.2 Remove `uiScale` (10 min)
**Decision:** Remove entirely (it never worked)
- Delete from `config.h` line 27
- Delete from `menu_overlay.cpp` line 960
- Delete from `config.cpp` save/load

#### 1.3 Implement `perfMode` (2 hours)
**Files:** `main.cpp:applyConfig()`
```cpp
// Add after line 877:
switch (cfg.perfMode) {
    case PerfMode::BatterySaver:
        projectm_set_mesh_size(g_pm, 32, 24);
        SDL_GL_SetSwapInterval(0);  // No VSync
        break;
    case PerfMode::Balanced:
        projectm_set_mesh_size(g_pm, 64, 48);
        SDL_GL_SetSwapInterval(1);  // VSync
        break;
    case PerfMode::Quality:
        projectm_set_mesh_size(g_pm, 128, 96);
        SDL_GL_SetSwapInterval(1);
        break;
}
```

---

### **PHASE 2: Add Missing Controls** (4-6 hours)
**Priority: MEDIUM** - Expose projectM features users can't access

#### 2.1 Mesh Detail Slider (1.5 hours)
**New Setting:** `float meshDetail` (32–128)  
**Effect:** Smoother warp effects, GPU load  
**UI Location:** Advanced tab

```cpp
// config.h - add:
float meshDetail = 64.0f;  // 32-128

// menu_overlay.cpp - add slider:
if (ImGui::SliderFloat("Visual Detail", &m_config->meshDetail, 32.0f, 128.0f, "%.0f")) {
    changed = true;
}

// main.cpp:applyConfig() - apply:
int meshW = (int)cfg.meshDetail;
int meshH = meshW * 3 / 4;
projectm_set_mesh_size(g_pm, meshW, meshH);
```

#### 2.2 Aspect Correction Toggle (30 min)
**New Setting:** `bool aspectCorrection = true`  
**Effect:** Fixes ultrawide monitor stretching  
**UI Location:** Display tab

```cpp
// main.cpp:applyConfig()
projectm_set_aspect_correction(g_pm, cfg.aspectCorrection);
```

#### 2.3 Easter Egg (Preset Variety) (30 min)
**New Setting:** `float easterEgg = 0.0f` (0–1.0)  
**Effect:** Randomizes preset durations for variety  
**UI Location:** Presets tab

```cpp
projectm_set_easter_egg(g_pm, cfg.easterEgg);
```

#### 2.4 Audio Frequency Weighting (2-3 hours)
**New Settings:** `float bassBoost`, `float trebleBoost` (-1.0 to +1.0)  
**Effect:** Genre-specific reactivity (EDM vs rock)  
**Location:** Audio processing before projectM

---

### **PHASE 3: Reorganize UI** (3-4 hours)
**Priority: LOW** - Better organization, no new functionality

#### New Tab Structure:
```
┌─────────────────────────────────────────┐
│ [Vibes] [Presets] [Display] [Advanced] │
├─────────────────────────────────────────┤
│                                         │
│  VIBES TAB:                             │
│    • Audio Gain (0% - 300%)            │
│    • Beat Sensitivity (0 - 5)          │
│    • Speed (0.05x - 4.0x)              │
│    • Mood Presets (dropdown)           │
│    • Flash Limiter (checkbox)          │
│    • Reduced Motion (checkbox)         │
│                                         │
│  PRESETS TAB:                           │
│    • Auto-Advance (checkbox)           │
│    • Duration (10s - 120s)             │
│    • Transition Time (0s - 10s)        │
│    • Easter Egg / Variety (0 - 1.0)    │
│    • Hard Cut (enable/sensitivity)     │
│    • Vibe Lock (checkbox)              │
│                                         │
│  DISPLAY TAB:                           │
│    • Fullscreen (checkbox)             │
│    • Show FPS (checkbox)               │
│    • Font Scale (0.75 - 2.0)           │
│    • Overlay Opacity (10% - 95%)       │
│    • Aspect Correction (checkbox)      │
│                                         │
│  ADVANCED TAB:                          │
│    • Performance Mode (dropdown)       │
│    • Visual Detail / Mesh (32-128)     │
│    • Gamepad Deadzone (2k-16k)         │
│    • Shuffle (checkbox)                │
│                                         │
└─────────────────────────────────────────┘
```

#### Benefits:
- ✅ Related settings grouped logically
- ✅ Basic users see "Vibes" first (most important)
- ✅ Power users can dive into Advanced
- ✅ Fewer scrollbars, cleaner layout

---

## 📚 **PART 3: PRESET CATEGORIZATION**

### Current Problem:
- 9,797 presets in **flat shuffled list**
- No way to browse by visual style
- Random button is truly random (no filtering)

### Discovered Structure:
**Presets are ALREADY organized in folders!**
```
presets/
├─ Reaction/ (1,791)      ← Music-reactive effects
├─ Fractal/ (1,354)       ← Mathematical patterns
├─ Dancer/ (1,351)        ← Organic flowing motion
├─ Waveform/ (1,279)      ← Audio visualizations
├─ Drawing/ (1,143)       ← Line art aesthetics
├─ Geometric/ (1,027)     ← Shapes & architecture
├─ Sparkle/ (797)         ← Luminous particle effects
├─ Particles/ (389)
├─ Supernova/ (380)
├─ Hypnotic/ (280)
└─ ! Transition/ (4)
```

### Solution Designed:
**New PresetDatabase Class** (`src/preset_database.h/cpp` already created by agent!)

#### Features:
1. **Auto-detect categories** from folder structure at startup
2. **Category browser UI** - select category → see filtered presets
3. **Subcategory support** - e.g., Reaction → Liquid Ripples (470 presets)
4. **Author detection** - parse filenames, group by creator
5. **Tag system** - collaborations, remixes, noise variants
6. **Maintains existing features** - random/search/favorites still work

#### New UI Flow:
```
Main Menu
  ↓
Browse Presets
  ↓
┌─────────────────────────────────┐
│ [All] [By Category] [Favorites] │  ← New tabs
├─────────────────────────────────┤
│                                 │
│ BY CATEGORY VIEW:               │
│  ┌─────────────────────┐       │
│  │ 🎵 Reaction (1,791) │       │
│  │ 🌀 Fractal (1,354)  │       │
│  │ 💃 Dancer (1,351)   │       │
│  │ 🌊 Waveform (1,279) │       │
│  │ ✏️  Drawing (1,143)  │       │
│  │ 🔷 Geometric (1,027)│       │
│  │ ✨ Sparkle (797)    │       │
│  └─────────────────────┘       │
│                                 │
│  [Select category to browse]   │
│                                 │
└─────────────────────────────────┘
```

When user clicks category:
```
┌─────────────────────────────────┐
│ ← Back   Reaction (1,791)       │
├─────────────────────────────────┤
│ Search: [____________]          │
│                                 │
│ 📁 Liquid Ripples (470)         │
│ 📁 Liquid Blobby (196)          │
│ 📁 Rorschach (191)              │
│ 📁 Particle Fireworks (157)     │
│ ... (86 more subcategories)     │
│                                 │
│ [Random from Reaction] [Shuffle]│
└─────────────────────────────────┘
```

### Implementation Steps:
1. **Integrate PresetDatabase class** into project (1 hour)
2. **Add category browser UI** to MenuOverlay (2 hours)
3. **Wire category selection** to preset filtering (1 hour)
4. **Add "Random from Category"** button (30 min)
5. **Test with all 9,797 presets** (30 min)

---

## 🚀 **IMPLEMENTATION PRIORITIES**

### **Quick Wins** (Day 1 - 4 hours)
1. ✅ Disable touch functionality (30 min)
2. ✅ Fix gamepadDeadzone bug (5 min)
3. ✅ Remove broken uiScale (10 min)
4. ✅ Implement perfMode properly (2 hours)
5. ✅ Test all changes (1 hour)

### **High Impact** (Day 2 - 6 hours)
1. ✅ Add Mesh Detail slider (1.5 hours)
2. ✅ Add Aspect Correction toggle (30 min)
3. ✅ Integrate PresetDatabase (1 hour)
4. ✅ Build category browser UI (2 hours)
5. ✅ Test & iterate (1 hour)

### **Polish** (Day 3 - 4 hours)
1. ✅ Reorganize settings into 4 tabs (3 hours)
2. ✅ Add Easter Egg slider (30 min)
3. ✅ Final testing (30 min)

---

## 📊 **TOTAL EFFORT ESTIMATE**

| Task | Time | Priority |
|------|------|----------|
| Touch Removal | 30 min | HIGH |
| Settings Fixes (Phase 1) | 2-3 hours | HIGH |
| New Settings (Phase 2) | 4-6 hours | MEDIUM |
| Preset Categories | 4-6 hours | HIGH |
| UI Reorganization (Phase 3) | 3-4 hours | LOW |
| **TOTAL** | **14-20 hours** | — |

**Realistic Timeline:** 2-3 days of focused development

---

## 📁 **DELIVERABLES CREATED**

All design documents are ready in `docs/`:

| File | Purpose |
|------|---------|
| `SETTINGS_IMPLEMENTATION.md` | Complete code snippets for all changes |
| `SETTINGS_ARCHITECTURE.md` | System diagrams & UI mockups |
| `SETTINGS_CHECKLIST.md` | Phase-by-phase task tracker |
| `PRESET_CATEGORIES_DESIGN.md` | Category system design doc |
| `src/preset_database.h` | Ready-to-integrate class header |
| `src/preset_database.cpp` | Preset loading & categorization |

---

## 🎬 **NEXT STEPS**

**Option A: Start with Quick Wins** (Recommended)
- Disable touch in 30 minutes
- Fix broken settings in 2 hours
- Have cleaner, more functional build today

**Option B: Full Implementation** 
- Execute complete 3-part overhaul
- 2-3 day project
- Transforms user experience

**Option C: Review Designs First**
- Read the 6 design documents in `docs/`
- Provide feedback on approach
- Adjust priorities

---

## 🛠️ **READY TO IMPLEMENT?**

I can start immediately with any of these:
1. **Disable touch functionality** (30 min)
2. **Fix broken settings** (2-3 hours)
3. **Add preset categories** (4-6 hours)
4. **Complete overhaul** (14-20 hours)

Which would you like me to start with?
