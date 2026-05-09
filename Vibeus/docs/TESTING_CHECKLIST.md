# Testing Checklist - [EPIC NAME]

---

## Epic Overview
**Epic ID:** [EPIC-X]  
**Epic Name:** [e.g., Beat Detection & Audio Reactivity]  
**Sprint(s):** [Sprint numbers]  
**Testing Date:** [Date]  
**Tested By:** [Your name / AI-assisted]

---

## ✅ Pre-Testing Setup

### Environment:
- [ ] Windows 10/11 (build version: ______)
- [ ] GPU: [NVIDIA/AMD/Intel model]
- [ ] RAM: [Amount]
- [ ] Audio Device: [Speakers/Headphones model]
- [ ] Vibeus Version: [Build number or commit hash]
- [ ] projectM Version: [4.x.x]

### Test Data:
- [ ] Test music library prepared (10+ songs, various genres)
- [ ] Test presets selected (5-10 diverse presets)
- [ ] Fresh config.json (reset to defaults)
- [ ] Debug mode enabled (`--debug` flag)

---

## 🎵 EPIC 1: Beat Detection & Audio Reactivity

### Test Case 1.1: Accurate Beat Detection
**Story:** Users want visuals to sync with music beats

#### Test Scenarios:
- [ ] **EDM (140 BPM)** - Play electronic dance music
  - Visual beat pulses align with kick drum
  - Lag: <50ms (acceptable)
  - False positives: <10% (miss rate)
  - **Pass/Fail:** ______
  - **Notes:** ______

- [ ] **Rock (120 BPM)** - Play rock/metal music
  - Snare hits trigger visual response
  - Bass drum recognized as beats
  - **Pass/Fail:** ______
  - **Notes:** ______

- [ ] **Classical (60-80 BPM)** - Play orchestral music
  - Beat detection works with sparse beats
  - No false positives during silent passages
  - **Pass/Fail:** ______
  - **Notes:** ______

- [ ] **Hip-Hop (90 BPM)** - Play hip-hop/rap
  - 808 bass kicks detected
  - Syncopated rhythms handled
  - **Pass/Fail:** ______

- [ ] **Acoustic (Variable BPM)** - Play acoustic guitar
  - Beat detection adapts to tempo changes
  - Strumming patterns recognized
  - **Pass/Fail:** ______

#### Performance:
- [ ] CPU usage: <20% during beat detection
- [ ] No audio stuttering or dropouts
- [ ] Beat detection latency: <50ms

#### Settings:
- [ ] Beat sensitivity slider (0-5) visibly affects detection
- [ ] Debug overlay shows beat indicator (red flash)
- [ ] Settings persist after restart

---

### Test Case 1.2: Genre-Specific Audio Profiles
**Story:** Users want presets optimized for their music type

#### Test Scenarios:
- [ ] **EDM Profile** - Select "EDM" from dropdown
  - Bass frequencies boosted
  - High-energy response
  - Visuals "pump" with bass
  - **Pass/Fail:** ______

- [ ] **Rock Profile** - Select "Rock"
  - Mid-range emphasis
  - Guitar frequencies highlighted
  - Balanced response
  - **Pass/Fail:** ______

- [ ] **Classical Profile** - Select "Classical"
  - Full frequency spectrum
  - Smooth, less aggressive response
  - No overwhelming bass
  - **Pass/Fail:** ______

- [ ] **Custom Profile** - Manually adjust bass/treble sliders
  - Changes take effect immediately
  - Saved to config.json
  - **Pass/Fail:** ______

#### Validation:
- [ ] Profile changes visibly affect visualization
- [ ] Switching profiles mid-song works smoothly
- [ ] Settings persist after restart

---

### Test Case 1.3: Visual Beat Feedback
**Story:** Users want to see when beats are detected

#### Test Scenarios:
- [ ] **Debug Overlay** - Enable debug mode
  - Beat indicator (red flash/pulse) appears
  - Indicator syncs with audio beats
  - Indicator toggleable with keyboard shortcut
  - **Pass/Fail:** ______

- [ ] **Beat Counter** - Debug mode shows beat count
  - Increments on each detected beat
  - Resets when song changes
  - **Pass/Fail:** ______

---

## 🌊 EPIC 2: Smooth Transitions & Flow

### Test Case 2.1: Smooth Blend Transitions
**Story:** Users want seamless preset changes

#### Test Scenarios:
- [ ] **0s Transition (Instant)** - Set duration to 0
  - Instant hard cut between presets
  - No visual artifacts
  - **Pass/Fail:** ______

- [ ] **2s Transition (Short Fade)** - Set duration to 2s
  - Smooth alpha blend over 2 seconds
  - Outgoing preset fades out
  - Incoming preset fades in
  - No black frames or glitches
  - **Pass/Fail:** ______

- [ ] **10s Transition (Long Blend)** - Set duration to 10s
  - Extended crossfade
  - Both presets visible during transition
  - Smooth interpolation
  - **Pass/Fail:** ______

#### Performance:
- [ ] FPS stable during transition (60 FPS)
- [ ] No stuttering or frame drops
- [ ] GPU usage: <80%

---

### Test Case 2.2: Smart Hard Cut Logic
**Story:** Users want hard cuts on musical peaks

#### Test Scenarios:
- [ ] **Hard Cut Probability 0%** - Disable hard cuts
  - All transitions are smooth blends
  - No sudden cuts
  - **Pass/Fail:** ______

- [ ] **Hard Cut Probability 50%** - 50% chance on beat
  - Some transitions are hard cuts
  - Cuts happen on beat events
  - Feels intentional, not random
  - **Pass/Fail:** ______

- [ ] **Hard Cut Probability 100%** - Always hard cut
  - All transitions are instant
  - Cuts align with beats
  - **Pass/Fail:** ______

#### Validation:
- [ ] Hard cuts feel musically timed
- [ ] Not jarring or disorienting
- [ ] User can adjust sensitivity

---

### Test Case 2.3: Preset Duration Variety
**Story:** Users want some presets to linger longer

#### Test Scenarios:
- [ ] **Easter Egg 0.0** - Fixed duration
  - All presets stay for exact duration (e.g., 30s)
  - Predictable timing
  - **Pass/Fail:** ______

- [ ] **Easter Egg 0.5** - Moderate variety
  - Some presets 15s, some 45s
  - Variety without chaos
  - **Pass/Fail:** ______

- [ ] **Easter Egg 1.0** - Maximum variety
  - Presets range from 10s to 90s+
  - Unpredictable but organic
  - **Pass/Fail:** ______

---

### Test Case 2.4: Transition Effect Types
**Story:** Users want different transition styles

#### Test Scenarios:
- [ ] **Fade Transition** - Select "Fade"
  - Alpha blend between presets
  - **Pass/Fail:** ______

- [ ] **Wipe Transition** - Select "Wipe"
  - Directional wipe effect
  - **Pass/Fail:** ______
  - **Notes:** May not be implemented (research spike)

- [ ] **Morph Transition** - Select "Morph"
  - Preset morphs into next
  - **Pass/Fail:** ______
  - **Notes:** May not be implemented

---

## ⚙️ EPIC 3: Settings Overhaul

### Test Case 3.1: Fix Broken Settings
**Story:** All settings should work as expected

#### Test Scenarios:
- [ ] **gamepadDeadzone** - Adjust slider (2k-16k)
  - Deadzone affects stick sensitivity
  - Lower values = more sensitive
  - Higher values = larger deadzone
  - **Pass/Fail:** ______

- [ ] **perfMode** - Select performance mode
  - Battery Saver: 32×24 mesh, no VSync, 30 FPS
  - Balanced: 64×48 mesh, VSync, 60 FPS
  - Quality: 128×96 mesh, VSync, 60 FPS
  - Changes apply immediately
  - **Pass/Fail:** ______

- [ ] **uiScale** - Should be removed
  - Setting no longer exists in UI
  - No crashes if old config.json has it
  - **Pass/Fail:** ______

---

### Test Case 3.2: Visual Detail Control
**Story:** Users can adjust mesh size for performance

#### Test Scenarios:
- [ ] **Mesh Size 32** - Lowest detail
  - Blocky/pixelated warp effects
  - High FPS (100+)
  - **Pass/Fail:** ______

- [ ] **Mesh Size 64** - Balanced detail
  - Smooth warp effects
  - 60 FPS stable
  - **Pass/Fail:** ______

- [ ] **Mesh Size 128** - Maximum detail
  - Ultra-smooth warp effects
  - FPS may drop on low-end GPU
  - **Pass/Fail:** ______

#### Performance:
- [ ] Changes apply immediately (no restart)
- [ ] FPS scales inversely with mesh size

---

### Test Case 3.3: Performance Profiles
**Story:** One-click performance presets

#### Test Scenarios:
- [ ] **Battery Saver** - Select profile
  - Mesh: 32×24
  - VSync: Off
  - FPS: 30 (capped)
  - Power draw: <10W (if measurable)
  - **Pass/Fail:** ______

- [ ] **Balanced** - Select profile
  - Mesh: 64×48
  - VSync: On
  - FPS: 60
  - **Pass/Fail:** ______

- [ ] **Quality** - Select profile
  - Mesh: 128×96
  - VSync: On
  - FPS: 60
  - **Pass/Fail:** ______

- [ ] **Steam Deck Auto-Detect** - Run on Steam Deck
  - Automatically selects Battery Saver
  - **Pass/Fail:** ______

---

### Test Case 3.4: Reorganize Settings Tabs
**Story:** Settings are grouped logically

#### Test Scenarios:
- [ ] **Vibes Tab** - Check layout
  - Audio Gain, Beat Sensitivity, Speed, Flash Limiter
  - Most important settings visible
  - No scrolling required on 1280×800
  - **Pass/Fail:** ______

- [ ] **Presets Tab** - Check layout
  - Duration, Transition Time, Easter Egg, Hard Cut
  - Clear labels and tooltips
  - **Pass/Fail:** ______

- [ ] **Display Tab** - Check layout
  - Fullscreen, FPS, Font Scale, Overlay Opacity
  - **Pass/Fail:** ______

- [ ] **Advanced Tab** - Check layout
  - Performance Mode, Mesh Detail, Gamepad Deadzone
  - For power users
  - **Pass/Fail:** ______

---

## 📚 EPIC 4: Preset Management

### Test Case 4.1: Category Browser
**Story:** Users can browse presets by visual style

#### Test Scenarios:
- [ ] **Category List** - Open preset browser
  - Shows 11 categories with counts
  - Categories: Dancer, Fractal, Reaction, Waveform, etc.
  - Counts match actual preset numbers
  - **Pass/Fail:** ______

- [ ] **Select Category** - Click "Fractal"
  - Filters to only Fractal presets (1,354)
  - Can navigate back to "All"
  - **Pass/Fail:** ______

- [ ] **Search Within Category** - Type "martin" in search box
  - Filters Fractal presets by author name
  - Combined filtering works
  - **Pass/Fail:** ______

---

### Test Case 4.2: Favorites System
**Story:** Users can star favorite presets

#### Test Scenarios:
- [ ] **Star a Preset** - Click star icon
  - Star changes from ☆ to ★
  - Preset added to favorites list
  - **Pass/Fail:** ______

- [ ] **Unstar a Preset** - Click filled star
  - Star changes from ★ to ☆
  - Preset removed from favorites
  - **Pass/Fail:** ______

- [ ] **Favorites Tab** - Navigate to Favorites
  - Shows only starred presets
  - Count accurate
  - **Pass/Fail:** ______

- [ ] **Persistence** - Restart app
  - Favorites still present
  - Saved in config.json
  - **Pass/Fail:** ______

#### Performance:
- [ ] Can favorite 100+ presets without lag
- [ ] Favorites tab loads in <100ms

---

### Test Case 4.3: Custom Playlists
**Story:** Users can create named playlist collections

#### Test Scenarios:
- [ ] **Create Playlist** - Click "New Playlist"
  - Prompt for name (e.g., "Party")
  - Playlist created and saved
  - **Pass/Fail:** ______

- [ ] **Add Presets to Playlist** - Select presets
  - Multi-select or drag-drop
  - Presets added to "Party" playlist
  - **Pass/Fail:** ______

- [ ] **Play Playlist** - Select "Party" playlist
  - Shuffle/random scoped to playlist only
  - Doesn't play presets outside playlist
  - **Pass/Fail:** ______

- [ ] **Rename Playlist** - Right-click → Rename
  - Playlist name updated
  - **Pass/Fail:** ______

- [ ] **Delete Playlist** - Right-click → Delete
  - Playlist removed (presets remain)
  - **Pass/Fail:** ______

#### Persistence:
- [ ] Playlists saved in config.json
- [ ] Playlists survive app restart

---

### Test Case 4.4: Upload Custom Presets
**Story:** Users can import `.milk` files

#### Test Scenarios:
- [ ] **Import Preset** - Click "Import Preset" button
  - File picker opens
  - Select `.milk` file
  - Preset copied to custom folder
  - Appears in "Custom" category
  - **Pass/Fail:** ______

- [ ] **Invalid File** - Try to import `.txt` file
  - Error message: "Invalid preset file"
  - No crash
  - **Pass/Fail:** ______

- [ ] **Duplicate Import** - Import same preset twice
  - Handles gracefully (overwrite or rename)
  - **Pass/Fail:** ______

---

### Test Case 4.5: Preset Search & Filtering
**Story:** Users can search presets by name/author

#### Test Scenarios:
- [ ] **Search by Name** - Type "trippy"
  - Filters presets with "trippy" in filename
  - Real-time filtering (no lag)
  - **Pass/Fail:** ______

- [ ] **Search by Author** - Type "flexi"
  - Shows all presets by author "flexi"
  - **Pass/Fail:** ______

- [ ] **Clear Search** - Delete search text
  - Returns to full preset list
  - **Pass/Fail:** ______

---

## 🎨 EPIC 5: UI/UX Polish

### Test Case 5.1: ImGui Theme & Styling
**Story:** Interface should look modern and professional

#### Test Scenarios:
- [ ] **Visual Inspection** - Open menu
  - Colors match visualizer aesthetic
  - Rounded buttons (if applicable)
  - Smooth hover/click animations
  - Dark theme with neon accents
  - **Pass/Fail:** ______

- [ ] **Consistency** - Check all tabs
  - Padding and spacing consistent
  - Font sizes readable
  - Icons aligned
  - **Pass/Fail:** ______

---

### Test Case 5.2: Preset Metadata Display
**Story:** Show preset author credits

#### Test Scenarios:
- [ ] **Preset Info Overlay** - During playback
  - Shows "Preset: [name] by [author]"
  - Toggleable with keyboard shortcut
  - **Pass/Fail:** ______

- [ ] **Credits Screen** - Navigate to Credits
  - Lists top preset contributors
  - Links to projectM
  - **Pass/Fail:** ______

---

### Test Case 5.3: Onboarding Tutorial
**Story:** New users learn controls easily

#### Test Scenarios:
- [ ] **First Run** - Launch app with fresh config
  - Tutorial overlay appears
  - Shows basic controls (arrows, menu, fullscreen)
  - "Got it" button dismisses
  - **Pass/Fail:** ______

- [ ] **Replay Tutorial** - Navigate to Help → Tutorial
  - Tutorial overlay appears again
  - **Pass/Fail:** ______

---

### Test Case 5.4: Improved Debug Overlay
**Story:** Power users see detailed metrics

#### Test Scenarios:
- [ ] **Enable Debug Overlay** - Press `D` key
  - Shows FPS, frame time
  - Shows audio buffer stats
  - Shows beat detection indicator
  - Shows preset name
  - **Pass/Fail:** ______

- [ ] **Performance Metrics** - Check accuracy
  - FPS counter accurate (compare to external tool)
  - Frame time in ms
  - **Pass/Fail:** ______

---

## 🔧 EPIC 7: Performance & Stability

### Test Case 7.1: Preset Crash Quarantine
**Story:** App shouldn't crash on buggy presets

#### Test Scenarios:
- [ ] **Crashing Preset** - Play known buggy preset
  - App detects crash or OpenGL error
  - Auto-skips to next preset
  - Logs preset to quarantine list
  - **Pass/Fail:** ______

- [ ] **Quarantine List** - View quarantine list
  - Shows quarantined presets
  - User can remove from quarantine
  - **Pass/Fail:** ______

---

### Test Case 7.2: Memory Leak Detection
**Story:** App runs stably for hours

#### Test Scenarios:
- [ ] **2-Hour Stress Test** - Run app for 2+ hours
  - Memory usage stable (<500MB growth)
  - No slowdown or stuttering
  - FPS remains consistent
  - **Pass/Fail:** ______

- [ ] **Memory Profiling** - Use Visual Studio diagnostics
  - No detected memory leaks
  - All allocations freed
  - **Pass/Fail:** ______

---

### Test Case 7.3: GPU Compatibility
**Story:** Works on different GPUs

#### Test Scenarios:
- [ ] **NVIDIA GPU** - Test on NVIDIA card
  - Renders correctly
  - 60 FPS stable
  - **Pass/Fail:** ______

- [ ] **AMD GPU** - Test on AMD card
  - Renders correctly
  - 60 FPS stable
  - **Pass/Fail:** ______

- [ ] **Intel iGPU** - Test on integrated GPU
  - Renders correctly (may need lower mesh)
  - 30+ FPS on balanced settings
  - **Pass/Fail:** ______

- [ ] **Unsupported GPU** - Test on old GPU (pre-OpenGL 3.3)
  - Graceful error message
  - Doesn't crash
  - **Pass/Fail:** ______

---

## 📊 Overall Epic Results

### Summary:
- **Total Test Cases:** [Count]
- **Passed:** [Count] ✅
- **Failed:** [Count] ❌
- **Not Tested:** [Count] ⚠️

### Critical Issues:
1. [Issue 1 - Description]
2. [Issue 2 - Description]

### Recommendations:
- [ ] [Recommendation 1]
- [ ] [Recommendation 2]

---

## ✅ Sign-Off

**Epic Ready for Release:** Yes / No / With Caveats

**Tester:** [Your name]  
**Date:** [Date]  
**Build Version:** [Version number or commit]

**Notes:**
[Any final notes or observations]
