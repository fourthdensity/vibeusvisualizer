# Quick Start Guide - Solo Dev + Multi-Agent AI Workflow

**Goal:** Get you productive with the multi-agent AI workflow in 30 minutes

---

## 🚀 Step 1: Set Up AI Access (15 minutes)

### GitHub Copilot (In-Editor Assistant)
1. **Install VS Code Extension:**
   - Open VS Code
   - Extensions → Search "GitHub Copilot"
   - Install + Sign in with GitHub account
   - **Cost:** $10-20/month (free for students/open source)

2. **Test it works:**
   ```cpp
   // Type this in a .cpp file:
   void toggleFavorite(const std::string& path) {
       // Press Tab when Copilot suggests code
   }
   ```

### Google Gemini (Fast Research & Design)
1. **Free Access:**
   - Visit https://aistudio.google.com/
   - Sign in with Google account
   - **Cost:** FREE (50 requests/day)

2. **Test it works:**
   - Paste this: "Explain how C++ std::vector works"
   - Should get instant response

### Claude (Deep Reasoning & Architecture)
1. **Free Access:**
   - Visit https://claude.ai/
   - Sign in (email or Google)
   - **Cost:** FREE tier available

2. **Create a Project:**
   - Click "New Project" → Name it "Vibeus Dev"
   - Upload key files: `main.cpp`, `config.h`, `preset_manager.h`
   - Claude remembers context across conversations

---

## 📋 Step 2: Your First Sprint (5 minutes setup)

### Create Sprint 1 Tracking File
```bash
# Copy the template
cp Vibeus/docs/SPRINT_TEMPLATE.md Vibeus/docs/SPRINT_01.md

# Edit with your sprint goal
# Example: "Fix beat detection and implement audio profiles"
```

### Select Your First User Story
Open `Vibeus/RELEASE_PLAN.md` and pick from Sprint 1:
- **Recommended first story:** Epic 1.3 - Visual Beat Feedback (2 points, easy win)
- **Alternative:** Epic 3.1 - Fix Broken Settings (3 points)

---

## 🎯 Step 3: Your First Multi-Agent Task (10 minutes)

Let's implement **Visual Beat Feedback** using all three AIs:

### Task: Add a beat indicator to debug overlay

#### Phase 1: Exploration (Gemini - 3 min)
**Go to:** https://aistudio.google.com/

**Prompt:**
```
I need to add a beat indicator to the debug overlay in Vibeus.

Analyze these files and tell me:
1. Where is the debug overlay rendered? (which file and line number)
2. How can I detect when a beat occurs?
3. What's the best way to render a simple flash/pulse indicator?

Files to analyze:
[Paste contents of Vibeus/src/main.cpp (around line 700-900)]
```

**What you'll get:** Quick overview of where to add code

---

#### Phase 2: Design (Claude - 3 min)
**Go to:** https://claude.ai/ (your "Vibeus Dev" project)

**Prompt:**
```
I want to add a visual beat indicator to the debug overlay.

Requirements:
- Show a red flash or pulse when a beat is detected
- Indicator should fade out over 200ms
- Toggleable with a keyboard shortcut (e.g., 'B' key)
- Rendered in debug overlay (top-left corner)

Current setup:
- projectM provides beat detection events
- Debug overlay rendered in main.cpp around line 850
- ImGui used for UI rendering

Please provide:
1. Code structure (which functions to modify)
2. Beat detection listening (how to hook into projectM events)
3. Visual rendering code (ImGui or SDL2 rendering)
4. Keyboard shortcut implementation
```

**What you'll get:** Complete implementation plan with code snippets

---

#### Phase 3: Implementation (GitHub Copilot - 4 min)
**Open VS Code** to `Vibeus/src/main.cpp`

1. **Add beat indicator state (around line 100):**
   ```cpp
   // Global state for beat indicator
   bool g_showBeatIndicator = false;
   float g_beatIndicatorAlpha = 0.0f;  // 0.0 = invisible, 1.0 = full red
   ```

2. **Add keyboard shortcut (in event loop):**
   ```cpp
   case SDLK_b:
       g_showBeatIndicator = !g_showBeatIndicator;
       break;
   ```

3. **Update beat indicator when beat detected (in render loop):**
   ```cpp
   // After calling projectm_render_frame()
   if (projectm_get_beat_detected(g_pm)) {  // Copilot will suggest correct API
       g_beatIndicatorAlpha = 1.0f;  // Flash on beat
   }
   
   // Fade out over time
   if (g_beatIndicatorAlpha > 0.0f) {
       g_beatIndicatorAlpha -= deltaTime * 5.0f;  // Fade in 200ms
       if (g_beatIndicatorAlpha < 0.0f) g_beatIndicatorAlpha = 0.0f;
   }
   ```

4. **Render indicator in debug overlay:**
   ```cpp
   // In debug overlay rendering section
   if (g_showBeatIndicator && g_beatIndicatorAlpha > 0.0f) {
       // Copilot will suggest ImGui or SDL2 rendering code
       ImGui::GetBackgroundDrawList()->AddCircleFilled(
           ImVec2(50, 50),  // Top-left position
           20,  // Radius
           ImColor(1.0f, 0.0f, 0.0f, g_beatIndicatorAlpha)  // Red with fade
       );
   }
   ```

**Pro tip:** Type the comments first, then press Tab to let Copilot complete

---

#### Phase 4: Test & Review (5 min later)
1. **Build and run:** `cmake --build build --config Release`
2. **Test:**
   - Press `D` to enable debug overlay
   - Press `B` to toggle beat indicator
   - Play music and watch red flash appear on beats
3. **If it works:** ✅ Story complete!
4. **If it doesn't:** Ask Claude to debug:
   ```
   The beat indicator isn't showing. Here's my code:
   [Paste your implementation]
   
   What might be wrong?
   ```

---

## 🎉 Congratulations!

You just completed your first user story using multi-agent AI workflow!

**What you learned:**
- ✅ Gemini for fast exploration
- ✅ Claude for detailed design
- ✅ Copilot for rapid implementation
- ✅ Each AI used for its strengths

---

## 📚 Next Steps

### Today:
1. ✅ Mark story complete in `Vibeus/docs/SPRINT_01.md`
2. ✅ Update velocity tracking
3. ✅ Pick next story (Epic 3.1 - Fix Broken Settings)

### This Week:
1. Complete 2-3 more stories using the same pattern
2. Track which AI works best for which tasks
3. Adjust your workflow based on learnings

### Resources:
- **Full AI Guide:** `Vibeus/docs/AI_AGENTS_GUIDE.md`
- **Release Plan:** `Vibeus/RELEASE_PLAN.md`
- **Sprint Template:** `Vibeus/docs/SPRINT_TEMPLATE.md`

---

## 🛠️ Troubleshooting

### Copilot not suggesting code?
- Check subscription is active
- Try reloading VS Code window
- Make sure you're in a `.cpp` or `.h` file

### Gemini rate limited?
- Free tier is 50 requests/day
- Wait 24 hours or consider paid tier
- Use Claude as backup

### Claude not understanding context?
- Upload more source files to your project
- Provide more code context in your prompts
- Break down complex questions into smaller parts

---

## 💡 Pro Tips

### Speed Up Workflow:
1. **Keep AI tabs open** - Switch between them quickly
2. **Use keyboard shortcuts** - Copilot: Tab (accept), Ctrl+Enter (alternatives)
3. **Save good prompts** - Create a "prompts.txt" file with templates
4. **Batch questions** - Ask Gemini multiple things at once

### Improve AI Responses:
1. **Be specific** - Include file names, line numbers, exact requirements
2. **Provide context** - Paste relevant code snippets
3. **Iterate** - First response not perfect? Ask follow-up questions
4. **Cross-validate** - If unsure, ask 2 different AIs the same question

### Stay Organized:
1. **One story at a time** - Don't juggle too many features
2. **Update sprint tracking daily** - 5-minute habit, huge payoff
3. **Test incrementally** - Don't accumulate untested code
4. **Document learnings** - Note what worked in sprint retrospective

---

## 🚦 When to Use Which AI

### Use Gemini When:
- ❓ You don't know where to start
- 🔍 You need to find something in the codebase
- 📚 You need to research a technology (projectM API, ImGui patterns)
- 📝 You need test plans or documentation

### Use Claude When:
- 🏗️ You're designing a new system
- 🐛 You're debugging a tricky bug
- 🔐 You need security or edge case analysis
- 📖 You need detailed explanations
- 👀 You need thorough code review

### Use Copilot When:
- ⚡ You're typing code and want autocomplete
- 🔁 You're writing repetitive code (loops, JSON serialization)
- 🎨 You're implementing a design someone else created
- 🔧 You need quick boilerplate (getters, setters, constructors)

---

## ✅ Quick Checklist

Before starting your first sprint:
- [ ] GitHub Copilot installed and working in VS Code
- [ ] Gemini AI Studio account created (free)
- [ ] Claude.ai account created with "Vibeus Dev" project
- [ ] Copied `SPRINT_TEMPLATE.md` to `SPRINT_01.md`
- [ ] Selected 3-5 user stories for Sprint 1 (20-25 points)
- [ ] Completed first story (Visual Beat Feedback) as practice
- [ ] Read `AI_AGENTS_GUIDE.md` sections 1-3

**You're ready to build Vibeus at 2-3x velocity!** 🚀

---

**Questions?** Consult `AI_AGENTS_GUIDE.md` or ask Claude: "I'm stuck on [problem], what should I do?"
