# Vibeus v0.2.1-dev Release Notes

**Release Date:** 2026-03-23  
**Build Type:** Development Release  
**Installer:** `Vibeus-0.2.1-dev-setup.exe` (5.26 MB)

---

## 🎉 What's New in v0.2.1-dev

### **🔥 Major Features**

#### **1. Automatic Broken Preset Detection & Quarantine**
- Validates all presets on startup for shader compilation errors
- **Quarantine System:** Broken `.milk` files are moved to `broken_presets_quarantine/` folder
- Creates `broken_presets.txt` log with list of quarantined presets
- Broken presets won't reload on restart - permanently removed from playlist!
- Safe & reversible: files moved, not deleted
- Console feedback: Shows progress every 100 presets

**User Impact:** Clean preset list, no shader error spam, smooth visual experience!

---

#### **2. Controls Panel** 🎮
- New "Controls" tab in Settings menu
- Displays all 22 keyboard bindings + 13 gamepad bindings
- Color-coded by category (Navigation, Audio, Speed, etc.)
- Control bindings now stored in config and persist between sessions

**Ready for v0.2.2:** Full control remapping UI coming next!

---

#### **3. Settings Overhaul - Phase 2** ⚙️

**Fixed Broken Settings:**
- ✅ **Gamepad Deadzone:** Now actually applies (was hardcoded at 8000)
- ✅ **Performance Mode:** 
  - Battery Saver: 32×24 mesh, no VSync
  - Balanced: 64×48 mesh, VSync on
  - Quality: 128×96 mesh, VSync on
- ❌ **UI Scale:** Removed (never worked, feature deleted)

**New Settings:**
- **Mesh Detail:** Slider 32-128 (controls warp mesh resolution)
- **Aspect Correction:** Toggle ultrawide monitor stretching fix
- **Easter Egg:** 0-100% slider (preset duration variety)
- **Validate Presets on Startup:** Toggle preset validation (Settings → Advanced)

---

### **🐛 Bug Fixes**

#### **1. Config Loading Order Bug**
- **Problem:** Config was loaded AFTER preset validation check
- **Fix:** Moved config loading to happen BEFORE validation
- **Result:** `validatePresetsOnStartup` setting now actually works!

#### **2. ImGui Missing End() Bug**
- **Problem:** Early returns in splash screen bypassed `ImGui::End()`
- **Fix:** Moved `End()` before return statements
- **Result:** No more `[imgui-error] ##splash: missing End()` messages

#### **3. Touch Functionality Disabled**
- All touch/flow mode code neutralized with `false &&` guards
- Can be easily re-enabled in future if needed
- Prevents broken touch interactions

---

## 📦 Installer Updates

**v0.2.1-dev Installer Includes:**
- ✅ Vibeus.exe with all new features
- ✅ SDL2, projectM-4, projectM-4-playlist DLLs
- ✅ 9,795+ presets (cream-of-the-crop collection)
- ✅ Milkdrop texture pack
- ✅ Default configuration with new settings
- ✅ Favorites system (preserved on updates)

**Installation:**
1. Run `Vibeus-0.2.1-dev-setup.exe`
2. Choose install location (default: `C:\Program Files\Vibeus`)
3. Wait for preset files to extract (~5.3 MB compressed)
4. Launch Vibeus from Start Menu or Desktop

**First Launch:**
- Preset validation will run automatically (~8 minutes for 9,795 presets)
- Console shows: `[Vibeus] Validating presets for shader errors...`
- Broken presets moved to `broken_presets_quarantine/` folder
- `broken_presets.txt` created with list of quarantined files

---

## 🎯 Performance & Validation

### **Validation Stats:**
- **Time:** ~8 minutes for 9,795 presets (50ms per preset)
- **Progress:** Updates every 100 presets
- **Detection:** Checks for shader compilation errors
- **Action:** Moves broken files to quarantine folder
- **Configurable:** Can be disabled in Settings → Advanced

### **Expected Results:**
- Typically 0.5-1% of presets have shader errors (~50-100 broken)
- Varies by GPU, drivers, and preset pack
- Broken presets saved for review and potential upstream fixes

---

## 📁 Directory Structure (Post-Install)

```
C:\Program Files\Vibeus\
├── Vibeus.exe                    ← Main application (v0.2.1-dev)
├── SDL2.dll
├── projectM-4.dll
├── projectM-4-playlist.dll
├── vibeus_config.json            ← Settings (JSON)
├── presets/                      ← 9,795+ .milk files
│   ├── Fractal/
│   ├── Waveform/
│   ├── Abstract/
│   └── ...
├── textures/                     ← Milkdrop textures
├── broken_presets_quarantine/    ← Broken presets moved here (NEW!)
├── broken_presets.txt            ← Log of quarantined files (NEW!)
└── favorites.txt                 ← User favorites
```

---

## 🔄 Upgrade from v0.2.0

**Automatic:**
- Installer preserves existing `vibeus_config.json`
- Favorites preserved (`favorites.txt`)
- Old presets replaced with new collection

**Manual Steps:**
1. Backup your `favorites.txt` if customized
2. Run new installer
3. First launch will re-validate all presets
4. Old `broken_presets_quarantine/` folder preserved if exists

---

## 🛠️ For Developers

### **Build from Source:**
```bash
cd F:\chilltittiesvisualizer\Vibeus
cmake --build build --config Release
cd installer
.\build-installer.bat
```

### **Files Modified This Release:**
- `src/main.cpp` (+50 lines) - Config order fix, validation call, version bump
- `src/preset_manager.h/cpp` (+138 lines) - Validation, quarantine, logging
- `src/config.h/cpp` (+8 lines) - New settings fields
- `src/menu_overlay.h/cpp` (+190 lines) - Controls panel, new setting controls
- `installer/vibeus-installer.iss` (1 line) - Version 0.2.1-dev

### **New Documentation:**
- `PRESET_FILTERING_LOG.md` - Validation implementation
- `QUARANTINE_FEATURE_LOG.md` - Quarantine system details
- `VALIDATION_INIT_BUGFIX.md` - Config order bug fix
- `CONTROL_REMAPPING_STATUS.md` - Control binding infrastructure
- `IMGUI_FIX_AND_CONTROLS_LOG.md` - ImGui bug + Controls panel

---

## 🎮 Controls Reference

### **Keyboard:**
- **N / Right Arrow** - Next preset
- **P / Left Arrow** - Previous preset
- **R** - Random preset
- **H** - History (last preset)
- **S** - Toggle shuffle
- **F / F11** - Toggle fullscreen
- **D** - Toggle debug info
- **Q** - Quit
- **Up / Down** - Beat sensitivity
- **[ / ]** - Speed down/up
- **Backspace** - Reset speed
- **- / =** - Audio gain down/up
- **0** - Reset audio gain
- **Esc** - Pause menu
- **Tab** - Settings menu

### **Gamepad:**
- **A** - Next preset
- **B** - Previous preset
- **X** - Random preset
- **Y** - Toggle shuffle
- **LB / RB** - Audio gain down/up
- **Start** - Pause menu

---

## ⚠️ Known Issues

1. **First Launch Delay:** 8-minute validation on first run (can be disabled)
2. **Control Remapping:** UI not yet implemented (bindings stored in config, manual edit possible)
3. **Settings UI:** Phase 3 reorganization pending (4-tab layout coming in v0.2.2)
4. **Preset Categories:** Not yet implemented (9,797 presets in one long list)

---

## 🚀 Roadmap (v0.2.2+)

### **Coming Soon:**
- [ ] Control remapping UI with "Remap" buttons
- [ ] Settings reorganization into 4 tabs (Vibes/Presets/Display/Advanced)
- [ ] Preset categorization system (organize by type: Fractal, Waveform, etc.)
- [ ] Validation result caching (skip re-scan on subsequent launches)
- [ ] Quarantine management UI (view/restore broken presets)

### **Future Features:**
- [ ] Mobile/tablet support (Android port)
- [ ] Custom preset import/export
- [ ] Preset rating system
- [ ] Audio input device selection
- [ ] Multi-monitor support
- [ ] Plugin system for custom effects

---

## 📞 Support & Feedback

**Issues:** Report bugs on GitHub Issues  
**Discord:** Join our community for support  
**Documentation:** See `README.md` and `docs/` folder

---

## 📜 Changelog Summary

**v0.2.1-dev (2026-03-23):**
- ✨ Preset validation & quarantine system
- ✨ Controls panel with binding display
- ✨ New settings: meshDetail, aspectCorrection, easterEgg
- 🐛 Fixed config loading order bug
- 🐛 Fixed ImGui missing End() error
- 🐛 Fixed gamepad deadzone not applying
- 🐛 Fixed performance mode not switching
- 🗑️ Removed broken uiScale setting
- 🚫 Disabled touch functionality
- 📝 Control bindings now persist in config

**v0.2.0 (2026-03-22):**
- Initial development release
- Settings overhaul Phase 1 & 2
- Inno Setup installer

---

**Thank you for testing Vibeus v0.2.1-dev! 🎨🎵**
