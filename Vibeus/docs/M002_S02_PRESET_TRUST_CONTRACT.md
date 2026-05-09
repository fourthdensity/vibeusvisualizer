# M002 S02 — Preset Trust Flow Contract

**Milestone:** M002 Playable Product Closure  
**Slice:** S02 Preset Curation Status & Browser Trust  
**Task:** T01 Preset Trust Flow Audit  
**Generated:** 2026-05-08

## Purpose

S02 must make preset validation/quarantine and browsing trustworthy without creating a second source of truth. The key invariant from project context still applies:

> `PresetManager` owns the live projectM playlist. `PresetDatabase` owns categorized browser indexing. Their indices are not guaranteed to match. Bridge by full path.

## Existing Surfaces

| Surface | File | Current Role | Status |
|---|---|---|---|
| Live playlist | `preset_manager.*` | Owns projectM playlist, next/prev/random/blacklist, validation/quarantine removal. | Keep as source of truth for playback positions. |
| Browser database | `preset_database.*` | Scans presets into categories, parsed display names/authors/tags, search indices. | Keep as source of truth for UI metadata. |
| Browser UI | `menu_overlay.cpp` | Shows search, favorites, categories, category counts, grid display. | Needs path bridge for PlayPreset. |
| Validation toggle | `menu_overlay.cpp` Advanced → Preset Management | Exposes `validatePresetsOnStartup` with tooltip. | Needs clearer status/help text. |
| Validation/quarantine runtime | `main.cpp`, `preset_manager.cpp` | Applies blacklists, optionally validates, writes validation log, moves/blacklists broken presets. | Functional, but mostly log/file-system visible. |

## Audit Findings

### 1. Browser playback assumed browser index == playlist position — RESOLVED

Evidence before T02:

```text
Vibeus/src/menu_overlay.cpp:846: m_selectedPreset = idx;
Vibeus/src/main.cpp:1231: uint32_t idx = g_menu.selectedPresetIndex();
Vibeus/src/main.cpp:1232: projectm_playlist_set_position(g_presets.handle(), idx, true);
```

When `PresetDatabase` is active, category/search results are database-global indices. `projectm_playlist_set_position()` expects a live playlist position. These may diverge after blacklist/quarantine, different scan ordering, category filtering, or future curation.

T02 added:

- `MenuOverlay::selectedPresetPath()` — returns the exact clicked full path captured during selection, with legacy fallback for older selection paths.
- `findPlaylistPositionByPath()` — reconciles the selected full path against the live projectM playlist before calling `projectm_playlist_set_position()`.
- `[PresetBrowser] WARNING` log and user toast when selected path cannot be reconciled, avoiding silent wrong-preset playback.

T04 inline review found and fixed an edge case: when `PresetDatabase` was loaded but the browser was showing `All Presets` from the flat list, a numeric index could still have been interpreted through the database. The browser now stores `m_selectedPresetPath` at click time so playback uses the actual clicked row's path. The category-mode fallback now also bounds-checks the flat-list fallback before using it.

**Invariant after T02:** `PlayPreset` may use a browser/database index to locate metadata, but it must use full path reconciliation to locate the live playlist position.

### 2. Favorites mostly use path persistence, but selected playback does not

Evidence:

- `favoritePresetPaths` persists full paths in config.
- `MenuOverlay::toggleFavorite()` writes full paths when `PresetDatabase` is loaded.
- `main.cpp` repopulates favorites from `favoritePresetPaths` with an O(n) scan.

This matches the project rule. Playback should follow the same path-based approach.

### 3. Validation/quarantine status is in-app discoverable — RESOLVED

Evidence before T03:

- Startup logs identified validation outcomes and `preset_validation.log`.
- `PresetManager::saveBrokenPresetLog()` wrote restore instructions.
- Advanced settings tooltip mentioned `broken_presets.txt`, but not the full user-data location, validation log, quarantine folder, or current enabled/skipped meaning.

T03 added visible status text under Settings → Advanced → Preset Management:

- validation enabled/disabled for next startup
- non-destructive explanation: broken presets are blacklisted and moved to reversible quarantine when possible
- actual user-data paths for `broken_presets.txt`, `broken_presets_quarantine/`, and `preset_validation.log` when available
- fallback `%APPDATA%\\Vibeus` explanation when the path is not set

This uses existing validation/quarantine files and does not add a second curation source.

### 4. Validation/quarantine implementation is best-effort and non-destructive

Evidence:

- Quarantine directory is under user data.
- Move failures log warnings and continue.
- Blacklist file persists broken presets even if moving fails.
- Files are moved, not deleted.

This supports S02 requirements. T03 should make that visible, not redesign it.

### 5. Preset browser already has useful curation primitives

Evidence:

- Category metadata and counts via `PresetDatabase`.
- Search bar.
- Favorites Only filter.
- Add All / Clear All within current category.
- Parsed display name/author/tag data exists in `PresetEntry`.

S02 should improve trust/status and path correctness, not build playlists/flows.

## Required Fixes

| Priority | Fix | Rationale | Target Task |
|---|---|---|---|
| P0 | Path-based browser playback bridge | Prevents playing the wrong preset when database index != playlist index. | T02 — DONE |
| P1 | Failed path reconciliation warning/toast/log | Avoid silent wrong behavior and support diagnosis. | T02 — DONE |
| P1 | Validation/quarantine status text in Settings or Browser | Makes curation understandable without file inspection. | T03 — DONE |
| P2 | Contract/build/review evidence | Supports M002 final integration. | T04 — DONE |

## Out of Scope

- User-created ordered playlists/flows.
- Import/export shareable flow JSON.
- Full quarantine management UI with restore buttons.
- New preset curation database or rating system.
- Permanent deletion of broken presets.

## Final Verification Evidence

```text
Command: cmake --build Vibeus/build --config Release
Exit: 0
menu_overlay.cpp
Vibeus.vcxproj -> F:\chilltittiesvisualizer\Vibeus\build\Release\Vibeus.exe
Copying projectM DLLs to output directory
Copying Milkdrop texture pack to output directory
```

Static audit: `.gsd/exec/3bf52df2-5ddc-4687-899a-db191a8983ae.stdout`

## S02 Conclusion

S02 passes. Browser playback now uses full-path reconciliation before touching the live projectM playlist, preventing category/database indices from being mistaken for playlist positions. Validation/quarantine status is visible in Settings with AppData paths and non-destructive restore semantics. The Release build succeeds after the final defensive path-selection fixes. Downstream slices S03 and S04 can consume this preset-trust baseline.
