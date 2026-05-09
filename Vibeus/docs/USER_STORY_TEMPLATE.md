# User Story Template

---

## Story ID: [EPIC-###]
**Title:** [One sentence description]  
**Epic:** [Link to parent epic in RELEASE_PLAN.md]  
**Sprint:** [Sprint number or "Backlog"]  
**Story Points:** [1, 2, 3, 5, 8, 13, 21]  
**Priority:** ⭐⭐⭐⭐⭐ (5 = Critical, 1 = Nice to have)

---

## 📖 User Story
**As a** [type of user],  
**I want** [goal/desire],  
**So that** [benefit/value].

**Example:**
> As a user, I want to favorite presets by clicking a star icon, so that I can easily find my favorite visuals later.

---

## ✅ Acceptance Criteria
Clear, testable conditions that must be met for the story to be "done":

- [ ] **Given** [context/precondition]  
      **When** [action taken]  
      **Then** [expected result]

- [ ] [Additional criterion]

- [ ] [Additional criterion]

**Example:**
- [ ] **Given** the preset browser is open  
      **When** I click the star icon next to a preset name  
      **Then** the preset is added to my favorites list
      
- [ ] **Given** I have favorited a preset  
      **When** I navigate to the "Favorites" tab  
      **Then** the preset appears in the filtered list
      
- [ ] **Given** I restart the application  
      **When** I check my favorites  
      **Then** previously favorited presets are still there (persisted)

---

## 🎨 UI/UX Mockup (if applicable)
[Link to screenshot, drawing, or description]

**Example:**
```
┌─────────────────────────────┐
│ Preset Browser              │
├─────────────────────────────┤
│ ☆ Preset Name 1    [Author] │  ← Hollow star = not favorited
│ ★ Preset Name 2    [Author] │  ← Filled star = favorited
│ ☆ Preset Name 3    [Author] │
└─────────────────────────────┘
```

---

## 🛠️ Implementation Details

### Files to Modify:
- `config.h` - Add `std::vector<std::string> favoritePresets;`
- `config.cpp` - Add JSON serialization for favorites array
- `menu_overlay.cpp` - Add star icon button, click handler
- `preset_manager.h/cpp` - Add `bool isFavorite(path)`, `void toggleFavorite(path)` methods

### Technical Approach:
[High-level description of implementation strategy]

**Example:**
1. Store favorites as array of preset file paths in config.json
2. Add star button UI using ImGui button with Unicode star character
3. On click, call `preset_manager.toggleFavorite(currentPresetPath)`
4. Favorites tab filters preset list by checking `isFavorite()` for each preset

### API/Library Usage:
- `nlohmann/json` for favorites array serialization
- `ImGui::Button()` with Unicode "☆"/"★" characters
- `std::find()` to check if preset is in favorites vector

### Edge Cases to Handle:
- [ ] What if preset file is deleted but still in favorites?
- [ ] What if favorites.json is corrupted?
- [ ] What if user favorites 1000+ presets (performance)?

---

## 🧪 Testing Checklist

### Manual Testing:
- [ ] Can favorite a preset by clicking star icon
- [ ] Star icon changes from ☆ to ★ when favorited
- [ ] Can unfavorite by clicking filled star
- [ ] Favorites tab shows only favorited presets
- [ ] Favorites persist after app restart
- [ ] Works with 100+ favorites (performance test)
- [ ] Favoriting a preset during playback doesn't interrupt visuals

### Edge Case Testing:
- [ ] Favoriting non-existent preset (error handling)
- [ ] Favorites list survives config.json corruption
- [ ] Can favorite presets from different categories
- [ ] Favorites work with custom uploaded presets

### Performance Testing:
- [ ] Favoriting 1000 presets: <100ms load time
- [ ] Filtering favorites tab: <50ms response time

---

## 📚 Dependencies

### Depends On:
- [ ] [EPIC-XXX] - [Story that must complete first]

### Blocked By:
- [ ] [External dependency, e.g., projectM API change]

### Blocks:
- [ ] [EPIC-YYY] - [Story waiting on this]

---

## 🤖 AI Agent Assignment

**Primary Agent:** [GitHub Copilot / Gemini / Claude / Mixed]

**Agent Tasks:**
- [ ] **Analysis:** Explore existing preset_manager.cpp to understand playlist structure
- [ ] **Design:** Generate JSON schema for favorites storage
- [ ] **Implementation:** Write toggleFavorite() method
- [ ] **UI:** Create star icon button in ImGui
- [ ] **Testing:** Generate test cases
- [ ] **Review:** Code review for memory leaks

**Agent Handoff:**
- **Gemini** → Analyze existing code structure (fast exploration)
- **GitHub Copilot** → Implement JSON serialization (in-editor)
- **Claude Opus** → Review for edge cases and performance

---

## 📝 Implementation Notes

### Progress Updates:
**[Date]** - [Your update here]
- Started implementation
- Completed config.h changes
- Etc.

### Learnings:
- [Any discoveries made during implementation]

### Challenges:
- [Issues encountered and how they were resolved]

---

## ✅ Definition of Done

Story is complete when:
- [ ] All acceptance criteria met
- [ ] Code committed to repo (or documented if no repo)
- [ ] Manual testing checklist complete (all pass)
- [ ] No critical bugs introduced
- [ ] Performance meets targets (<100ms for operations)
- [ ] UI fits 1280×800 resolution (Steam Deck)
- [ ] Changes documented in RELEASE_NOTES.md
- [ ] AI code review complete
- [ ] Validated by running app end-to-end

---

## 🎥 Demo / Screenshot
[Link to video or image showing completed feature]

---

## 📊 Estimation Accuracy

**Estimated:** [X] story points  
**Actual Time:** [Y] days  
**Notes:** [Was estimate accurate? Why or why not?]

This helps improve future estimation.

---

**Created:** [Date]  
**Completed:** [Date]  
**Sprint:** [Sprint number]
