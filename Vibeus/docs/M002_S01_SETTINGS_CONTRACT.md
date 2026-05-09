# M002 S01 — Settings Contract Inventory

**Milestone:** M002 Playable Product Closure  
**Slice:** S01 Runtime Settings Audit & Hardening  
**Task:** T01 Settings Contract Inventory  
**Generated:** 2026-05-08

## Purpose

M002/S01 must prove that user-facing runtime settings are real: they are persisted, reachable in the UI or intentionally hidden, and wired into runtime behavior through the project pattern `VibeusConfig → JSON → MenuOverlay → applyConfig()`.

This inventory is based on static inspection of:

- `Vibeus/src/config.h`
- `Vibeus/src/config.cpp`
- `Vibeus/src/menu_overlay.cpp`
- `Vibeus/src/main.cpp`
- `Vibeus/docs/SETTINGS_ARCHITECTURE.md`
- `Vibeus/SETTINGS_OVERHAUL_LOG.md`
- `Vibeus/TOUCH_REMOVAL_LOG.md`

## Inventory Summary

| Setting | Config | JSON | UI | Runtime | Evidence / Notes | Status |
|---|---:|---:|---:|---:|---|---|
| `audioGain` | yes | yes | yes | yes | Settings slider updates `g_audioGain`; audio feed uses gain. | PASS |
| `beatSensitivity` | yes | yes | yes | yes | UI slider; `applyConfig()` routes through `setBeatSensitivity()` with flash limiter clamp. | PASS |
| `beatReactivity` | yes | yes | yes | yes | UI slider; consumed by storyteller path via `g_config`. | PASS |
| `adaptiveBeat` | yes | yes | yes | yes | UI checkbox; runtime loop uses fixed sensitivity override when false. | PASS |
| `beatHoldTime` | yes | yes | yes | yes | UI slider; runtime beat/onset gating references `g_config`. | PASS |
| `autoAdvance` | yes | yes | yes | yes | UI checkbox; `applyConfig()` locks preset when auto-advance is disabled and gates hard cuts. | PASS |
| `presetDuration` | yes | yes | yes | yes | UI slider; `projectm_set_preset_duration()`. | PASS |
| `shuffle` | yes | yes | yes | yes | UI checkbox/hotkey; `applyConfig()` syncs PresetManager shuffle. | PASS |
| `transitionTime` | yes | yes | yes | yes | UI slider; `projectm_set_soft_cut_duration()`. | PASS |
| `hardCutEnabled` | yes | yes | yes | yes | UI checkbox; `projectm_set_hard_cut_enabled()`. | PASS |
| `hardCutSensitivity` | yes | yes | yes | yes | UI slider; `projectm_set_hard_cut_sensitivity()`. | PASS |
| `hardCutDuration` | yes | yes | yes | yes | UI slider; `projectm_set_hard_cut_duration()`. | PASS |
| `easterEgg` | yes | yes | yes | yes | UI slider; `projectm_set_easter_egg()`. | PASS |
| `fullscreen` | yes | yes | yes | yes | UI checkbox/hotkey; `toggleFullscreen()` through `applyConfig()` and startup handling. | PASS |
| `showFps` | yes | yes | yes | yes | UI checkbox/hotkey; controls `g_debug` and window title. | PASS |
| `perfMode` | yes | yes | yes | yes | UI combo sets baseline mesh; `applyConfig()` controls VSync. | PASS |
| `meshDetail` | yes | yes | yes | yes | UI slider; `projectm_set_mesh_size()`. | PASS |
| `aspectCorrection` | yes | yes | yes | yes | UI checkbox; `projectm_set_aspect_correction()`. | PASS |
| `flashLimiter` | yes | yes | yes | yes | UI checkbox; clamps beat sensitivity in `applyConfig()` and sets `g_flashLimiter`. | PASS |
| `reducedMotion` | yes | yes | yes | yes | UI checkbox; copied to `g_reducedMotion`; `updateVirtualTime()` applies a 0.5x factor when enabled. | PASS |
| `fontScale` | yes | yes | yes | yes | UI slider; `ImGui::GetIO().FontGlobalScale`. | PASS |
| `speedMultiplier` | yes | yes | yes | yes | UI slider/hotkeys; copied to `g_speedMultiplier`. | PASS |
| `gamepadDeadzone` | yes | yes | yes | yes | UI slider; `stickAxis()` uses `g_config.gamepadDeadzone`. | PASS |
| `storytellingEnabled` | yes | yes | yes | yes | UI checkbox; runtime loop gates storyteller. | PASS |
| `validatePresetsOnStartup` | yes | yes | yes | yes | UI checkbox; startup validation branch uses config before validation. | PASS |
| `overlayOpacity` | yes | yes | yes | indirect | UI slider; consumed directly by `MenuOverlay::renderSettings()` as panel backdrop opacity. This intentionally bypasses `applyConfig()` because it is local overlay rendering state, not a projectM/SDL side effect. Documented as local UI state. | PASS-DIRECT |
| `flowMode` | yes | yes | no | disabled | Persisted and copied to `g_flowMode`, but touch/flow mode was deliberately disabled in `TOUCH_REMOVAL_LOG.md` via `false` guards and commented UI. It is not currently user-facing. | OUT-OF-SCOPE |
| `touchEnabled` | yes | yes | no | disabled | Persisted but deliberately disabled in dev builds; not part of M002 runtime settings claim. | OUT-OF-SCOPE |
| `stasisEnabled` | yes | yes | yes | yes | Runtime loop uses `g_config.stasisEnabled`; Settings → Visuals → Accessibility exposes `Freeze on Silence`; Live Status shows active stasis. | PASS |
| `stasisThreshold` | yes | yes | yes | yes | Runtime loop uses threshold; Settings exposes `Silence Threshold`; stasis enter/exit logs include threshold. | PASS |
| `stasisFadeTime` | yes | yes | yes | yes | Runtime loop uses fade time; Settings exposes `Stasis Delay`; stasis enter log includes delay. | PASS |

## T01 Gaps and T02 Resolution

### 1. Stasis settings were persisted/runtime but not user-facing — RESOLVED

`stasisEnabled`, `stasisThreshold`, and `stasisFadeTime` are part of the M002 safety/low-audio behavior story. T01 found that they existed in config and runtime but were not exposed in the Settings UI.

T02 added a **Silence / Stasis** section under **Visuals → Accessibility**:

- `Freeze on Silence` checkbox → `stasisEnabled`
- `Silence Threshold` slider → `stasisThreshold`
- `Stasis Delay` slider → `stasisFadeTime`

The existing Live Status indicator still shows `STASIS — Visuals frozen (no audio)`, and `main.cpp` now logs stasis enter/exit events with RMS, threshold, and delay context.

### 2. `reducedMotion` runtime impact needed verification — RESOLVED

`reducedMotion` is persisted, user-facing, copied to `g_reducedMotion`, and used by `updateVirtualTime()` to apply a 0.5x animation-speed factor when enabled.

### 3. Flow/touch mode remains explicitly out of scope

`flowMode` and `touchEnabled` are persisted but deliberately disabled. Because old launch copy lists Flow Mode as a competitive advantage, M002 must not accidentally claim it as verified unless it is re-enabled and tested. Current M002 treatment: **out of scope / disabled dev feature**.

## Verification Evidence

Static inventory script output saved at:

- `.gsd/exec/5113675e-fc39-4a50-8310-c7bb65b42d60.stdout`

Key findings from the script:

```text
Potential gaps:
- flowMode: json=true, ui=false, runtime=true
- stasisEnabled: json=true, ui=false, runtime=true
- stasisThreshold: json=true, ui=false, runtime=true
- stasisFadeTime: json=true, ui=false, runtime=true
- overlayOpacity: json=true, ui=true, runtime=false
```

Manual interpretation refined those as:

- `flowMode` / `touchEnabled`: intentionally disabled and not M002 user-facing.
- `overlayOpacity`: direct local UI rendering state, not an `applyConfig()` side effect.
- Stasis settings: real gap because runtime uses them and M002 safety/low-audio behavior should be understandable.

## T02 Hardening Applied

### Stasis controls added

T02 added a **Silence / Stasis** section under Settings → Visuals → Accessibility:

- `Freeze on Silence` checkbox → `stasisEnabled`
- `Silence Threshold` slider → `stasisThreshold`
- `Stasis Delay` slider → `stasisFadeTime`

These settings now follow the project pattern:

- `VibeusConfig` fields already existed.
- JSON load/save already existed.
- `MenuOverlay::renderSettings()` now exposes the controls.
- `main.cpp` already uses the values in the visualizer loop.
- `main.cpp` now logs stasis enter/exit events with RMS, threshold, and delay context.
- `main.cpp` holds projectM frame time steady while `g_inStasis` is active so the visualizer actually freezes instead of only reducing beat sensitivity.
- `setBeatSensitivity()` allows exact `0.0` only for the `stasis` reason, and the non-adaptive fixed-sensitivity override is skipped while stasis is active.

### Reduced Motion verified

`reducedMotion` is not a dead control. It is copied to `g_reducedMotion` in `applyConfig()` and used by `updateVirtualTime()` to apply a 0.5x animation-speed factor.

## Final S01 Verification

### Command-level build evidence

```text
Command: cmake --build Vibeus/build --config Release
Exit: 0
Evidence:
  storyteller.cpp
  Vibeus.vcxproj -> F:\chilltittiesvisualizer\Vibeus\build\Release\Vibeus.exe
  Copying projectM DLLs to output directory
  Copying Milkdrop texture pack to output directory
```

The final build was rerun after review-driven fixes to the stasis runtime path and storyteller diagnostic logging.

### Static contract status

- All M002-relevant user-facing settings are now PASS or PASS-DIRECT.
- `flowMode` and `touchEnabled` remain explicitly OUT-OF-SCOPE because touch/flow functionality is deliberately disabled in the current dev build.
- `overlayOpacity` remains PASS-DIRECT because it is consumed inside `MenuOverlay::renderSettings()` as local UI rendering state, not an `applyConfig()` side effect.
- `reducedMotion` is verified through `updateVirtualTime()` and `g_reducedMotion`.
- Stasis settings are exposed in Settings → Visuals → Accessibility and observable through both Live Status and `[Stasis]` logs.
- Stasis now holds projectM frame time steady and suppresses beat sensitivity to exactly zero without being overwritten by non-adaptive fixed sensitivity.
- Storyteller diagnostic `[StoryDiag]` logs are gated behind `storyDebug` so normal app logs are not flooded.

### Manual/UAT notes for final milestone integration

- Human visual judgment is still needed later for whether stasis timing and reduced-motion feel good, but the controls are no longer dead or JSON-only.
- S04 should include a runtime path that opens Settings, verifies the stasis controls are reachable, and observes Live Status or log evidence when audio is silent.

## T01/T02/T03 Conclusion

S01 passes. The core settings architecture is intact, the main hidden stasis gap has been fixed, and the Release build succeeds after the changes. Downstream slices can consume this settings/runtime baseline.
