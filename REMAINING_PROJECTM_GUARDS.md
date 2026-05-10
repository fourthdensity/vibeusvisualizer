# Remaining ProjectM Guards Needed in main.cpp

## Status
The major architectural changes have been completed:
- ✅ AudioCapture guards for `projectm_handle` parameter
- ✅ MenuOverlay guards for `projectm_playlist_handle` parameter
- ✅ ProjectM touch types, ripples, and velocity trail functions guarded
- ✅ PresetManager conditionally compiled in CMakeLists.txt
- ✅ PresetManager header and global variable guarded

## Remaining Work

### 1. Guard all g_presets usage in main.cpp (~30 locations)

The following functions contain g_presets calls that need `#ifdef USE_PROJECTM_BACKEND` guards:

#### updateDebugTitle() (line ~274)
- Lines 281, 296, 307: `g_presets.currentPresetName()`, `g_presets.position()`, `g_presets.count()`
- **Solution**: Add conditional blocks to show "N/A" or simplified debug info when ProjectM is disabled

#### handleVisualizerKeyboard() (line ~610)
- Lines 621-663: All preset navigation/management keys (next, prev, random, shuffle, blacklist, favorite, quarantine)
- **Solution**: Wrap entire preset control block (lines 620-668) in `#ifdef USE_PROJECTM_BACKEND`

#### doStorytellerLogic() (line ~780)
- Line 784: `g_presets.next(false)`
- **Solution**: Guard storyteller preset transitions for ProjectM backend only

#### handleGamepadInput() (line ~835)
- Lines 841, 846, 851, 856-857: Gamepad preset controls (next, prev, random, shuffle)
- **Solution**: Wrap gamepad preset controls in `#ifdef USE_PROJECTM_BACKEND`

#### applyPendingSettings() (line ~1020)
- Lines 1026-1027: `g_presets.isShuffled()`, `g_presets.toggleShuffle()`
- **Solution**: Guard shuffle state synchronization

#### findPlaylistPositionByPath() (line ~1067)
- Function signature uses `projectm_playlist_handle`
- Lines 1074: `g_presets.count()`
- **Solution**: Guard entire function with `#ifdef USE_PROJECTM_BACKEND`

#### handleMenuActions() (line ~1100)
- Lines 1114, 1138: `g_presets.handle()`, `g_presets.count()`
- **Solution**: Guard preset browser actions for ProjectM

#### SDL/main initialization (line ~1300)
- Lines 1306-1332: PresetManager init, validation, and filtering
- **Solution**: Guard entire preset initialization block

#### main visualization loop (line ~1550)
- Line 1553: `g_presets.currentPresetName()`
- **Solution**: Guard status update for ProjectM

### 2. Other ProjectM-specific items

#### g_lastPresetHadError, g_captureProjectMErrors, g_validationLogFile (line ~256)
- Currently inside `#ifdef USE_PROJECTM_BACKEND` block ✅

#### findPlaylistPositionByPath() function
- Guard with `#ifdef USE_PROJECTM_BACKEND` (entire function)

## Build Test Command

After completing guards:
```bash
cmake .. -DUSE_PROJECTM=OFF -DUSE_VIBEUS_BACKEND=ON \
  -DCMAKE_TOOLCHAIN_FILE="vcpkg/scripts/buildsystems/vcpkg.cmake" -A x64
cmake --build . --config Release
```

## Expected Behavior

When built with `USE_PROJECTM=OFF USE_VIBEUS_BACKEND=ON`:
- No ProjectM headers should be included in compilation
- No linking to ProjectM libraries
- Preset management features disabled (key bindings become no-ops or removed)
- Visualization works with VibeusVisualizer backend
- Audio capture and beat detection still functional

## Implementation Strategy

1. **Systematic approach**: Work through each function listed above
2. **Fallback behavior**: When guarding controls, either:
   - Remove the functionality (silent no-op)
   - Show toast "Feature requires ProjectM backend"
   - Provide alternative behavior for Vibeus backend
3. **Test incrementally**: After each major section, attempt a build
