# Broken Preset Quarantine Feature

**Date:** 2026-03-23  
**Status:** ✅ **IMPLEMENTED**

---

## Problem

User reported: "I went to fire up the app and it was still seeing all the broken presets that were already detected"

### **Root Cause:**
The validation was only removing presets from the in-memory playlist, not from disk. On every app restart, all preset files (including broken ones) were re-loaded from the presets folder, causing broken presets to reappear.

---

## Solution: Quarantine System

Implemented a **quarantine folder** that moves broken preset files out of the presets directory.

### **How It Works:**

1. **Detection:** Validation identifies presets with shader errors
2. **Quarantine:** Broken `.milk` files are MOVED to `broken_presets_quarantine/`
3. **Persistence:** Since files are physically moved, they won't reload on next startup
4. **Safety:** Files aren't deleted - user can review and restore if needed

---

## Implementation Details

### **File Movement:**
```cpp
fs::path quarantineDir = fs::path(SDL_GetBasePath()) / "broken_presets_quarantine";
fs::create_directories(quarantineDir);

// For each broken preset:
fs::path presetPath(name);
fs::path quarantinePath = quarantineDir / presetPath.filename();
fs::rename(presetPath, quarantinePath); // Move file
```

### **Duplicate Handling:**
If a preset with the same name already exists in quarantine (from a previous run), a number suffix is added:
- `broken_preset.milk`
- `broken_preset_1.milk`
- `broken_preset_2.milk`

---

## Directory Structure

**Before Validation:**
```
Release/
├── Vibeus.exe
├── presets/
│   ├── working_preset_1.milk
│   ├── broken_preset.milk ← Has shader error
│   └── working_preset_2.milk
└── vibeus_config.json
```

**After Validation:**
```
Release/
├── Vibeus.exe
├── presets/
│   ├── working_preset_1.milk
│   └── working_preset_2.milk  ← Only working presets remain
├── broken_presets_quarantine/
│   └── broken_preset.milk  ← Moved here
├── broken_presets.txt  ← Log of what was moved
└── vibeus_config.json
```

---

## User Experience

### **Console Output:**
```
[PresetManager] Validating 9795 presets...
[PresetManager] Created quarantine: F:\...\broken_presets_quarantine
[PresetManager] Testing preset 0/9795...
[PresetManager] Testing preset 100/9795...
...
[PresetManager] QUARANTINED: broken_preset.milk
...
[PresetManager] Validation complete: 42 broken presets found
[PresetManager] Removing 42 broken presets...
[PresetManager] Saved broken preset list to broken_presets.txt

[Vibeus] Removed 42 broken presets (see broken_presets.txt)
[Vibeus] Final count: 9753 working presets
```

### **broken_presets.txt:**
```
# Broken Presets Detected by Vibeus
# These presets had shader compilation errors and were MOVED to quarantine.
# Total: 42 presets
#
# Location: broken_presets_quarantine/ (same directory as Vibeus.exe)
#
# To restore a preset:
# 1. Move the .milk file back to the presets folder
# 2. Restart Vibeus
#
# To review:
# 1. Open in text editor to inspect shader code
# 2. Try loading in projectM standalone
# 3. Report to preset pack maintainer for fixes
#

F:\...\presets\broken_preset_1.milk
F:\...\presets\broken_preset_2.milk
...
```

---

## Benefits

### **✅ Persistent:**
- Broken presets won't reload on restart
- User sees clean preset list every time

### **✅ Safe:**
- No data loss - files are moved, not deleted
- Can review broken presets in quarantine folder
- Easy to restore: just move file back to presets/

### **✅ Informative:**
- `broken_presets.txt` lists what was moved
- Console shows which files were quarantined
- Clear instructions for restoration

---

## Files Modified

**preset_manager.cpp:**
- Added `#include <filesystem>` 
- Created `broken_presets_quarantine/` directory
- Added file move logic in `validateAndFilter()`
- Updated `saveBrokenPresetLog()` with quarantine instructions

**Changes:** +25 lines

---

## Build Status

✅ **SUCCESS** - MSVC 17.14, Release x64  
**Timestamp:** 2026-03-23 00:00

---

## Testing Instructions

### **Initial Run (with broken presets):**
1. Run Vibeus.exe
2. Wait for validation (~8 minutes for 9795 presets)
3. Check console: `QUARANTINED: preset_name.milk`
4. Check folders:
   - `presets/` - broken files should be gone
   - `broken_presets_quarantine/` - contains moved files
   - `broken_presets.txt` - list of moved files

### **Second Run (verification):**
1. Run Vibeus.exe again
2. Validation should complete quickly (all presets working)
3. Console: `All presets validated successfully!`
4. No new files in quarantine

### **Restore Test (optional):**
1. Move a file from quarantine back to presets/
2. Run Vibeus.exe
3. File should be detected as broken again
4. Moved back to quarantine (with `_1` suffix if original still there)

---

## Edge Cases Handled

### **1. Quarantine Directory Missing:**
- Creates directory automatically on first run
- No manual setup required

### **2. Duplicate Filenames:**
- Adds numeric suffix: `preset.milk`, `preset_1.milk`, `preset_2.milk`
- Prevents overwriting previously quarantined files

### **3. File Move Fails:**
- Catches exceptions and logs error
- Continues validation of remaining presets
- File stays in presets folder (will be detected again next time)

### **4. Empty Quarantine:**
- If no broken presets found, quarantine folder created but empty
- `broken_presets.txt` not created (only created when issues found)

---

## User Actions

### **To Restore All Presets:**
```bash
# Move all files back from quarantine
cd F:\chilltittiesvisualizer\Vibeus\build\Release
move broken_presets_quarantine\*.milk presets\
```

### **To Permanently Delete Quarantined Presets:**
```bash
# Delete quarantine folder
rmdir /s broken_presets_quarantine
```

### **To Disable Validation:**
1. Open Settings → Advanced
2. Uncheck "Validate Presets on Startup"
3. Save and restart
4. All presets (including broken) will load

---

## Future Enhancements

### **Possible Improvements:**
1. **UI Panel:** "View Quarantined Presets" with restore button
2. **Selective Restore:** Restore individual presets from UI
3. **Quarantine Report:** Detailed error messages for each preset
4. **Auto-Delete:** Option to permanently delete after X days
5. **Export:** Button to zip quarantine for bug reports

---

## Performance Impact

**None** - File moves happen during validation, which already takes ~8 minutes.  
File system operations are negligible compared to shader compilation time.

---

**Status:** ✅ **Complete - Broken presets will now stay removed!**
