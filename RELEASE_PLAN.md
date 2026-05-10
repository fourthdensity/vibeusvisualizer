# Vibeus - SCRUM Release Plan
**Target:** Steam Early Access → Full Release  
**Team Structure:** Solo Developer + AI Pair Programming  
**Planning Methodology:** Agile SCRUM (2-week sprints)  
**Last Updated:** 2026-03-26

---

## 📊 Project Status

**Current State:** ✅ Functional MVP
- Core visualizer works with 9,797 presets
- WASAPI audio capture functional
- Basic controls (keyboard + gamepad)
- Settings menu exists but needs polish
- projectM integration stable

**Pain Points to Address:**
- ❌ Beat detection feels off-sync with music
- ❌ Transitions are jarring (hard cuts too frequent)
- ❌ No way to favorite or organize presets
- ❌ Some settings don't actually work
- ❌ UI/UX needs polish for Steam release

---

## 🎯 Release Milestones

### **Milestone 1: Core Polish** (Target: 6 weeks)
**Goal:** Make the existing experience feel professional and responsive  
**Definition of Done:** Software feels "Steam-ready" for early access

### **Milestone 2: User Customization** (Target: +4 weeks)
**Goal:** Users can personalize their experience (favorites, playlists, uploads)  
**Definition of Done:** Users can curate their own visual collections

### **Milestone 3: Steam Integration** (Target: +3 weeks)
**Goal:** Steam SDK, achievements, cloud saves, Steam Deck verified  
**Definition of Done:** Listed on Steam Early Access

### **Milestone 4: Community & Polish** (Target: +4 weeks)
**Goal:** Workshop support, user feedback iteration, performance optimization  
**Definition of Done:** Ready for full Steam release (v1.0)

**Total Timeline:** ~17 weeks (4 months) for full release

---

## 📦 Product Backlog (Prioritized Epics)

### **EPIC 1: Beat Detection & Audio Reactivity** 🎵
**User Story:** *As a user, I want the visuals to feel perfectly in sync with my music so the experience is immersive.*

**Priority:** ⭐⭐⭐⭐⭐ CRITICAL (Milestone 1)

#### User Stories:
1. **Accurate Beat Detection**
   - As a user, I want beats to trigger visual events reliably
   - **Acceptance Criteria:**
     - Test with 10+ songs across genres (EDM, rock, hip-hop, classical)
     - Visual "pulse" aligns with bass drum/kick within 50ms
     - User can adjust sensitivity and see immediate effect
   - **Implementation:** Tune `projectm_set_beat_sensitivity()`, add debug beat indicator
   - **Effort:** 3 story points (1-2 days)

2. **Genre-Specific Audio Profiles**
   - As a user, I want presets for different music types (bass-heavy EDM vs acoustic)
   - **Acceptance Criteria:**
     - Dropdown with "EDM", "Rock", "Classical", "Custom" profiles
     - Each profile adjusts bass/treble weighting
     - Changes persist in config.json
   - **Implementation:** Add frequency weighting in audio processing pipeline
   - **Effort:** 5 story points (2-3 days)

3. **Visual Beat Feedback**
   - As a user, I want to see when beats are detected to verify sync
   - **Acceptance Criteria:**
     - Debug overlay shows beat indicator (red flash or pulse icon)
     - Toggle on/off with keyboard shortcut
     - Helps users tune sensitivity for their music
   - **Implementation:** Add beat event listener, render indicator in debug overlay
   - **Effort:** 2 story points (1 day)

**Epic Total:** 10 story points (~1 week)

---

### **EPIC 2: Smooth Transitions & Flow** 🌊
**User Story:** *As a user, I want seamless transitions between presets so the experience feels like a continuous journey, not a slideshow.*

**Priority:** ⭐⭐⭐⭐⭐ CRITICAL (Milestone 1)

#### User Stories:
1. **Smooth Blend Transitions**
   - As a user, I want presets to fade/blend into each other naturally
   - **Acceptance Criteria:**
     - Transition duration slider (0s = instant, 10s = long fade)
     - Alpha blend or crossfade between outgoing and incoming preset
     - No black flashes or glitches during transition
   - **Implementation:** Use `projectm_set_soft_cut_duration()`, test with variety of presets
   - **Effort:** 3 story points (1-2 days)

2. **Smart Hard Cut Logic**
   - As a user, I want hard cuts to happen on intense musical moments, not randomly
   - **Acceptance Criteria:**
     - Hard cuts trigger on detected beats or high energy moments
     - Probability adjustable (0% = never, 100% = always on beat)
     - Feels intentional, not jarring
   - **Implementation:** Hook hard cut probability to beat detection energy level
   - **Effort:** 5 story points (2-3 days)

3. **Preset Duration Variety**
   - As a user, I want some presets to linger longer than others for variety
   - **Acceptance Criteria:**
     - Easter egg slider (0.0 = fixed duration, 1.0 = random variety)
     - Some presets stay for 10s, others for 60s+ organically
     - User can still manually skip with arrow keys
   - **Implementation:** Use `projectm_set_easter_egg()` API
   - **Effort:** 2 story points (1 day)

4. **Transition Effect Types**
   - As a user, I want different transition styles (fade, wipe, morph)
   - **Acceptance Criteria:**
     - Dropdown: "Auto", "Fade", "Hard Cut", "Blend", "Wipe"
     - Each style visually distinct
     - Documented in help/tutorial
   - **Implementation:** Explore projectM transition API, may require shader work
   - **Effort:** 8 story points (3-4 days) - **RESEARCH NEEDED**

**Epic Total:** 18 story points (~1.5 weeks)

---

### **EPIC 3: Settings Overhaul** ⚙️
**User Story:** *As a user, I want every setting to visibly affect the visualization so I can customize my experience.*

**Priority:** ⭐⭐⭐⭐ HIGH (Milestone 1)

#### User Stories:
1. **Fix Broken Settings**
   - As a user, I expect settings to work when I change them
   - **Acceptance Criteria:**
     - `gamepadDeadzone` actually used in stick normalization
     - `perfMode` switches mesh size and VSync correctly
     - `uiScale` removed (doesn't work, causes confusion)
   - **Implementation:** See `OVERHAUL_MASTER_PLAN.md` Phase 1
   - **Effort:** 3 story points (1-2 days)

2. **Visual Detail Control**
   - As a user, I want to trade visual quality for performance
   - **Acceptance Criteria:**
     - Slider: "Visual Detail" (32-128 mesh size)
     - Changes apply immediately without restart
     - Visible difference between low and high settings
   - **Implementation:** Add mesh size slider in Advanced tab
   - **Effort:** 2 story points (1 day)

3. **Performance Profiles**
   - As a user, I want one-click presets for battery saving or max quality
   - **Acceptance Criteria:**
     - Dropdown: "Battery Saver", "Balanced", "Quality"
     - Each profile bundles mesh, VSync, FPS settings
     - Steam Deck profile auto-detected
   - **Implementation:** See `OVERHAUL_MASTER_PLAN.md` Phase 1.3
   - **Effort:** 5 story points (2-3 days)

4. **Reorganize Settings Tabs**
   - As a user, I want settings grouped logically so I can find them easily
   - **Acceptance Criteria:**
     - Tabs: "Vibes" (audio/speed), "Presets" (duration/transitions), "Display" (fullscreen/UI), "Advanced" (perf/technical)
     - Most common settings in first two tabs
     - Layout fits 1280×800 (Steam Deck) without scrolling
   - **Implementation:** Refactor `menu_overlay.cpp` tab structure
   - **Effort:** 5 story points (2-3 days)

**Epic Total:** 15 story points (~1 week)

---

### **EPIC 4: Preset Management** 📚
**User Story:** *As a user, I want to organize and curate my favorite presets so I can revisit the visuals I love.*

**Priority:** ⭐⭐⭐⭐ HIGH (Milestone 2)

#### User Stories:
1. **Category Browser**
   - As a user, I want to browse presets by visual style (Fractal, Dancer, Reaction, etc.)
   - **Acceptance Criteria:**
     - UI shows 11 categories with preset counts
     - Clicking category filters to only those presets
     - "Back to All" button returns to full list
   - **Implementation:** Integrate `preset_database.cpp`, add category UI
   - **Effort:** 8 story points (3-4 days)

2. **Favorites System**
   - As a user, I want to star presets I love so I can find them later
   - **Acceptance Criteria:**
     - Star icon next to preset name (click to toggle)
     - "Favorites" tab shows only starred presets
     - Favorites persist in config.json
     - Can favorite/unfavorite during playback
   - **Implementation:** Add favorites array to config, UI toggle, filter logic
   - **Effort:** 5 story points (2-3 days)

3. **Custom Playlists**
   - As a user, I want to create named playlists (e.g., "Party", "Chill", "Trippy")
   - **Acceptance Criteria:**
     - Create/rename/delete playlists
     - Add/remove presets to playlists (multi-select or drag-drop)
     - Shuffle/random scoped to active playlist
     - Playlists persist in config.json
   - **Implementation:** Playlist UI in menu, playlist selection logic
   - **Effort:** 13 story points (5-6 days)

4. **Upload Custom Presets**
   - As a user, I want to import my own `.milk` files from the community
   - **Acceptance Criteria:**
     - "Import Preset" button opens file picker
     - Validates `.milk` format (basic sanity check)
     - Copies to `%APPDATA%/Vibeus/custom_presets/`
     - Appears in "Custom" category immediately
   - **Implementation:** File picker dialog, preset validation, database refresh
   - **Effort:** 5 story points (2-3 days)

5. **Preset Search & Filtering**
   - As a user, I want to search presets by name or author
   - **Acceptance Criteria:**
     - Search box filters preset list in real-time
     - Can combine search with category filter
     - Highlights matching text
   - **Implementation:** Text input + filter logic on preset list
   - **Effort:** 3 story points (1-2 days)

**Epic Total:** 34 story points (~2.5 weeks)

---

### **EPIC 5: UI/UX Polish** 🎨
**User Story:** *As a user, I want the interface to feel modern, intuitive, and visually appealing.*

**Priority:** ⭐⭐⭐ MEDIUM (Milestone 2)

#### User Stories:
1. **ImGui Theme & Styling**
   - As a user, I want the menu to look professional, not like a debug UI
   - **Acceptance Criteria:**
     - Custom color scheme (matches visualizer aesthetic)
     - Rounded buttons, smooth animations
     - Consistent padding and spacing
     - Dark theme with neon accents
   - **Implementation:** Apply ImGui style overrides, custom colors
   - **Effort:** 5 story points (2-3 days)

2. **Preset Metadata Display**
   - As a user, I want to see who created the current preset to credit artists
   - **Acceptance Criteria:**
     - Shows "Preset: [name] by [author]" on screen (toggleable)
     - Credits screen lists top preset contributors
     - Respects author attribution from `.milk` files
   - **Implementation:** Parse author from filename, render overlay text
   - **Effort:** 3 story points (1-2 days)

3. **Onboarding Tutorial**
   - As a new user, I want to learn controls without reading a manual
   - **Acceptance Criteria:**
     - First-run overlay shows basic controls (arrow keys, menu, fullscreen)
     - Dismissible with "Got it" button
     - Can replay from Help menu
   - **Implementation:** Overlay UI, first-run detection in config
   - **Effort:** 3 story points (1-2 days)

4. **Improved Debug Overlay**
   - As a power user, I want detailed performance metrics
   - **Acceptance Criteria:**
     - FPS, frame time, audio buffer stats
     - Beat detection indicator (see Epic 1)
     - GPU memory usage (if available)
     - Preset render time
   - **Implementation:** Enhance existing debug overlay with more metrics
   - **Effort:** 3 story points (1-2 days)

**Epic Total:** 14 story points (~1 week)

---

### **EPIC 6: Steam Integration** 🎮
**User Story:** *As a Steam user, I want full platform integration (achievements, cloud saves, Steam Deck support).*

**Priority:** ⭐⭐⭐ MEDIUM (Milestone 3)

#### User Stories:
1. **Steam SDK Integration**
   - As a developer, I want the game to authenticate with Steam
   - **Acceptance Criteria:**
     - Steam SDK initialized on launch
     - Steam overlay works in-game
     - Graceful fallback if launched outside Steam
   - **Implementation:** Add Steamworks SDK, init in main.cpp
   - **Effort:** 8 story points (3-4 days)

2. **Achievements**
   - As a user, I want to unlock achievements for milestones
   - **Acceptance Criteria:**
     - 10-15 achievements (e.g., "100 presets viewed", "10 favorites", "1 hour playtime")
     - Toast notifications when unlocked
     - Trackable progress (e.g., "50/100 presets viewed")
   - **Implementation:** Define achievements, hook events, call Steam API
   - **Effort:** 5 story points (2-3 days)

3. **Cloud Saves**
   - As a user, I want my favorites and settings synced across devices
   - **Acceptance Criteria:**
     - config.json, favorites, playlists saved to Steam Cloud
     - Auto-sync on launch/exit
     - Manual "Sync Now" button in settings
   - **Implementation:** Steam Cloud API for config file sync
   - **Effort:** 5 story points (2-3 days)

4. **Steam Deck Optimization**
   - As a Steam Deck user, I want optimized performance and controls
   - **Acceptance Criteria:**
     - Auto-detects Steam Deck, applies Battery Saver profile
     - All controls work with gamepad + touch
     - Verified checkmark on Steam store
     - 40 FPS stable, 2+ hours battery
   - **Implementation:** Platform detection, performance testing
   - **Effort:** 8 story points (3-4 days)

5. **Steam Input API**
   - As a user, I want to remap controls via Steam settings
   - **Acceptance Criteria:**
     - All actions exposed to Steam Input configurator
     - Default profile ships with game
     - Works with any controller
   - **Implementation:** Replace SDL gamepad with Steam Input API
   - **Effort:** 8 story points (3-4 days)

**Epic Total:** 34 story points (~2.5 weeks)

---

### **EPIC 7: Performance & Stability** 🔧
**User Story:** *As a user, I want the app to run smoothly without crashes or stutters.*

**Priority:** ⭐⭐ LOW (Continuous, Milestone 4)

#### User Stories:
1. **Preset Crash Quarantine**
   - As a user, I don't want the app to crash on buggy presets
   - **Acceptance Criteria:**
     - Detect preset crashes (OpenGL errors, infinite loops)
     - Auto-skip crashing presets
     - Log to quarantine list
     - User can view/remove from quarantine
   - **Implementation:** Exception handling, preset blacklist system
   - **Effort:** 8 story points (3-4 days)

2. **Memory Leak Detection**
   - As a user, I want the app to run for hours without slowing down
   - **Acceptance Criteria:**
     - No memory growth over 2+ hour test sessions
     - Profile with Visual Studio diagnostic tools
     - Fix any identified leaks
   - **Implementation:** Memory profiling, leak fixing
   - **Effort:** 8 story points (3-4 days)

3. **GPU Compatibility Testing**
   - As a user, I want the app to work on different GPUs (AMD, NVIDIA, Intel)
   - **Acceptance Criteria:**
     - Test on 3+ GPU vendors
     - Document minimum requirements (OpenGL 3.3+)
     - Graceful error messages for unsupported hardware
   - **Implementation:** Hardware testing, error handling
   - **Effort:** 5 story points (2-3 days)

**Epic Total:** 21 story points (~1.5 weeks)

---

## 🗓️ Sprint Structure (2-Week Sprints)

### Sprint Planning Template
**Solo Dev + AI Workflow:**
- **Sprint Capacity:** 20-25 story points (assuming 5-6 focused dev days per week)
- **Daily Standup:** Self-reflection (5 min journaling)
  - What did I complete yesterday?
  - What will I work on today?
  - Any blockers? (Use AI to unblock)
- **Sprint Review:** End-of-sprint demo video (record for dev log)
- **Sprint Retrospective:** What went well? What to improve? (15 min notes)

### Velocity Tracking
- **Story Points = Complexity** (Fibonacci: 1, 2, 3, 5, 8, 13, 21)
  - 1-2 points: ~1 day or less
  - 3-5 points: 2-3 days
  - 8 points: 3-4 days
  - 13+ points: Break down further (too large)

---

## 📅 Suggested Sprint Breakdown

### **Sprint 1-3: Core Polish (Milestone 1)**

#### **Sprint 1 (Weeks 1-2): Beat Detection & Audio**
**Goal:** Make visuals feel in sync with music  
**Stories:**
- [ ] Epic 1.1: Accurate Beat Detection (3 pts)
- [ ] Epic 1.2: Genre-Specific Audio Profiles (5 pts)
- [ ] Epic 1.3: Visual Beat Feedback (2 pts)
- [ ] Epic 3.1: Fix Broken Settings (3 pts)
- [ ] Epic 3.2: Visual Detail Control (2 pts)
- **Total:** 15 points (manageable for first sprint)

#### **Sprint 2 (Weeks 3-4): Smooth Transitions**
**Goal:** Make preset changes feel natural  
**Stories:**
- [ ] Epic 2.1: Smooth Blend Transitions (3 pts)
- [ ] Epic 2.2: Smart Hard Cut Logic (5 pts)
- [ ] Epic 2.3: Preset Duration Variety (2 pts)
- [ ] Epic 3.3: Performance Profiles (5 pts)
- [ ] Epic 3.4: Reorganize Settings Tabs (5 pts)
- **Total:** 20 points

#### **Sprint 3 (Weeks 5-6): Transition Effects Research**
**Goal:** Explore advanced transition possibilities  
**Stories:**
- [ ] Epic 2.4: Transition Effect Types (8 pts) - **SPIKE: Research projectM API**
- [ ] Epic 5.2: Preset Metadata Display (3 pts)
- [ ] Epic 5.4: Improved Debug Overlay (3 pts)
- [ ] Epic 7.3: GPU Compatibility Testing (5 pts)
- **Total:** 19 points

**Milestone 1 Review:** Is the core experience Steam-ready?

---

### **Sprint 4-6: User Customization (Milestone 2)**

#### **Sprint 4 (Weeks 7-8): Preset Organization**
**Goal:** Users can find and organize presets  
**Stories:**
- [ ] Epic 4.1: Category Browser (8 pts)
- [ ] Epic 4.2: Favorites System (5 pts)
- [ ] Epic 4.5: Preset Search & Filtering (3 pts)
- **Total:** 16 points

#### **Sprint 5 (Weeks 9-10): Custom Playlists**
**Goal:** Users can create personal collections  
**Stories:**
- [ ] Epic 4.3: Custom Playlists (13 pts) - **MAJOR FEATURE**
- [ ] Epic 5.1: ImGui Theme & Styling (5 pts)
- **Total:** 18 points

#### **Sprint 6 (Weeks 11-12): Polish & Upload**
**Goal:** Complete customization features  
**Stories:**
- [ ] Epic 4.4: Upload Custom Presets (5 pts)
- [ ] Epic 5.3: Onboarding Tutorial (3 pts)
- [ ] Epic 7.1: Preset Crash Quarantine (8 pts)
- **Total:** 16 points

**Milestone 2 Review:** Can users fully customize their experience?

---

### **Sprint 7-8: Steam Integration (Milestone 3)**

#### **Sprint 7 (Weeks 13-14): Steam SDK & Deck**
**Goal:** Basic Steam integration working  
**Stories:**
- [ ] Epic 6.1: Steam SDK Integration (8 pts)
- [ ] Epic 6.3: Cloud Saves (5 pts)
- [ ] Epic 6.4: Steam Deck Optimization (8 pts)
- **Total:** 21 points

#### **Sprint 8 (Weeks 15-16): Achievements & Input**
**Goal:** Full Steam feature parity  
**Stories:**
- [ ] Epic 6.2: Achievements (5 pts)
- [ ] Epic 6.5: Steam Input API (8 pts)
- [ ] Epic 7.2: Memory Leak Detection (8 pts)
- **Total:** 21 points

**Milestone 3 Review:** Ready for Early Access launch?

---

### **Sprint 9 (Week 17+): Community & Final Polish (Milestone 4)**
**Goal:** Iterate on user feedback, prepare v1.0  
**Stories:**
- [ ] Fix bugs from Early Access feedback (TBD)
- [ ] Steam Workshop integration (if planned)
- [ ] Performance optimization based on telemetry
- [ ] Marketing materials (trailer, screenshots)

**Total Timeline:** ~17 weeks (4 months) to Full Release

---

## ✅ Definition of Done

### For User Stories:
- [ ] Code implemented and tested manually
- [ ] Works on developer machine (Windows 10/11)
- [ ] Settings persist across restarts
- [ ] No new crashes or critical bugs introduced
- [ ] UI changes fit in 1280×800 resolution (Steam Deck)
- [ ] Performance: 60 FPS stable on reference hardware
- [ ] AI code review passed (use `/model claude-opus-4.5` for major changes)

### For Epics:
- [ ] All user stories completed
- [ ] Integration testing passed
- [ ] User acceptance criteria met
- [ ] Documented in copilot-instructions.md (if architectural)
- [ ] Demo video recorded (for portfolio/marketing)

### For Milestones:
- [ ] All epics in milestone complete
- [ ] Stakeholder review (self-review + optional friend testing)
- [ ] Regression testing: All previous features still work
- [ ] Release notes drafted
- [ ] Build deployed to test environment

---

## 🛠️ AI Pair Programming Workflow

### Using AI Effectively as Solo Dev

#### 1. **Planning Phase (Use AI for Analysis)**
```bash
# Use explore agents to understand codebase
/model claude-haiku-4.5  # Fast exploration
"Analyze how preset transitions work in main.cpp and preset_manager.cpp"

# Use Opus for architecture design
/model claude-opus-4.5
"Design a favorites system that integrates with existing preset_database.cpp"
```

#### 2. **Implementation Phase (Use AI for Coding)**
```bash
# Start implementation with clear context
"Implement the favorites star toggle UI in menu_overlay.cpp.
The favorites list should be stored in config.h as std::vector<std::string>.
Reference the existing preset browser code around line 450."

# Use heavy model for complex refactors
/model claude-opus-4.5
"Refactor audio capture to support platform abstraction for future mobile support"
```

#### 3. **Testing Phase (Use AI for Test Plans)**
```bash
"Generate a test plan for beat detection with 10 diverse songs.
Include expected behavior and pass/fail criteria."
```

#### 4. **Review Phase (Use AI for Code Review)**
```bash
# Use code-review agent
"Review the changes in preset_manager.cpp for memory leaks and performance issues"
```

### Best Practices:
- ✅ **Break down large features** into <8 point stories
- ✅ **Use AI for research spikes** before committing to implementation
- ✅ **Pair program complex logic** (describe approach, AI implements, you review)
- ✅ **Let AI write boilerplate** (JSON serialization, UI layouts)
- ✅ **You own architecture decisions** (AI provides options, you choose)
- ✅ **Test incrementally** (don't merge multiple stories without testing)

---

## 📈 Progress Tracking

### Burndown Chart (Manual Tracking)
Track remaining story points per sprint:
```
Sprint 1: 15 → 12 → 8 → 3 → 0 ✅ (on track)
Sprint 2: 20 → 18 → 15 → 10 → 4 → 0 ✅
Sprint 3: 19 → 16 → 10 → 8 → 2 ⚠️ (carried over 2 pts)
```

### Velocity Tracking
```
Sprint 1: 15 points completed
Sprint 2: 20 points completed
Sprint 3: 17 points completed (2 carried over)
Average velocity: 17-18 points/sprint
```

Use this to adjust future sprint planning.

---

## 🚨 Risk Management

### High-Risk Items:
1. **Epic 2.4: Transition Effect Types** (8 pts)
   - **Risk:** projectM API may not support custom transition shaders
   - **Mitigation:** Research spike in Sprint 3, may need to contribute to projectM upstream
   - **Fallback:** Stick with built-in fade/hard cut transitions

2. **Epic 6: Steam Integration** (34 pts total)
   - **Risk:** First-time Steam SDK integration, unknown unknowns
   - **Mitigation:** Allocate extra buffer time, consult Steamworks docs early
   - **Fallback:** Launch without Steam features on itch.io first

3. **Epic 7.1: Preset Crash Quarantine** (8 pts)
   - **Risk:** Some crashes may be in projectM core (harder to fix)
   - **Mitigation:** Report upstream, maintain quarantine list
   - **Fallback:** Accept that some presets will crash (document known issues)

### Dependencies:
- **projectM Library:** If upstream bugs block progress, may need to fork and patch
- **Steam Approval:** Early Access submission can take 1-2 weeks for review

---

## 📝 Documentation Requirements

### For Each Sprint:
- [ ] Update `RELEASE_NOTES.md` with completed features
- [ ] Update `README.md` controls table if new shortcuts added
- [ ] Update `.github/copilot-instructions.md` if architecture changed
- [ ] Record demo video (30-60 seconds) showing new features

### For Milestone Releases:
- [ ] Write detailed release notes (user-facing language)
- [ ] Update all screenshots/GIFs in README
- [ ] Update Steam store page (if applicable)
- [ ] Post devlog update (Twitter, Reddit, Discord)

---

## 🎬 Post-Release Plan (v1.0+)

### Ongoing Maintenance:
- **Bug fixes:** Priority patches within 48 hours
- **Community presets:** Curate best submissions for official pack
- **Steam Workshop:** Monthly featured preset collections
- **Performance:** Monitor crash reports, optimize hot paths

### Future Content Updates:
- **v1.1:** Additional audio reactivity modes (e.g., speech/vocal isolation)
- **v1.2:** VR support (separate major epic)
- **v2.0:** Mobile ports (Android/iOS)

---

## 🏁 Success Metrics

### Pre-Launch:
- [ ] 100+ hours personal testing
- [ ] 10+ friends/beta testers provide feedback
- [ ] 0 critical bugs in bug tracker
- [ ] 60 FPS average on GTX 1060 / RX 580 (mid-range GPUs)

### Post-Launch (Early Access):
- [ ] 95%+ positive Steam reviews
- [ ] <1% crash rate (telemetry)
- [ ] Average session length 30+ minutes
- [ ] 50%+ users create at least one favorite

### Full Release (v1.0):
- [ ] "Very Positive" Steam rating (80%+)
- [ ] Steam Deck Verified badge
- [ ] Featured in Steam "New & Trending" or "Top Sellers"
- [ ] Break even on development costs within 6 months

---

## 🤝 Community Engagement Plan

### Beta Testing (Sprint 6-7):
- Recruit 10-20 beta testers from Discord/Reddit
- Provide Steam keys or itch.io builds
- Collect feedback via Google Forms survey
- Weekly check-ins for bug reports

### Early Access Launch:
- Steam page: Emphasize "work in progress" nature
- Roadmap visible on store page
- Monthly dev update posts
- Discord community for direct feedback

### Marketing Channels:
- Reddit: r/visualizers, r/gamedev, r/SteamDeck
- YouTube: Demo videos, dev vlogs
- Twitter/X: Weekly progress updates with GIFs
- Steam Community: Announce features, engage with reviews

---

## 📚 Additional Resources

### Key Documents:
- `OVERHAUL_MASTER_PLAN.md` - Technical implementation details for settings
- `docs/PRESET_CATEGORIES_DESIGN.md` - Preset organization system spec
- `ROADMAP.md` - Long-term vision and competitive analysis
- `.github/copilot-instructions.md` - AI pair programming guide

### External References:
- projectM Wiki: https://github.com/projectM-visualizer/projectm/wiki
- Steamworks SDK Docs: https://partner.steamgames.com/doc/sdk
- Agile/SCRUM Guide: https://scrumguides.org/scrum-guide.html

---

**Next Steps:**
1. Review this plan and adjust timeline based on available dev time
2. Set up sprint tracking (Trello, Notion, or simple markdown checklist)
3. Start Sprint 1: Focus on beat detection and audio reactivity
4. Use AI pair programming for implementation (see workflow section)
5. Track velocity after first sprint to calibrate future estimates

**Remember:** This is a living document. Adjust based on learnings and feedback. Solo dev + AI is powerful—stay focused, iterate fast, and ship! 🚀
