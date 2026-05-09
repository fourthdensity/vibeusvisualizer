# Phase Plan: M1-SETTINGS — Settings Panel (HIGH PRIORITY)

**Phase:** 1 of Milestone 1 (v0.3.0 "Playable Product")
**Priority:** HIGH (addresses core user complaint: "NO controls")
**Dependencies:** None (builds on existing config + ImGui menu)
**Estimated Effort:** Medium (UI + persistence + integration)

## Discussion Summary (Autonomous)
- No blockers or grey areas.
- Existing: hotkey controls (Up/Down for beat sens, LB/RB for gain), partial config.json (load/save), g_config struct, applyConfig stub.
- Design docs available: docs/SETTINGS_DESIGN.md, SETTINGS_IMPLEMENTATION.md, SETTINGS_ARCHITECTURE.md, SETTINGS_OVERHAUL_LOG.md.
- Goal: Full ImGui settings panel in menu (tab or modal), sliders for Speed, Audio Gain, Beat Sensitivity, Transition Duration, Fullscreen toggle, Save/Load buttons, live apply where safe.
- Aligns with E3.1 "Fix Broken Settings" story in sprint backlog.

## Execution Plan (Tasks)
1. Extend config.h with missing fields (speed, transitionDuration, fullscreen, etc.) and defaults.
2. Update config.cpp load/save to persist new fields (nlohmann/json).
3. Implement Settings UI in menu_overlay.cpp (new ImGui window or tab in main menu, sliders + buttons).
4. Wire live updates in main loop (applyConfig on slider change, save on exit/apply).
5. Update main.cpp init + hotkey handling to respect config values.
6. Test: build, run --debug, verify sliders affect visuals immediately, persist across restart.
7. Update ROADMAP.md + STATE.md progress.

## Risks & Mitigations
- ImGui layout in glassmorphism style: follow existing menu_overlay patterns.
- Live apply for beat sens/speed: safe (projectm setters exist).
- Preset transition duration: hook into projectM soft/hard cut.

## Success Criteria
- Settings panel accessible from main/pause menu.
- All 4 core sliders + fullscreen work + persist.
- No regression on existing hotkeys or storyteller.
- Clean shutdown saves state.

## Artifacts to Produce
- Updated code in src/config.h/cpp, menu_overlay.cpp, main.cpp
- .planning/M1-SETTINGS-SUMMARY.md post-execution
- ROADMAP.md marked complete for this item
