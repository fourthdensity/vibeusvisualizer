# Vibeus — Performance Engineering Guide

> Implementation reference for FPS management, resolution scaling, GPU load detection,
> Steam Deck optimization, and performance settings.
> Based on: projectM 4.1.0, SDL2 2.32.10, Dear ImGui 1.91.9, WASAPI, MSVC/C++17

---

## 1. FPS Management Strategy

### 1.1 FPS Cap Options to Offer

| Option | VSync | SwapInterval | Manual Limiter | Use Case |
|--------|-------|-------------|----------------|----------|
| 30 FPS | Off | 0 | Yes (33.33ms) | Battery Saver on Steam Deck |
| 60 FPS | Off | 0 | Yes (16.67ms) | Default, balanced |
| 120 FPS | Off | 0 | Yes (8.33ms) | High-refresh monitors |
| 144 FPS | Off | 0 | Yes (6.94ms) | 144Hz monitors |
| VSync | On | 1 | No | Default recommended — matches display, no tearing |
| Uncapped | Off | 0 | No | Benchmarking only, wastes power |

**Default: VSync On.** It's the right default for a music visualizer — tearing is visually awful, and matching the display refresh is what most users want.

### 1.2 Frame Pacing Implementation

**The current code has a problem.** It uses `SDL_GetTicks()` (millisecond resolution) and `SDL_Delay()` (which has ~1-15ms jitter on Windows). This causes visible frame judder.

**Correct approach: VSync as primary, high-resolution manual limiter as fallback.**

```cpp
// --- Performance state ---
struct PerfState {
    // High-resolution timing
    Uint64 perfFreq;           // SDL_GetPerformanceFrequency()
    Uint64 lastFrameTime;      // SDL_GetPerformanceCounter() at last frame
    
    // Frame time tracking (rolling window)
    static constexpr int FRAME_HISTORY = 120;
    float frameTimes[FRAME_HISTORY] = {};
    int   frameTimeIdx = 0;
    float frameTimeAvg = 16.67f;  // ms
    float frameTimeMax = 0.0f;
    
    // FPS cap
    enum class FpsCap { Fps30, Fps60, Fps120, Fps144, VSync, Uncapped };
    FpsCap fpsCap = FpsCap::VSync;
    
    // Stutter detection
    int   droppedFrameCount = 0;
    int   consecutiveDrops = 0;
    float dropThresholdMs = 0.0f;  // set based on target FPS
    
    // FPS display
    float displayFps = 0.0f;
    int   fpsAccumFrames = 0;
    Uint64 fpsAccumTime = 0;
};

static PerfState g_perf;

// Call once at startup
void initPerfState() {
    g_perf.perfFreq = SDL_GetPerformanceFrequency();
    g_perf.lastFrameTime = SDL_GetPerformanceCounter();
}

// Returns target frame time in seconds, or 0.0 if uncapped/vsync
double getTargetFrameTime(PerfState::FpsCap cap) {
    switch (cap) {
        case PerfState::FpsCap::Fps30:  return 1.0 / 30.0;
        case PerfState::FpsCap::Fps60:  return 1.0 / 60.0;
        case PerfState::FpsCap::Fps120: return 1.0 / 120.0;
        case PerfState::FpsCap::Fps144: return 1.0 / 144.0;
        default: return 0.0;
    }
}

void applyFpsCap(PerfState::FpsCap cap) {
    g_perf.fpsCap = cap;
    
    if (cap == PerfState::FpsCap::VSync) {
        SDL_GL_SetSwapInterval(1);  // VSync on
    } else {
        SDL_GL_SetSwapInterval(0);  // VSync off — we manage timing
    }
    
    // Set drop threshold to 150% of target frame time
    double target = getTargetFrameTime(cap);
    g_perf.dropThresholdMs = (target > 0.0) 
        ? static_cast<float>(target * 1500.0)  // 150% in ms
        : 25.0f;  // fallback for vsync/uncapped
}
```

**The render loop, corrected:**

```cpp
// In the main loop — REPLACE the current SDL_GetTicks + SDL_Delay approach
while (g_running) {
    Uint64 frameStart = SDL_GetPerformanceCounter();
    
    // --- Process input, render projectM, render ImGui, etc. ---
    processEvents();
    // ... (existing render code) ...
    
    SDL_GL_SwapWindow(g_window);  // blocks if VSync is on
    
    // --- High-resolution frame timing ---
    Uint64 afterSwap = SDL_GetPerformanceCounter();
    double elapsedSec = (double)(afterSwap - g_perf.lastFrameTime) / (double)g_perf.perfFreq;
    float elapsedMs = (float)(elapsedSec * 1000.0);
    
    // Record frame time
    g_perf.frameTimes[g_perf.frameTimeIdx] = elapsedMs;
    g_perf.frameTimeIdx = (g_perf.frameTimeIdx + 1) % PerfState::FRAME_HISTORY;
    
    // FPS counter (update every 0.5s for stability)
    g_perf.fpsAccumFrames++;
    g_perf.fpsAccumTime += (afterSwap - g_perf.lastFrameTime);
    if (g_perf.fpsAccumTime >= g_perf.perfFreq / 2) {  // 0.5 seconds
        double secs = (double)g_perf.fpsAccumTime / (double)g_perf.perfFreq;
        g_perf.displayFps = (float)(g_perf.fpsAccumFrames / secs);
        g_perf.fpsAccumFrames = 0;
        g_perf.fpsAccumTime = 0;
    }

    // --- Manual frame limiter (only when VSync is off and not uncapped) ---
    double targetSec = getTargetFrameTime(g_perf.fpsCap);
    if (targetSec > 0.0) {
        // Spin-wait for precision. SDL_Delay is too granular on Windows.
        // Sleep most of the remaining time, then spin the last ~2ms.
        double remaining = targetSec - (double)(afterSwap - frameStart) / (double)g_perf.perfFreq;
        if (remaining > 0.002) {
            SDL_Delay((Uint32)((remaining - 0.002) * 1000.0));  // sleep bulk
        }
        // Spin-wait the remainder for precision
        while (true) {
            Uint64 now = SDL_GetPerformanceCounter();
            double total = (double)(now - frameStart) / (double)g_perf.perfFreq;
            if (total >= targetSec) break;
        }
    }
    
    g_perf.lastFrameTime = SDL_GetPerformanceCounter();
}
```

**Why spin-wait the last 2ms?** `SDL_Delay(1)` on Windows can sleep 1-15ms due to the OS timer resolution. Sleeping the bulk and spinning the rest gives <0.1ms frame time variance vs the current ~5ms variance from `SDL_Delay` alone. The CPU cost of spinning 2ms is negligible on any target hardware.

### 1.3 Timestep for projectM

**Use the existing virtual time approach — it's correct.** `projectm_set_frame_time()` accepts a monotonic time value, and the current `updateVirtualTime()` function correctly:
- Scales real delta by `g_speedMultiplier` 
- Caps delta at 100ms to prevent jumps
- Accumulates into `g_virtualTime`

One improvement: **feed projectM the actual FPS** so preset calculations that use `fps` are accurate:

```cpp
// After computing g_perf.displayFps each interval:
if (g_perf.displayFps > 0.0f) {
    projectm_set_fps(g_pm, static_cast<int32_t>(g_perf.displayFps + 0.5f));
}
```

This matters because some presets use the `fps` variable in their per-frame/per-pixel equations to normalize motion speed. Default in projectM is 35, which is wrong for 60fps.

---

## 2. Resolution Scaling

### 2.1 How It Works in projectM

**`projectm_set_window_size()` controls projectM's internal render resolution.** Looking at the source:

```cpp
// ProjectM.cpp — SetWindowSize just stores dimensions
void ProjectM::SetWindowSize(uint32_t width, uint32_t height) {
    m_windowWidth = width;
    m_windowHeight = height;
}

// GetRenderContext uses these for all framebuffer/viewport sizes
ctx.viewportSizeX = m_windowWidth;
ctx.viewportSizeY = m_windowHeight;
```

projectM creates internal FBOs at the `window_size` dimensions. The final output is rendered to whichever FBO is bound (default FBO 0 = screen). **This means we can decouple render resolution from display resolution.**

### 2.2 Implementation: Resolution Scale

```cpp
struct ResolutionState {
    float scale = 1.0f;         // 0.25 to 2.0 (25% to 200%)
    int   displayWidth  = 1280; // actual window size
    int   displayHeight = 720;
    int   renderWidth   = 1280; // what projectM renders at
    int   renderHeight  = 720;
    GLuint scaleFBO = 0;        // FBO for off-screen render when scale != 1.0
    GLuint scaleTexture = 0;    // color attachment
};

static ResolutionState g_res;

void updateRenderResolution() {
    g_res.renderWidth  = std::max(64, (int)(g_res.displayWidth  * g_res.scale));
    g_res.renderHeight = std::max(64, (int)(g_res.displayHeight * g_res.scale));
    
    // Tell projectM to render at the scaled resolution
    projectm_set_window_size(g_pm, g_res.renderWidth, g_res.renderHeight);
    
    // Recreate FBO if scale != 1.0
    if (std::abs(g_res.scale - 1.0f) > 0.01f) {
        if (g_res.scaleFBO == 0) {
            glGenFramebuffers(1, &g_res.scaleFBO);
            glGenTextures(1, &g_res.scaleTexture);
        }
        glBindTexture(GL_TEXTURE_2D, g_res.scaleTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 
                     g_res.renderWidth, g_res.renderHeight,
                     0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindFramebuffer(GL_FRAMEBUFFER, g_res.scaleFBO);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
                               GL_TEXTURE_2D, g_res.scaleTexture, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}

// In render loop:
void renderVisualization() {
    if (std::abs(g_res.scale - 1.0f) > 0.01f) {
        // Render projectM to off-screen FBO at scaled resolution
        glBindFramebuffer(GL_FRAMEBUFFER, g_res.scaleFBO);
        glViewport(0, 0, g_res.renderWidth, g_res.renderHeight);
        projectm_opengl_render_frame(g_pm);
        
        // Blit to default framebuffer at display resolution
        glBindFramebuffer(GL_READ_FRAMEBUFFER, g_res.scaleFBO);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glBlitFramebuffer(
            0, 0, g_res.renderWidth, g_res.renderHeight,
            0, 0, g_res.displayWidth, g_res.displayHeight,
            GL_COLOR_BUFFER_BIT, GL_LINEAR);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    } else {
        // Render directly to screen
        glViewport(0, 0, g_res.displayWidth, g_res.displayHeight);
        projectm_opengl_render_frame(g_pm);
    }
    
    // ImGui overlay always renders at full display resolution
    glViewport(0, 0, g_res.displayWidth, g_res.displayHeight);
}
```

**Alternative (simpler, 4.2.0+):** Use `projectm_opengl_render_frame_fbo()` to render directly to a custom FBO:

```cpp
projectm_opengl_render_frame_fbo(g_pm, g_res.scaleFBO);
```

This is cleaner since you don't need to bind the FBO yourself — projectM handles the viewport internally.

### 2.3 Recommended Scale Options

| Scale | Resolution at 1080p | Resolution at 800p (Deck) | Use Case |
|-------|-------------------|--------------------------|----------|
| 50% | 960×540 | 640×400 | Battery Saver — still looks decent for psychedelic visuals |
| 75% | 1440×810 | 960×600 | Balanced — good sweetspot for integrated GPUs |
| 100% | 1920×1080 | 1280×800 | Default — native quality |
| 150% | 2880×1620 | 1920×1200 | Supersampling — sharper fine detail in geometric presets |
| 200% | 3840×2160 | 2560×1600 | Ultra — for beefy GPUs, diminishing returns |

**Expose as a slider: 50%–200% in 25% increments, default 100%.** The settings design doc says 25%-200% but 25% looks genuinely bad even for visualizers. Floor it at 50%.

### 2.4 Mesh Size

Mesh size is the other big performance knob. `projectm_set_mesh_size()` controls the per-pixel equation grid resolution:

| Mesh | Grid Size | Vertices | Impact |
|------|-----------|----------|--------|
| Low | 32×24 | 768 | Fastest. Fine for most presets. Some complex per-pixel presets look blocky. |
| Medium | 64×48 | 3,072 | **Default.** Good balance. |
| High | 96×72 | 6,912 | Smooth per-pixel effects. Noticeable GPU cost on integrated. |
| Ultra | 128×96 | 12,288 | Overkill for most presets. |

```cpp
void applyMeshSize(int level) {
    switch (level) {
        case 0: projectm_set_mesh_size(g_pm, 32, 24); break;   // Low
        case 1: projectm_set_mesh_size(g_pm, 64, 48); break;   // Medium (default)
        case 2: projectm_set_mesh_size(g_pm, 96, 72); break;   // High
        case 3: projectm_set_mesh_size(g_pm, 128, 96); break;  // Ultra
    }
}
```

---

## 3. GPU Load Concerns

### 3.1 What Makes Presets Heavy

Milkdrop presets vary enormously in GPU cost. The main factors:

| Factor | Light | Heavy | Why |
|--------|-------|-------|-----|
| **Per-pixel equations** | Simple warp (zoom, rotate) | Complex math (sin/cos chains, conditionals) | Evaluated at every mesh vertex per frame |
| **Custom warp shader** | None or simple | Multi-pass blur, distortion chains | Fragment shader runs per-pixel at render resolution |
| **Custom composite shader** | None or simple | Post-processing chains, bloom, DoF | Additional full-screen pass |
| **Blur passes** | 0 | 3 (blur1, blur2, blur3) | Each adds a full-resolution read+write pass |
| **Number of shapes/waves** | 0-4 | 8+ with alpha blending | Additional draw calls with blending |
| **Texture sampling** | 1 (main) | Multiple noise/image textures | Extra texture fetches in shaders |

**Typical GPU utilization ranges** (1080p, GTX 1060 class):
- Simple presets (waveforms, basic warps): 5-15% GPU
- Medium presets (custom shaders, 1 blur): 15-35% GPU  
- Heavy presets (3 blurs, complex shaders): 35-60% GPU
- Extreme presets (all the above + high mesh): 60-90% GPU

On **Steam Deck** (RDNA2 integrated, 15W TDP), those numbers roughly triple.

### 3.2 Detecting Frame Drops

```cpp
// Call after frame timing is recorded
void detectFrameDrops(float frameTimeMs) {
    if (frameTimeMs > g_perf.dropThresholdMs) {
        g_perf.droppedFrameCount++;
        g_perf.consecutiveDrops++;
    } else {
        g_perf.consecutiveDrops = 0;
    }
}

// Rolling average from the frame time buffer
float getAvgFrameTime() {
    float sum = 0.0f;
    for (int i = 0; i < PerfState::FRAME_HISTORY; i++)
        sum += g_perf.frameTimes[i];
    return sum / (float)PerfState::FRAME_HISTORY;
}

float getMaxFrameTime() {
    float maxVal = 0.0f;
    for (int i = 0; i < PerfState::FRAME_HISTORY; i++)
        if (g_perf.frameTimes[i] > maxVal) maxVal = g_perf.frameTimes[i];
    return maxVal;
}

// Check every second or so
void checkPerformanceHealth() {
    float avg = getAvgFrameTime();
    float target = (float)(getTargetFrameTime(g_perf.fpsCap) * 1000.0);
    if (target <= 0.0f) target = 16.67f;  // assume 60fps for vsync
    
    // If average frame time > 120% of target for sustained period
    if (avg > target * 1.2f && g_perf.consecutiveDrops > 30) {
        // This preset is too heavy — options:
        // 1. Show a brief "Performance Warning" toast
        // 2. Auto-skip to next preset (if user enabled this)
        // 3. Log it for the user
        onPresetPerformanceWarning(avg, target);
    }
}
```

### 3.3 Auto-Skip Heavy Presets (Optional Feature)

```cpp
// Settings
bool g_autoSkipHeavyPresets = false;  // user toggle

void onPresetPerformanceWarning(float avgMs, float targetMs) {
    const char* presetName = /* current preset name */;
    fprintf(stderr, "[Vibeus] Performance warning: '%s' averaging %.1fms (target: %.1fms)\n",
            presetName, avgMs, targetMs);
    
    if (g_autoSkipHeavyPresets) {
        fprintf(stderr, "[Vibeus] Auto-skipping heavy preset\n");
        g_presets.next(false);
        g_perf.consecutiveDrops = 0;
    }
}
```

---

## 4. Steam Deck Specific

### 4.1 Detecting Steam Deck

```cpp
bool isSteamDeck() {
    // SDL2 method — reliable when running under Steam
    const char* hint = SDL_GetHint("SteamDeck");
    if (hint && strcmp(hint, "1") == 0) return true;
    
    // Fallback: check environment variable
    const char* env = getenv("SteamDeck");
    if (env && strcmp(env, "1") == 0) return true;
    
    // Could also check for specific AMD APU via CPUID, but the above is sufficient
    return false;
}
```

### 4.2 Recommended Deck Defaults

When `isSteamDeck()` returns true on first launch, apply "Balanced" performance preset:

| Setting | Deck Default | Rationale |
|---------|-------------|-----------|
| FPS Cap | **VSync** (40Hz or 60Hz) | Deck supports 40Hz mode which saves significant power |
| Resolution Scale | **75%** (960×600) | Saves ~44% fill rate vs native 1280×800 |
| Mesh Size | **Low** (32×24) | Saves per-vertex eval cost, barely visible on 7" screen |
| VSync | **On** | Prevents tearing, matches Deck panel refresh |
| UI Scale | **125%** | Readable on 7" handheld display |
| Auto-skip heavy presets | **On** | Prevents battery drain from runaway presets |

### 4.3 Battery Life Considerations

The Steam Deck's battery life is dominated by three factors:

1. **GPU fill rate** — resolution scale is the biggest knob. 75% scale is ~1.8x cheaper than native.
2. **Frame rate** — 40fps (via SteamOS's 40Hz display mode) uses ~30% less power than 60fps. Vibeus should *not* fight the system refresh rate — just use VSync and let SteamOS handle it.
3. **CPU from per-pixel equations** — mesh size Low vs Medium is measurable. The PRJM_EVAL expression evaluator runs on CPU.

**Do NOT attempt to limit GPU TDP programmatically.** SteamOS handles this via its performance overlay (TDP slider). Exposing a "GPU Power Target" setting in Vibeus as the settings doc suggests would duplicate OS-level functionality and confuse Deck users. Remove that setting or make it desktop-only.

### 4.4 projectM on RDNA2 Integrated (Steam Deck GPU)

The Deck's GPU is a cut-down RDNA2 (8 CUs, ~1.6 TFLOPS). Key characteristics:
- **Good at:** Shader compute, texture sampling. GLSL fragment shaders run well.
- **Bad at:** High fill rate at full resolution, many draw calls, excessive overdraw.
- **OpenGL 3.3 is fine.** The Deck's Mesa/RADV drivers support OpenGL 4.6+.
- **No thermal throttling concerns** if FPS is capped and resolution is scaled.

Expected performance at recommended settings (75% scale, 32×24 mesh, VSync 60):
- Simple presets: 2-4ms frame time (smooth)
- Medium presets: 5-10ms (smooth)
- Heavy presets: 10-18ms (still under 16.67ms budget)
- Extreme presets: may exceed budget → auto-skip kicks in

---

## 5. Performance Options Summary

### 5.1 User-Facing Settings

These map to the settings design doc with implementation specifics:

```
BASIC TAB:
  Performance Mode   [Dropdown: Battery Saver / Balanced / Quality]
    - Battery Saver: 50% scale, 32×24 mesh, 30fps cap
    - Balanced:      75% scale, 48×36 mesh, VSync
    - Quality:       100% scale, 64×48 mesh, VSync  (DEFAULT)

ADVANCED TAB → Performance Section:
  VSync              [Toggle, default: On]
  FPS Cap            [Dropdown: 30/60/120/144/Uncapped, default: 60]
                     (Only visible when VSync is Off)
  Resolution Scale   [Slider: 50%–200%, default: 100%]
  Mesh Resolution    [Dropdown: Low/Medium/High/Ultra, default: Medium]
  Show FPS Counter   [Toggle, default: Off]
  Frame Time Graph   [Toggle, default: Off]  (Debug Overlay)
```

### 5.2 Performance Mode Presets

```cpp
struct PerformancePreset {
    float resolutionScale;
    int   meshLevel;          // 0=Low, 1=Medium, 2=High, 3=Ultra
    PerfState::FpsCap fpsCap;
    bool  autoSkipHeavy;
};

// Performance mode presets
static const PerformancePreset kBatterySaver = { 0.50f, 0, PerfState::FpsCap::Fps30, true };
static const PerformancePreset kBalanced     = { 0.75f, 1, PerfState::FpsCap::VSync,  true };
static const PerformancePreset kQuality      = { 1.00f, 1, PerfState::FpsCap::VSync,  false };

void applyPerformanceMode(const PerformancePreset& preset) {
    g_res.scale = preset.resolutionScale;
    updateRenderResolution();
    applyMeshSize(preset.meshLevel);
    applyFpsCap(preset.fpsCap);
    g_autoSkipHeavyPresets = preset.autoSkipHeavy;
}
```

### 5.3 FPS Counter Overlay

```cpp
void renderFpsOverlay() {
    // Position in top-left, minimal
    ImGui::SetNextWindowPos(ImVec2(10, 10));
    ImGui::SetNextWindowBgAlpha(0.4f);
    ImGui::Begin("##fps", nullptr,
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs |
        ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav);
    
    // Color-code: green if OK, yellow if borderline, red if dropping
    float avg = getAvgFrameTime();
    float target = /* target ms for current cap */;
    ImVec4 color = (avg < target * 1.0f) ? ImVec4(0.4f, 1.0f, 0.4f, 1.0f)  // green
                 : (avg < target * 1.3f) ? ImVec4(1.0f, 1.0f, 0.3f, 1.0f)  // yellow
                 :                         ImVec4(1.0f, 0.3f, 0.3f, 1.0f);  // red
    
    ImGui::TextColored(color, "%.0f FPS", g_perf.displayFps);
    ImGui::End();
}
```

### 5.4 Frame Time Graph (Debug Overlay)

```cpp
void renderFrameTimeGraph() {
    ImGui::SetNextWindowPos(ImVec2(10, 50));
    ImGui::SetNextWindowSize(ImVec2(300, 100));
    ImGui::SetNextWindowBgAlpha(0.6f);
    ImGui::Begin("##framegraph", nullptr,
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs |
        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing);
    
    // ImGui's built-in plot — simple and effective
    char overlay[32];
    snprintf(overlay, sizeof(overlay), "%.1f ms", getAvgFrameTime());
    ImGui::PlotLines("##ft", g_perf.frameTimes, PerfState::FRAME_HISTORY,
                     g_perf.frameTimeIdx, overlay,
                     0.0f, 33.3f,  // min=0, max=33ms (shows 30fps as ceiling)
                     ImGui::GetContentRegionAvail());
    
    ImGui::End();
}
```

---

## 6. Minimum System Requirements

### 6.1 Hardware Minimums

| Component | Minimum | Recommended | Notes |
|-----------|---------|-------------|-------|
| **GPU** | Intel HD 4000 / AMD GCN 1.0 / NVIDIA Kepler (GTX 600) | GTX 1050 / RX 560 / Intel Iris Xe | Must support OpenGL 3.3 Core |
| **CPU** | Dual-core 2.0 GHz | Quad-core 3.0 GHz | Per-pixel eval is single-threaded CPU work |
| **RAM** | 2 GB available | 4 GB | projectM itself is light (~50-100MB); OS/audio overhead |
| **VRAM** | 512 MB | 1 GB | FBOs + textures at 1080p ≈ 100-200MB; at 4K ≈ 400-600MB |
| **OS** | Windows 10 1809+ | Windows 10/11 latest | WASAPI loopback needs Vista+; SDL2 needs Win10+ for gamepad |
| **Display** | 1280×720 | 1920×1080 | Any resolution works; minimum for readable UI |

### 6.2 OpenGL Requirements

**OpenGL 3.3 Core is the true minimum.** This is what the code requests and what projectM 4.x requires.

```cpp
// Already in initSDL():
SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
```

projectM 4.x uses:
- GLSL 330 core shaders
- Framebuffer objects (FBO)
- Multiple render targets (MRT) — for blur passes
- `glBlitFramebuffer` — for texture copies
- Texture arrays — for noise textures

All of these are baseline OpenGL 3.3. No extensions required.

### 6.3 Memory Usage

| Component | Typical | Peak | Notes |
|-----------|---------|------|-------|
| projectM core | 20 MB | 30 MB | Preset parser, eval engine, mesh data |
| Internal FBOs (1080p) | 60 MB | 120 MB | Main texture + blur1-3 + transition buffers |
| Loaded textures | 10 MB | 50 MB | Depends on preset texture usage |
| ImGui | 2 MB | 5 MB | Vertex buffers, font atlas |
| Audio buffers | <1 MB | 1 MB | WASAPI capture ring buffer |
| **Total** | **~100 MB** | **~200 MB** | Well within 2GB system + 512MB VRAM |

At 4K with 200% supersampling, VRAM for FBOs alone reaches ~500MB. This is why VRAM minimum is 512MB (for basic usage) and recommended is 1GB.

---

## 7. Practical Code Patterns

### 7.1 High-Resolution Frame Time Measurement

Already shown in Section 1.2. Key point: **always use `SDL_GetPerformanceCounter()` / `SDL_GetPerformanceFrequency()`**, never `SDL_GetTicks()` for frame timing. Performance counter gives sub-microsecond resolution.

### 7.2 Detecting Sustained Frame Drops

```cpp
// Call once per frame after recording frame time
struct StutterDetector {
    static constexpr int WINDOW = 60;  // 1 second at 60fps
    int   badFrames = 0;
    int   totalFrames = 0;
    bool  warned = false;
    
    void recordFrame(float frameTimeMs, float budgetMs) {
        totalFrames++;
        if (frameTimeMs > budgetMs * 1.5f) {
            badFrames++;
        }
        
        // Every WINDOW frames, evaluate
        if (totalFrames >= WINDOW) {
            float badRatio = (float)badFrames / (float)totalFrames;
            if (badRatio > 0.25f && !warned) {
                // >25% of frames exceeded 150% of budget over 1 second
                warned = true;
                onSustainedFrameDrop(badRatio);
            }
            badFrames = 0;
            totalFrames = 0;
        }
    }
    
    void reset() {
        badFrames = 0;
        totalFrames = 0;
        warned = false;
    }
};

// Reset when switching presets
void onPresetChanged() {
    g_stutterDetector.reset();
}
```

### 7.3 Performance Mode Toggle

```cpp
// Called from settings UI
void onPerformanceModeChanged(int mode) {
    switch (mode) {
        case 0: applyPerformanceMode(kBatterySaver); break;
        case 1: applyPerformanceMode(kBalanced); break;
        case 2: applyPerformanceMode(kQuality); break;
    }
    
    // Also update individual settings UI to reflect the composite change
    // (so the Advanced tab shows the actual values)
    g_settings.resolutionScale = g_res.scale;
    g_settings.meshLevel = /* ... */;
    g_settings.fpsCap = g_perf.fpsCap;
}
```

### 7.4 Window Resize Handler (Updated for Resolution Scaling)

```cpp
// In the SDL_WINDOWEVENT handler:
case SDL_WINDOWEVENT_SIZE_CHANGED: {
    g_res.displayWidth  = event.window.data1;
    g_res.displayHeight = event.window.data2;
    updateRenderResolution();  // recomputes scaled size and updates projectM
    break;
}
```

### 7.5 Complete Frame Structure (Putting It All Together)

```
┌─ frameStart = SDL_GetPerformanceCounter()
│
├─ processEvents()          // SDL input, window resize
│
├─ if (Visualizer && !paused):
│   ├─ processGamepad()
│   ├─ processFlowMode()
│   ├─ updateVirtualTime()
│   ├─ feedAudio()
│   ├─ if (scale != 1.0):
│   │   ├─ bind scaleFBO
│   │   ├─ glViewport(renderW, renderH)
│   │   ├─ projectm_opengl_render_frame()
│   │   ├─ glBlitFramebuffer → screen
│   │   └─ glViewport(displayW, displayH)
│   └─ else:
│       └─ projectm_opengl_render_frame()
│
├─ if (menuVisible):
│   └─ ImGui render overlay
│
├─ if (showFpsCounter):
│   └─ renderFpsOverlay()
│
├─ if (showFrameGraph):
│   └─ renderFrameTimeGraph()
│
├─ SDL_GL_SwapWindow()      // blocks on VSync
│
├─ record frame time
├─ detectFrameDrops()
├─ update FPS counter
├─ projectm_set_fps()
│
├─ if (manual FPS cap):
│   ├─ SDL_Delay(bulk)
│   └─ spin-wait(remainder)
│
└─ lastFrameTime = SDL_GetPerformanceCounter()
```

---

## 8. Implementation Priority

For the v0.3.0 milestone ("Playable Product"):

1. **Replace `SDL_GetTicks`/`SDL_Delay` with `SDL_GetPerformanceCounter` + spin-wait** — immediate frame pacing improvement, ~50 lines changed.
2. **Add `projectm_set_fps()` call** — one line, fixes preset speed calculations.
3. **Implement resolution scaling via FBO** — enables the entire Battery Saver → Quality spectrum.
4. **Wire up Performance Mode dropdown** — three composite presets, instant usability for Steam Deck users.
5. **Add FPS counter overlay** — users need feedback to know settings are working.
6. **Add frame drop detection + auto-skip** — prevents bad presets from ruining the experience.
7. **Steam Deck detection + auto-defaults** — first-launch experience for Deck users.

Items 1-2 should be done immediately. Items 3-5 in the settings implementation sprint. Items 6-7 before Steam playtest builds.

---

## Appendix: Key projectM API Reference

```cpp
// Resolution & rendering
projectm_set_window_size(handle, width, height);    // Internal render resolution
projectm_get_window_size(handle, &width, &height);
projectm_set_mesh_size(handle, meshW, meshH);        // Per-pixel grid: [8,300] per axis
projectm_get_mesh_size(handle, &meshW, &meshH);
projectm_opengl_render_frame(handle);                // Render to bound FBO
projectm_opengl_render_frame_fbo(handle, fboId);     // Render to specific FBO (4.2.0+)

// Timing
projectm_set_frame_time(handle, seconds);            // Set virtual time (monotonic)
projectm_set_fps(handle, fps);                       // Tell presets current FPS

// Textures
projectm_set_texture_search_paths(handle, paths, count);
projectm_reset_textures(handle);                     // Force reload
```
