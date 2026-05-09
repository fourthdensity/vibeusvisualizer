# Vibeus Development Documentation Index

This directory contains all planning, workflow, and technical documentation for Vibeus development.

---

## 📚 Documentation Map

### 🚀 **Start Here (New Developer)**
1. **[QUICK_START.md](../QUICK_START.md)** - 30-minute guide to get productive with AI workflow
2. **[RELEASE_PLAN.md](../RELEASE_PLAN.md)** - Full SCRUM roadmap (17 weeks, 7 epics, ~200 story points)
3. **[AI_AGENTS_GUIDE.md](AI_AGENTS_GUIDE.md)** - Multi-agent AI development patterns (GitHub Copilot, Gemini, Claude)

### 📋 **Planning & Tracking**
- **[SPRINT_TEMPLATE.md](SPRINT_TEMPLATE.md)** - Template for 2-week sprint tracking
- **[USER_STORY_TEMPLATE.md](USER_STORY_TEMPLATE.md)** - Template for feature user stories
- **[TESTING_CHECKLIST.md](TESTING_CHECKLIST.md)** - Comprehensive testing for each epic

### 🏗️ **Architecture & Design**
- **[SETTINGS_ARCHITECTURE.md](SETTINGS_ARCHITECTURE.md)** - Settings system flow diagram
- **[SETTINGS_DESIGN.md](SETTINGS_DESIGN.md)** - Settings UI design spec
- **[SETTINGS_IMPLEMENTATION.md](SETTINGS_IMPLEMENTATION.md)** - Settings implementation details
- **[SETTINGS_CHECKLIST.md](SETTINGS_CHECKLIST.md)** - Settings validation checklist
- **[PRESET_CATEGORIES_DESIGN.md](PRESET_CATEGORIES_DESIGN.md)** - Preset organization system spec
- **[PERFORMANCE_GUIDE.md](PERFORMANCE_GUIDE.md)** - Performance optimization guide

### 📝 **Project Planning**
- **[OVERHAUL_MASTER_PLAN.md](../OVERHAUL_MASTER_PLAN.md)** - 3-part overhaul plan (touch removal, settings, presets)
- **[ROADMAP.md](../ROADMAP.md)** - Long-term vision and competitive analysis

### 📖 **Change Logs**
- **[RELEASE_NOTES_v0.2.1.md](../RELEASE_NOTES_v0.2.1.md)** - Version 0.2.1 release notes
- **[IMGUI_FIX_AND_CONTROLS_LOG.md](../IMGUI_FIX_AND_CONTROLS_LOG.md)** - ImGui fixes and control changes
- **[PRESET_FILTERING_LOG.md](../PRESET_FILTERING_LOG.md)** - Preset filtering implementation log
- **[QUARANTINE_FEATURE_LOG.md](../QUARANTINE_FEATURE_LOG.md)** - Preset quarantine feature log
- **[SETTINGS_OVERHAUL_LOG.md](../SETTINGS_OVERHAUL_LOG.md)** - Settings overhaul implementation log
- **[TOUCH_REMOVAL_LOG.md](../TOUCH_REMOVAL_LOG.md)** - Touch functionality removal log

### 🐛 **Bug Fixes**
- **[VALIDATION_INIT_BUGFIX.md](../VALIDATION_INIT_BUGFIX.md)** - Validation initialization fix
- **[INSTALLER_CRASH_FIX.md](../INSTALLER_CRASH_FIX.md)** - Installer crash fix

### 🚢 **Deployment**
- **[DEPLOYMENT_SUMMARY.md](../DEPLOYMENT_SUMMARY.md)** - Deployment process summary
- **[CONTROL_REMAPPING_STATUS.md](../CONTROL_REMAPPING_STATUS.md)** - Control remapping status

---

## 🗺️ Workflow Overview

### Daily Workflow
```
Morning:
├─ Review SPRINT_XX.md (check progress)
├─ Pick next user story from backlog
└─ Use AI_AGENTS_GUIDE.md patterns to implement

Afternoon:
├─ Code using GitHub Copilot
├─ Design with Claude/Gemini as needed
└─ Test using TESTING_CHECKLIST.md

Evening:
├─ Update SPRINT_XX.md progress
├─ Commit changes (or document)
└─ Plan tomorrow's work
```

### Sprint Workflow (2 weeks)
```
Day 0 (Planning):
├─ Review RELEASE_PLAN.md for upcoming epics
├─ Copy SPRINT_TEMPLATE.md → SPRINT_XX.md
├─ Select 20-25 story points worth of stories
└─ Assign AI agents to each story

Days 1-9 (Execution):
├─ Implement user stories
├─ Update SPRINT_XX.md daily
├─ Use AI_AGENTS_GUIDE.md patterns
└─ Test using TESTING_CHECKLIST.md

Day 10 (Review & Retro):
├─ Complete sprint review in SPRINT_XX.md
├─ Run full regression testing
├─ Sprint retrospective (learnings)
└─ Plan next sprint
```

---

## 📊 Priority Documents by Role

### If You're Planning Features:
1. **RELEASE_PLAN.md** - See all epics and user stories
2. **USER_STORY_TEMPLATE.md** - Create new user stories
3. **PRESET_CATEGORIES_DESIGN.md** - Understand preset system
4. **SETTINGS_ARCHITECTURE.md** - Understand settings system

### If You're Implementing Features:
1. **AI_AGENTS_GUIDE.md** - Learn multi-agent workflow
2. **QUICK_START.md** - Get started in 30 minutes
3. **SPRINT_TEMPLATE.md** - Track your progress
4. **OVERHAUL_MASTER_PLAN.md** - See implementation details

### If You're Testing:
1. **TESTING_CHECKLIST.md** - Comprehensive test cases
2. **SPRINT_TEMPLATE.md** - Track test results
3. **PERFORMANCE_GUIDE.md** - Performance benchmarks

### If You're Doing Code Review:
1. **AI_AGENTS_GUIDE.md** - Use Claude for reviews
2. **SETTINGS_ARCHITECTURE.md** - Validate settings flow
3. **PRESET_CATEGORIES_DESIGN.md** - Validate preset logic

---

## 🎯 Key Concepts

### Story Points (Fibonacci Scale)
- **1-2 points:** ~1 day or less (simple feature, small change)
- **3-5 points:** 2-3 days (moderate complexity)
- **8 points:** 3-4 days (complex feature, multiple files)
- **13+ points:** Too large, break down into smaller stories

### Sprint Capacity
- **Solo developer:** 20-25 story points per 2-week sprint
- **With AI:** Can achieve 2-3x velocity (but maintain quality)

### Epics (Release Plan)
- **Epic 1:** Beat Detection & Audio Reactivity (10 pts)
- **Epic 2:** Smooth Transitions & Flow (18 pts)
- **Epic 3:** Settings Overhaul (15 pts)
- **Epic 4:** Preset Management (34 pts)
- **Epic 5:** UI/UX Polish (14 pts)
- **Epic 6:** Steam Integration (34 pts)
- **Epic 7:** Performance & Stability (21 pts)

**Total:** 146 story points (~7-8 sprints, ~17 weeks)

---

## 🤖 AI Workflow Quick Reference

### Exploration → Design → Implement → Review
```
1. Gemini Flash: "Analyze preset_manager.cpp..."
   └─ Fast codebase exploration (5 min)

2. Claude Opus: "Design a favorites system..."
   └─ Deep architectural design (30 min)

3. GitHub Copilot: [You implement in VS Code]
   └─ Rapid coding with autocomplete (1 hour)

4. Claude Sonnet: "Review this code for bugs..."
   └─ Thorough code review (15 min)
```

### Parallel Development
```
Story A (5 pts) → Gemini Pro + Claude Sonnet
Story B (3 pts) → GitHub Copilot (you implement)
Story C (8 pts) → Claude Opus (design only, implement later)

Result: 3 stories progressing simultaneously
```

---

## 📖 Document Relationships

```
QUICK_START.md
    ├─ References AI_AGENTS_GUIDE.md
    ├─ References RELEASE_PLAN.md
    └─ References SPRINT_TEMPLATE.md

RELEASE_PLAN.md
    ├─ Contains all 7 Epics
    ├─ References USER_STORY_TEMPLATE.md
    ├─ References TESTING_CHECKLIST.md
    └─ Links to OVERHAUL_MASTER_PLAN.md

AI_AGENTS_GUIDE.md
    ├─ References SPRINT_TEMPLATE.md
    ├─ References USER_STORY_TEMPLATE.md
    └─ Contains workflow patterns

OVERHAUL_MASTER_PLAN.md
    ├─ References SETTINGS_ARCHITECTURE.md
    ├─ References PRESET_CATEGORIES_DESIGN.md
    └─ Detailed implementation plans

SPRINT_TEMPLATE.md
    ├─ Links to USER_STORY_TEMPLATE.md
    ├─ Links to TESTING_CHECKLIST.md
    └─ Tracks daily progress
```

---

## 🚀 Getting Started Paths

### Path 1: "I want to start coding NOW"
1. Read **QUICK_START.md** (30 min)
2. Pick a story from **RELEASE_PLAN.md** Sprint 1 (5 min)
3. Follow **AI_AGENTS_GUIDE.md** patterns (ongoing)

### Path 2: "I want to understand the architecture first"
1. Read **OVERHAUL_MASTER_PLAN.md** (20 min)
2. Read **SETTINGS_ARCHITECTURE.md** (15 min)
3. Read **PRESET_CATEGORIES_DESIGN.md** (15 min)
4. Then follow Path 1

### Path 3: "I want to plan the entire project"
1. Read **RELEASE_PLAN.md** in full (45 min)
2. Read **ROADMAP.md** for long-term vision (15 min)
3. Read **AI_AGENTS_GUIDE.md** for workflow (30 min)
4. Create **SPRINT_01.md** from template (10 min)
5. Start implementing

---

## 🔍 Finding Information

### "How do I implement [feature]?"
1. Check **OVERHAUL_MASTER_PLAN.md** for existing plans
2. Check **RELEASE_PLAN.md** for user stories
3. Use **AI_AGENTS_GUIDE.md** patterns to implement
4. Follow **USER_STORY_TEMPLATE.md** structure

### "How does [system] work?"
1. Check **SETTINGS_ARCHITECTURE.md** for settings
2. Check **PRESET_CATEGORIES_DESIGN.md** for presets
3. Check **PERFORMANCE_GUIDE.md** for optimization
4. Ask Gemini/Claude to analyze source files

### "How do I test [feature]?"
1. Use **TESTING_CHECKLIST.md** for epic-level tests
2. Check **USER_STORY_TEMPLATE.md** acceptance criteria
3. Ask Gemini to generate test cases

### "How do I use AI effectively?"
1. Read **AI_AGENTS_GUIDE.md** (comprehensive)
2. Follow **QUICK_START.md** example (practical)
3. Reference task distribution matrix in AI guide

---

## 📅 Document Update Schedule

### Daily:
- **SPRINT_XX.md** - Update progress, blockers, completed stories

### Weekly:
- **SPRINT_XX.md** - Mid-sprint check-in, adjust if needed

### Per Sprint (2 weeks):
- **SPRINT_XX.md** - Complete review and retrospective
- **RELEASE_PLAN.md** - Update velocity, adjust future sprints
- **RELEASE_NOTES.md** - Document completed features

### Per Milestone (6-8 weeks):
- **ROADMAP.md** - Update long-term plans based on learnings
- **AI_AGENTS_GUIDE.md** - Add new patterns discovered
- **QUICK_START.md** - Update with new best practices

---

## 🎓 Learning Resources

### Internal Documentation:
- **All architecture docs** - Understand system design
- **All change logs** - Learn from past decisions
- **OVERHAUL_MASTER_PLAN.md** - Detailed implementation guide

### External Resources:
- projectM Wiki: https://github.com/projectM-visualizer/projectm/wiki
- ImGui Documentation: https://github.com/ocornut/imgui
- Agile/SCRUM Guide: https://scrumguides.org/
- Google Gemini: https://aistudio.google.com/
- Claude AI: https://claude.ai/

---

## ✅ Document Checklist

Before starting development, ensure you have:
- [ ] Read **QUICK_START.md**
- [ ] Reviewed **RELEASE_PLAN.md** Sprint 1
- [ ] Created **SPRINT_01.md** from template
- [ ] Set up AI access (Copilot, Gemini, Claude)
- [ ] Understand **AI_AGENTS_GUIDE.md** basic patterns

Before completing a sprint:
- [ ] All stories in **SPRINT_XX.md** marked complete
- [ ] **TESTING_CHECKLIST.md** executed for completed epics
- [ ] Sprint retrospective completed in **SPRINT_XX.md**
- [ ] Next sprint planned with story selection

Before releasing a milestone:
- [ ] All epics tested with **TESTING_CHECKLIST.md**
- [ ] **RELEASE_NOTES.md** updated
- [ ] **ROADMAP.md** reviewed and adjusted
- [ ] Demo video recorded

---

## 🆘 Help & Support

### "I'm stuck on implementation"
→ Ask Claude Sonnet: "I'm implementing [feature] and stuck on [problem]..."

### "I don't know where to start"
→ Ask Gemini Flash: "Analyze [files] and show me where to implement [feature]..."

### "My code isn't working"
→ Ask Claude Opus: "Review this code and debug: [paste code]..."

### "I need to make an architectural decision"
→ Use AI Committee pattern (AI_AGENTS_GUIDE.md Pattern 6)

### "I'm overwhelmed with too many tasks"
→ Focus on one epic at a time, one story per day

---

**Happy coding!** 🚀

This documentation is a living system. Update it as you discover new patterns and workflows.
