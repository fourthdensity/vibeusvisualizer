# AI Agents Workflow Guide
**Multi-Provider AI Development Strategy for Vibeus**

This guide explains how to leverage multiple AI providers (GitHub Copilot, Google Gemini, Claude/Anthropic) working in parallel on complementary tasks to maximize development velocity and code quality.

---

## 🎯 Philosophy: Distributed AI Development

**Core Concept:** Different AI models have different strengths. By distributing tasks across providers based on their capabilities, you can work on multiple features simultaneously while maintaining high quality.

**Benefits:**
- ⚡ **Faster Development:** Multiple agents work in parallel
- 🎯 **Specialized Expertise:** Use each AI's strengths
- 🔄 **Cross-Validation:** Different models review each other's work
- 🧠 **Broader Perspective:** Multiple approaches to same problem
- 💪 **Reduced Bottlenecks:** No single AI becomes overloaded

---

## 🤖 AI Provider Strengths Matrix

### GitHub Copilot (In-Editor)
**Best For:**
- ✅ Real-time code completion while typing
- ✅ Boilerplate code generation (getters, setters, JSON serialization)
- ✅ Implementing well-defined functions (when you know what you want)
- ✅ Quick refactoring (rename, extract method)
- ✅ Inline documentation and comments

**Model:** GPT-4 (GitHub Copilot)  
**Speed:** ⚡⚡⚡ Instant  
**Context:** Current file + nearby files  
**Cost:** $10-20/month subscription

**Usage Pattern:**
```cpp
// You type:
void PresetManager::toggleFavorite(const std::string& presetPath) {
    // [Copilot completes implementation]
}
```

**When to Use:**
- Writing straightforward implementations
- Adding new methods to existing classes
- Filling in repetitive code patterns
- Quick inline fixes

---

### Google Gemini (Via API or Chat)
**Best For:**
- ✅ Fast codebase exploration and analysis
- ✅ Large-scale refactoring planning
- ✅ Generating test plans and test cases
- ✅ Documentation generation (README, API docs)
- ✅ Multi-file analysis (understanding relationships)
- ✅ Research and technical feasibility studies

**Models:**
- **Gemini 2.0 Flash** - Ultra-fast, good for exploration
- **Gemini 2.0 Pro** - Balanced, good for most tasks
- **Gemini 3.0 Pro** - Premium, best reasoning

**Speed:** ⚡⚡⚡ Very Fast (especially Flash)  
**Context:** Up to 2M tokens (can read entire codebase)  
**Cost:** Free tier available, then pay-per-token

**Usage Pattern:**
```bash
# Gemini Flash for exploration
"Analyze all files in Vibeus/src/ and explain the settings application flow.
Show me which files interact with config.json and how settings are applied."

# Gemini Pro for design
"Design a favorites system that integrates with preset_manager.cpp and config.h.
Provide a detailed implementation plan with file changes and method signatures."
```

**When to Use:**
- Initial codebase exploration (new to project)
- Understanding complex multi-file systems
- Generating comprehensive test plans
- Planning architectural changes before coding
- Researching projectM API capabilities

---

### Claude (Anthropic)
**Best For:**
- ✅ Deep reasoning and architectural design
- ✅ Complex debugging and problem-solving
- ✅ Code review and security audits
- ✅ Writing detailed documentation
- ✅ Multi-step planning and task breakdown
- ✅ Ethical considerations and edge case analysis

**Models:**
- **Claude Haiku 4.5** - Fast, good for simple tasks
- **Claude Sonnet 4.5** - Balanced, best for most work
- **Claude Opus 4.5/4.6** - Premium, deepest reasoning

**Speed:** ⚡⚡ Fast (Haiku), ⚡ Standard (Sonnet/Opus)  
**Context:** 200K tokens  
**Cost:** API usage (Haiku cheap, Opus expensive)

**Usage Pattern:**
```bash
# Claude Sonnet for implementation
"Implement a favorites system in preset_manager.cpp that:
1. Stores favorites in config.json as an array
2. Provides thread-safe toggleFavorite() method
3. Handles edge cases (deleted presets, corrupted config)
Include error handling and performance considerations."

# Claude Opus for architectural decisions
"Refactor the settings system to support:
1. Real-time validation
2. Settings presets (Battery Saver, Quality)
3. Platform-specific overrides (Steam Deck)
Design the abstraction layers and explain trade-offs."
```

**When to Use:**
- Critical features requiring deep thought
- Architectural refactoring (settings overhaul, preset database)
- Security-sensitive code (file uploads, preset validation)
- Complex algorithm design (beat detection tuning)
- Final code review before merge

---

## 🎭 Multi-Agent Workflow Patterns

### Pattern 1: Parallel Feature Development
**Scenario:** You have 3 independent features to implement

**Strategy:**
1. **Gemini Flash** → Explore codebase for Feature A (5 min)
2. **GitHub Copilot** → Implement Feature B (in-editor, 30 min)
3. **Claude Sonnet** → Design and implement Feature C (async, 45 min)

**Example:**
```
You (Morning):
├─ Gemini: "Analyze preset_manager.cpp - how does shuffle work?"
├─ Copilot: [You implement simple JSON serialization changes]
└─ Claude: "Design a playlist system with create/delete/add/remove"

You (Afternoon):
├─ Read Gemini's analysis
├─ Use insights to improve Copilot-written code
└─ Implement Claude's playlist design using Copilot for boilerplate
```

**Result:** 3 features progressed simultaneously

---

### Pattern 2: Depth-First Quality Pipeline
**Scenario:** One complex feature requiring multiple passes

**Strategy:**
1. **Gemini Pro** → Initial design and planning (fast iteration)
2. **GitHub Copilot** → Boilerplate implementation (in-editor)
3. **Claude Sonnet** → Complex logic and edge cases
4. **Claude Opus** → Final review and optimization

**Example: Implementing Beat Detection Tuning**
```
Step 1: Gemini Pro (15 min)
"How does projectM beat detection work? What parameters control it?
Generate a list of all beat-related API calls in libprojectM."

Step 2: GitHub Copilot (30 min)
// You write structure, Copilot fills in:
void applyBeatSettings(float sensitivity, float bassWeight) {
    // Copilot completes with projectM API calls
}

Step 3: Claude Sonnet (45 min)
"Implement genre-specific audio profiles (EDM, Rock, Classical).
Each profile should adjust bass/treble weighting in audio processing.
Handle edge cases: invalid frequencies, audio buffer underruns."

Step 4: Claude Opus (30 min)
"Review the beat detection code for:
- Performance bottlenecks
- Memory leaks
- Thread safety issues
- Edge case handling
Suggest optimizations for 60 FPS stability."
```

**Result:** High-quality, production-ready feature

---

### Pattern 3: Cross-Validation Review
**Scenario:** You're unsure if an implementation is correct

**Strategy:**
1. Implement with one AI (e.g., Copilot)
2. Review with another AI (e.g., Claude)
3. Validate findings with third AI (e.g., Gemini)

**Example:**
```
Step 1: GitHub Copilot implements toggleFavorite()

Step 2: Claude Sonnet reviews
"Review this toggleFavorite() implementation for:
- Memory leaks
- Thread safety
- Performance issues
- Edge cases (null paths, deleted presets)"

Step 3: Gemini Pro validates
"Compare this favorites implementation against best practices
for C++ STL containers. Are we using std::vector correctly?
Should we use std::set instead for O(1) lookups?"
```

**Result:** Robust, peer-reviewed code

---

### Pattern 4: Research → Design → Implement
**Scenario:** You're implementing something you've never done before

**Strategy:**
1. **Gemini Flash** → Fast research (what's possible?)
2. **Claude Opus** → Deep design (what's best?)
3. **GitHub Copilot** → Rapid implementation (get it done)
4. **Claude Sonnet** → Testing and refinement

**Example: Custom Preset Uploads**
```
Step 1: Gemini Flash (10 min)
"Research:
1. How do .milk preset files work?
2. What's the file format?
3. What validation is needed?
4. How does projectM load custom presets?"

Step 2: Claude Opus (30 min)
"Design a preset upload system that:
1. Opens file picker (Windows API or ImGui)
2. Validates .milk format
3. Copies to %APPDATA%/Vibeus/custom_presets/
4. Refreshes preset database
5. Handles errors gracefully
Provide complete architecture with error handling."

Step 3: GitHub Copilot (1 hour)
// You implement based on Claude's design, Copilot assists
void importPreset() {
    // File picker code
    // Validation logic
    // File copy
    // Database refresh
}

Step 4: Claude Sonnet (20 min)
"Generate a comprehensive test plan for preset uploads:
- Valid .milk files
- Invalid files (txt, exe, corrupted)
- Duplicate filenames
- Disk full scenarios
- Permission errors"
```

**Result:** Well-researched, well-designed, well-tested feature

---

## 🗓️ Sprint-Level Multi-Agent Coordination

### Sprint Planning (Day 0)
**Task:** Plan the sprint backlog

**Agent Assignment:**
1. **Gemini Pro** → Analyze remaining epics, estimate complexity
2. **Claude Sonnet** → Break down epics into user stories
3. **You** → Review and prioritize based on AI insights

**Prompt Example:**
```
Gemini: "Review RELEASE_PLAN.md Epic 1 and Epic 2.
For each user story, estimate story points (1-13) based on:
- Lines of code to change
- Number of files affected
- API complexity
- Testing requirements"

Claude: "Break Epic 4.3 (Custom Playlists) into 5-8 user stories.
Each story should be independently testable and <8 story points.
Use the USER_STORY_TEMPLATE.md format."
```

---

### Daily Development (Days 1-9)

#### Morning (Planning)
1. **Review yesterday's progress** (5 min)
2. **Gemini Flash** → Quick exploration of today's target files (10 min)
3. **Plan tasks** with prioritization (5 min)

#### Midday (Implementation)
1. **GitHub Copilot** → In-editor coding (2-3 hours)
2. **Claude Sonnet** → Complex logic and edge cases (async, 1 hour)
3. **Gemini Pro** → Generate test cases (async, 30 min)

#### Afternoon (Integration & Testing)
1. **Integrate** Copilot and Claude implementations
2. **Test** using Gemini-generated test plan
3. **Claude Opus** → Final review (if critical feature)

#### Evening (Documentation)
1. **Gemini Pro** → Generate documentation and release notes
2. **Update** sprint tracking (SPRINT_TEMPLATE.md)
3. **Commit** changes (or document if no repo)

---

### Sprint Review (Day 10)
**Task:** Validate completed stories

**Agent Assignment:**
1. **Claude Sonnet** → Code review all changes
2. **Gemini Pro** → Generate demo script
3. **You** → Record demo video, update metrics

---

## 🛠️ Practical AI Handoff Examples

### Example 1: Settings Overhaul (Epic 3)

#### Story 3.1: Fix Broken Settings

**Step 1: Exploration** (Gemini Flash)
```bash
"Analyze main.cpp, config.h, and menu_overlay.cpp.
Find all references to 'gamepadDeadzone' and 'perfMode'.
Show me where these settings are defined, loaded, saved, and applied."
```

**Step 2: Fix Implementation** (GitHub Copilot)
```cpp
// In main.cpp, you type:
float normalizeStick(int16_t raw) {
    // Copilot suggests using g_config.gamepadDeadzone
    if (abs(raw) < g_config.gamepadDeadzone) return 0.0f;
    // ... rest of implementation
}
```

**Step 3: Validation** (Claude Sonnet)
```bash
"Review the gamepadDeadzone fix in main.cpp:408.
Verify:
1. The setting is correctly loaded from config.json
2. The UI slider is bound to the config value
3. Changes apply immediately without restart
4. Edge cases are handled (deadzone > 32767)"
```

---

### Example 2: Favorites System (Epic 4.2)

**Phase 1: Design** (Claude Opus)
```bash
"Design a favorites system for Vibeus that:
1. Stores favorited preset paths in config.json
2. Provides UI star toggle (☆/★) in preset browser
3. Adds 'Favorites' filter tab
4. Persists across app restarts

Requirements:
- Thread-safe (presets can change during playback)
- Fast lookups (O(1) for isFavorite check)
- Handles edge cases (deleted presets, corrupted config)

Provide:
- Data structures (config.h)
- Method signatures (preset_manager.h)
- UI mockup (ImGui code)
- JSON schema for persistence"
```

**Phase 2: Implementation** (GitHub Copilot + Claude Sonnet)
```bash
# You implement structure based on Claude's design
# Copilot fills in boilerplate (JSON serialization, getters/setters)
# Claude Sonnet writes complex logic:

"Implement toggleFavorite() in preset_manager.cpp with:
1. Thread-safe modification using std::mutex
2. Check if preset file exists before adding
3. Update UI state immediately after toggle
4. Log actions for debugging"
```

**Phase 3: Testing** (Gemini Pro)
```bash
"Generate a test plan for the favorites system:
- Normal operations (star/unstar)
- Edge cases (1000+ favorites, deleted presets)
- Performance tests (lookup speed)
- Persistence tests (restart app)
Use TESTING_CHECKLIST.md format."
```

**Phase 4: Review** (Claude Sonnet)
```bash
"Review the completed favorites system for:
- Memory leaks (std::vector operations)
- Performance bottlenecks
- Thread safety issues
- Missing error handling
Provide specific line-by-line suggestions."
```

---

### Example 3: Beat Detection Tuning (Epic 1.2)

**Research Phase** (Gemini Pro)
```bash
"Research projectM beat detection:
1. Read docs in projectchilltitties/docs/
2. Search for beat-related API calls in src/api/
3. Find example usage in src/sdl-test-ui/
4. Summarize:
   - How beat detection works (algorithm)
   - Tunable parameters
   - Typical values for different genres
   - Known limitations"
```

**Design Phase** (Claude Opus)
```bash
"Based on projectM beat detection research, design a genre-specific
audio profile system:

Profiles needed:
- EDM (140 BPM, bass-heavy, fast response)
- Rock (120 BPM, mid-range emphasis, punchy)
- Classical (60-80 BPM, full spectrum, smooth)
- Custom (user-adjustable)

For each profile, specify:
- Beat sensitivity values
- Frequency band weights (bass, mid, treble)
- Response curve (fast/slow attack)
- Example songs to test with

Provide implementation plan:
- Config storage (config.h)
- Profile selection UI (menu_overlay.cpp)
- Audio processing pipeline changes (audio_capture.cpp or main.cpp)
- projectM API calls"
```

**Implementation Phase** (GitHub Copilot)
```cpp
// You create structure, Copilot fills in:
struct AudioProfile {
    float beatSensitivity;
    float bassWeight;
    float midWeight;
    float trebleWeight;
    
    static AudioProfile EDM() {
        // Copilot suggests values
        return {2.5f, 1.5f, 1.0f, 0.8f};
    }
    
    static AudioProfile Rock() {
        // Copilot suggests values
        return {2.0f, 1.2f, 1.5f, 1.0f};
    }
    // ... etc
};
```

**Testing Phase** (Claude Sonnet)
```bash
"Generate a beat detection test protocol:
1. Select 10 test songs across genres
2. For each song, define expected behavior:
   - Beats per measure detected
   - False positive rate (<10%)
   - Visual sync quality (subjective 1-5 scale)
3. Test with each audio profile
4. Document results in TESTING_CHECKLIST.md format
5. Suggest tuning adjustments based on results"
```

---

## 📋 Task Distribution Cheat Sheet

| Task Type | Primary AI | Secondary AI | Tertiary AI |
|-----------|-----------|--------------|-------------|
| **Exploration** | Gemini Flash | Claude Haiku | - |
| **Design** | Claude Opus | Gemini Pro | Claude Sonnet |
| **Boilerplate** | GitHub Copilot | Gemini Flash | - |
| **Complex Logic** | Claude Sonnet | Claude Opus | Gemini Pro |
| **Code Review** | Claude Sonnet | Claude Opus | Gemini Pro |
| **Testing Plans** | Gemini Pro | Claude Sonnet | - |
| **Documentation** | Gemini Pro | Claude Sonnet | GitHub Copilot |
| **Debugging** | Claude Sonnet | Claude Opus | Gemini Pro |
| **Optimization** | Claude Opus | Gemini Pro | - |
| **Security Audit** | Claude Opus | Claude Sonnet | - |

---

## 🚀 Advanced Multi-Agent Patterns

### Pattern 5: Parallel Sprint Execution
**Scenario:** Sprint with 20 story points, 4 independent stories

**Strategy:** Work on all 4 stories simultaneously using different AIs

```
Monday Morning (Sprint Start):
├─ Story A (5 pts) - Gemini Pro designs, Claude Sonnet implements
├─ Story B (3 pts) - GitHub Copilot implements (straightforward)
├─ Story C (8 pts) - Claude Opus designs (complex, needs deep thought)
└─ Story D (4 pts) - Gemini Flash explores + plans

Monday Afternoon:
├─ Story A - Claude Sonnet finished, you test
├─ Story B - Copilot finished, Claude Sonnet reviews
├─ Story C - Claude Opus design done, you start implementing
└─ Story D - Gemini gave plan, you start implementing with Copilot

Tuesday:
├─ Story A - ✅ Complete, merged
├─ Story B - ✅ Complete, merged
├─ Story C - 50% done, Copilot helping with boilerplate
└─ Story D - 75% done, Claude Sonnet adding edge cases

Wednesday:
├─ Story C - ✅ Complete, Claude Opus reviews
└─ Story D - ✅ Complete, Gemini Pro generates tests
```

**Result:** 20 points done in 3 days (high velocity)

---

### Pattern 6: AI Committee Design Review
**Scenario:** Critical architectural decision (e.g., refactoring settings system)

**Strategy:** Get independent opinions from 3 AIs, synthesize best approach

```
Phase 1: Independent Designs (Parallel)
├─ Claude Opus: "Design a settings system with validation and presets"
├─ Gemini Pro: "Design a settings system with validation and presets"
└─ GitHub Copilot: [Inline code exploration of current system]

Phase 2: Compare Approaches
You: Read all 3 designs, identify:
- Common patterns (likely good ideas)
- Unique approaches (consider trade-offs)
- Conflicting recommendations (need deeper analysis)

Phase 3: Synthesis
Claude Opus: "Here are 3 different settings system designs.
Compare them and recommend the best hybrid approach considering:
- Maintainability
- Performance
- Testability
- Future extensibility (Steam Cloud sync, mobile)"

Phase 4: Implement
You + GitHub Copilot: Implement the synthesized design
```

**Result:** Battle-tested architecture with multiple perspectives

---

## 🎯 Sprint-Specific AI Assignments

### Sprint 1: Beat Detection & Audio

| Story | AI Agent | Rationale |
|-------|----------|-----------|
| 1.1 Beat Detection | Claude Sonnet | Complex algorithm tuning |
| 1.2 Audio Profiles | Gemini Pro → Copilot | Research → Implement |
| 1.3 Beat Feedback | GitHub Copilot | Simple UI addition |
| 3.1 Fix Settings | Copilot → Claude | Simple fix → Review |
| 3.2 Visual Detail | GitHub Copilot | Straightforward slider |

**Workflow:**
- **Day 1-2:** Gemini researches projectM beat detection
- **Day 3-5:** Claude implements beat detection tuning
- **Day 6-8:** Copilot implements UI and simple features
- **Day 9:** Claude reviews all code
- **Day 10:** Testing and sprint review

---

### Sprint 4: Preset Organization

| Story | AI Agent | Rationale |
|-------|----------|-----------|
| 4.1 Category Browser | Claude Opus → Copilot | Complex UI → Boilerplate |
| 4.2 Favorites | Claude Sonnet → Copilot | Thread safety → UI |
| 4.5 Search | Gemini Pro → Copilot | Algorithm design → Implement |

**Workflow:**
- **Day 1:** Claude Opus designs category browser architecture
- **Day 2-3:** You + Copilot implement category UI
- **Day 4:** Claude Sonnet designs favorites (parallel to Day 2-3)
- **Day 5-6:** You + Copilot implement favorites
- **Day 7:** Gemini designs search filtering
- **Day 8:** You + Copilot implement search
- **Day 9:** Claude Opus reviews entire preset management system
- **Day 10:** Integration testing

---

## 🔧 Tooling & Setup

### GitHub Copilot
**Setup:**
- Install Visual Studio Code extension
- Enable inline completions
- Configure for C++ (if needed)

**Workflow Integration:**
```bash
# In VS Code:
1. Start typing function signature
2. Press Tab to accept Copilot suggestion
3. Continue typing, Copilot predicts next lines
4. Use Ctrl+Enter to see alternative suggestions
```

---

### Google Gemini
**Access Options:**
1. **Google AI Studio** (free, web-based)
   - https://aistudio.google.com/
   - 2M token context window
   - Free tier: 50 requests/day

2. **Gemini API** (programmatic)
   - Python/JavaScript SDK
   - Pay-per-token after free tier

**Workflow Integration:**
```bash
# Web-based workflow:
1. Copy relevant code/docs to Gemini chat
2. Ask design/research questions
3. Copy response to markdown file for reference

# Programmatic workflow (optional):
# Create a Python script to batch-query Gemini
import google.generativeai as genai
genai.configure(api_key="YOUR_API_KEY")
model = genai.GenerativeModel('gemini-2.0-flash')
response = model.generate_content("Analyze preset_manager.cpp...")
```

---

### Claude (Anthropic)
**Access Options:**
1. **Claude.ai** (free, web-based)
   - https://claude.ai/
   - 200K token context
   - Free tier with usage limits

2. **Claude API** (programmatic)
   - Higher rate limits
   - Pay-per-token

**Workflow Integration:**
```bash
# Web-based workflow:
1. Create projects in Claude.ai for different epics
2. Upload relevant source files to project
3. Ask implementation/review questions
4. Copy code back to your editor

# Pro tip: Use Claude Projects feature
- One project per epic
- Upload all related files once
- Claude remembers context across conversations
```

---

## 📊 Tracking Multi-Agent Work

### Sprint Tracking Update
Add "AI Agent" column to sprint tracking:

```markdown
#### 🔄 IN PROGRESS
- [ ] **EPIC1-001** - Beat Detection Tuning **(5 pts)** 🔄
  - **Status:** In Progress
  - **Started:** 2026-03-26
  - **AI Agent:** Claude Sonnet (design), Copilot (implementation)
  - **Progress:** Design complete, 60% implemented
```

### AI Usage Log (Optional)
Track which AI did what for retrospective insights:

```markdown
## AI Usage This Sprint
- **Gemini Flash:** 5 hours (exploration, research)
- **Gemini Pro:** 3 hours (test plan generation)
- **GitHub Copilot:** 15 hours (in-editor implementation)
- **Claude Sonnet:** 8 hours (complex logic, reviews)
- **Claude Opus:** 2 hours (architectural design)

**Total AI Time:** 33 hours
**Human Time:** 40 hours
**Velocity Multiplier:** ~1.8x (AI-assisted vs solo)
```

---

## 🎓 Best Practices

### Do's ✅
- ✅ **Give clear context:** Always provide file names, line numbers, and relevant code snippets
- ✅ **Use templated prompts:** Standardize your queries for consistency
- ✅ **Cross-validate critical code:** Have 2+ AIs review important features
- ✅ **Document AI decisions:** Note which AI suggested what in commit messages
- ✅ **Iterate rapidly:** AI responses are cheap—don't hesitate to ask follow-up questions
- ✅ **Combine strengths:** Use Gemini for research → Claude for design → Copilot for implementation

### Don'ts ❌
- ❌ **Don't blindly copy AI code:** Always review and understand before using
- ❌ **Don't over-rely on one AI:** Different models have different blind spots
- ❌ **Don't skip testing:** AI-generated code still needs manual testing
- ❌ **Don't ignore warnings:** If AI says "this is risky," investigate why
- ❌ **Don't forget context limits:** Keep conversations focused on one epic/story
- ❌ **Don't use slow models for simple tasks:** Match AI capability to task complexity

---

## 🚦 Decision Matrix: Which AI to Use?

### Task: Exploring New Codebase
- **If need speed:** Gemini Flash ⚡⚡⚡
- **If need depth:** Claude Opus 🧠🧠🧠
- **If need both:** Gemini Flash first, then Claude Sonnet for deep dive

### Task: Designing New Feature
- **If straightforward:** GitHub Copilot 💨
- **If moderate complexity:** Claude Sonnet 🎯
- **If high complexity:** Claude Opus 🏆

### Task: Implementing Feature
- **If boilerplate:** GitHub Copilot ⚡
- **If complex logic:** Claude Sonnet 🧠
- **If both:** Copilot for structure, Claude for logic

### Task: Code Review
- **If quick sanity check:** Gemini Pro ⚡⚡
- **If thorough review:** Claude Sonnet 🔍
- **If critical feature:** Claude Opus 🛡️

### Task: Debugging
- **If simple bug:** GitHub Copilot 🔧
- **If mysterious bug:** Claude Sonnet 🕵️
- **If critical bug:** Claude Opus 🚨

---

## 🎬 Real-World Example: Full Feature End-to-End

### Feature: Custom Playlists (Epic 4.3, 13 story points)

**Day 1: Planning & Design**
```
Morning (1 hour):
└─ Gemini Pro: "Research playlist patterns in music apps.
   What UX patterns are common? What data structures are standard?"

Afternoon (2 hours):
└─ Claude Opus: "Design a playlist system for Vibeus with:
   - Create/rename/delete playlists
   - Add/remove presets (multi-select or drag-drop)
   - Scoped shuffle/random
   - JSON persistence
   Provide complete architecture, data structures, UI mockups."

Evening (30 min):
└─ Review both AI outputs, create implementation plan
```

**Day 2-3: Data Layer Implementation**
```
Morning (2 hours):
└─ GitHub Copilot: Implement playlist data structures in config.h
   std::map<std::string, std::vector<std::string>> playlists;

Afternoon (3 hours):
└─ Claude Sonnet: "Implement playlist management methods:
   - createPlaylist(name)
   - deletePlaylist(name)
   - addPresetToPlaylist(playlistName, presetPath)
   - removePresetFromPlaylist(playlistName, presetPath)
   - getPresetsInPlaylist(playlistName)
   Handle edge cases: duplicate names, empty playlists, invalid paths."

Evening (1 hour):
└─ Test data layer manually, fix bugs
```

**Day 4-5: UI Implementation**
```
Morning (1 hour):
└─ Gemini Pro: "Design ImGui UI layout for playlist management.
   Show mockup with buttons, list boxes, text input for naming."

Afternoon (4 hours):
└─ You + GitHub Copilot: Implement UI in menu_overlay.cpp
   - Playlist tab
   - Create button + modal dialog
   - Playlist list
   - Add/remove preset buttons
   Copilot handles ImGui boilerplate

Evening (1 hour):
└─ Claude Sonnet: "Review UI code for:
   - Memory leaks in ImGui widgets
   - Input validation (empty playlist names, special characters)
   - User feedback (success/error messages)"
```

**Day 6: Integration & Scoped Shuffle**
```
Morning (2 hours):
└─ You + GitHub Copilot: Integrate playlists with preset_manager.cpp
   - Filter presets to active playlist
   - Shuffle only within playlist scope

Afternoon (2 hours):
└─ Claude Sonnet: "Implement scoped random/shuffle logic.
   When a playlist is active, ensure:
   - Random button picks only from playlist
   - Shuffle only reorders playlist presets
   - Auto-advance stays within playlist
   - User can exit playlist mode to browse all presets"

Evening (1 hour):
└─ Test integration end-to-end
```

**Day 7: Polish & Edge Cases**
```
Morning (2 hours):
└─ Gemini Pro: "Generate comprehensive test plan for playlists:
   - Normal operations
   - Edge cases (1000+ playlists, empty playlists)
   - Persistence (restart app, corrupted config)
   - UI responsiveness (large playlists)"

Afternoon (2 hours):
└─ Execute test plan, fix bugs

Evening (1 hour):
└─ Claude Opus: "Final review of playlist system for:
   - Performance (large playlists)
   - User experience (is it intuitive?)
   - Maintainability (is code clean?)
   - Security (can malicious playlist.json crash app?)"
```

**Day 8: Documentation & Demo**
```
Morning (1 hour):
└─ Gemini Pro: "Generate documentation for playlist feature:
   - User guide (how to create playlists)
   - Developer docs (how to extend playlists)
   - Release notes entry"

Afternoon (2 hours):
└─ Record demo video
   Update sprint tracking
   Commit feature

Evening:
└─ Celebrate! 🎉
```

**Total Time:** 8 days, 13 story points  
**AI Distribution:**
- Gemini: 30% (research, docs, testing)
- Claude: 40% (design, complex logic, reviews)
- Copilot: 30% (boilerplate, UI, data structures)

---

## 🎉 Summary

**Key Takeaways:**
1. **Distribute tasks** based on AI strengths (exploration, design, implementation, review)
2. **Work in parallel** on independent features
3. **Cross-validate** critical code with multiple AIs
4. **Iterate rapidly** using AI for fast feedback loops
5. **Track AI usage** to optimize workflow over time

**Your Multi-Agent Superpower:**
> As a solo developer, you now have a team of specialized AI experts working for you 24/7. Use them wisely, and you can achieve 2-3x velocity while maintaining high code quality.

**Next Steps:**
1. Set up access to all three AI providers (Copilot, Gemini, Claude)
2. Try the "Parallel Feature Development" pattern in Sprint 1
3. Track AI usage in your sprint log
4. Refine your workflow based on what works best for you

**Remember:** AI is a force multiplier, not a replacement for your judgment. You're still the architect, product manager, and QA lead. AI is your implementation team.

---

**Questions? Experiment and iterate!** 🚀

Update this guide as you discover new patterns that work for your specific workflow.
