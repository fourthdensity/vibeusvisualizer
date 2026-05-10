#include "IVisualizer.h"
#include "ProjectMVisualizer.h"
#include "VibeusVisualizer.h"

#include "audio_capture.h"
#include "preset_manager.h"
#include "preset_database.h"
#include "storyteller.h"
#include "menu_overlay.h"
#include "config.h"

#include <imgui.h>

#include <cstdio>
#include <cstdlib>
#include <string>
#include <cstring>
#include <filesystem>
#include <vector>
#include <memory>

namespace fs = std::filesystem;

// ----- Configuration -----
static constexpr int    DEFAULT_WIDTH  = 1280;
static constexpr int    DEFAULT_HEIGHT = 720;
static constexpr int    TARGET_FPS     = 60;
static constexpr double PRESET_DURATION    = 30.0;
static constexpr double SOFT_CUT_DURATION  = 3.0;
static constexpr double HARD_CUT_DURATION  = 15.0;
static constexpr float  BEAT_SENSITIVITY   = 1.0f;

// ----- Globals -----
static SDL_Window*   g_window   = nullptr;
static SDL_GLContext  g_glCtx   = nullptr;
// Replaced projectm_handle with abstraction layer
static std::unique_ptr<IVisualizer> g_visualizer;
static bool          g_running  = true;
static bool          g_fullscreen = false;
static bool          g_debug    = false;
static bool          g_inStasis = false;

static AudioCapture  g_audio;
static PresetManager g_presets;
static MenuOverlay   g_menu;
static PresetDatabase g_presetDb;
Storyteller g_storyteller;

// Configuration (persisted to JSON)
static VibeusConfig  g_config;
static std::string   g_userDataDir;
static std::string   g_configPath;
static std::string   g_favoritesPath;
static std::string   g_brokenPresetBlacklistPath;
static std::string   g_userBlacklistPath;

enum class AppState { Splash, MainMenu, Visualizer };
static AppState g_appState = AppState::Splash;

static bool          g_paused    = false;
static double        g_pausedTime = 0.0;
static int           g_pauseRenderCount = 0;

static bool          g_showBeatIndicator = false;
static float         g_beatIndicatorAlpha = 0.0f;

static Uint32 g_frameCount  = 0;
static Uint32 g_fpsTimer    = 0;
static float  g_currentFps  = 0.0f;

static double g_speedMultiplier = 1.0;
static double g_virtualTime     = 0.0;
static Uint32 g_lastFrameTicks  = 0;

static float  g_audioGain       = 1.0f;

static bool   g_flashLimiter    = false;
static bool   g_reducedMotion   = false;

static void setBeatSensitivity(float value, const char* reason);

static bool   g_mouseDown       = false;
static int    g_touchPressure   = 1;
static int    g_touchTypeIndex  = 0;
static constexpr projectm_touch_type g_touchTypes[] = {
    PROJECTM_TOUCH_TYPE_RANDOM,
    PROJECTM_TOUCH_TYPE_CIRCLE,
    PROJECTM_TOUCH_TYPE_RADIAL_BLOB,
    PROJECTM_TOUCH_TYPE_BLOB2,
    PROJECTM_TOUCH_TYPE_BLOB3,
    PROJECTM_TOUCH_TYPE_DERIVATIVE_LINE,
    PROJECTM_TOUCH_TYPE_BLOB5,
    PROJECTM_TOUCH_TYPE_LINE,
    PROJECTM_TOUCH_TYPE_DOUBLE_LINE,
};
static constexpr int g_touchTypeCount = sizeof(g_touchTypes) / sizeof(g_touchTypes[0]);

static SDL_GameController* g_gamepad = nullptr;

static bool  g_flowMode = false;

struct Ripple {
    float cx, cy;
    Uint32 startMs;
    float durationSec;
    int   ringCount;
    int   pointsPerRing;
    float maxRadius;
    projectm_touch_type type;
};

static std::vector<Ripple> g_ripples;

static int    g_prevMouseX = 0, g_prevMouseY = 0;
static Uint32 g_prevMouseMs = 0;

static void spawnRipple(float nx, float ny, float intensity = 1.0f)
{
    Ripple r;
    r.cx         = nx;
    r.cy         = ny;
    r.startMs    = SDL_GetTicks();
    r.durationSec = 0.5f * intensity;
    r.ringCount   = static_cast<int>(4 * intensity);
    if (r.ringCount < 2) r.ringCount = 2;
    r.pointsPerRing = 8;
    r.maxRadius   = 0.12f * intensity;
    r.type        = g_touchTypes[g_touchTypeIndex];
    g_ripples.push_back(r);
}

static void processRipples()
{
    if (g_ripples.empty()) return;
    Uint32 now = SDL_GetTicks();

    for (auto it = g_ripples.begin(); it != g_ripples.end(); ) {
        float elapsed = (now - it->startMs) / 1000.0f;
        if (elapsed >= it->durationSec) {
            it = g_ripples.erase(it);
            continue;
        }

        float progress = elapsed / it->durationSec;
        int currentRing = static_cast<int>(progress * it->ringCount);

        static int lastRing[64] = {};
        size_t idx = static_cast<size_t>(it - g_ripples.begin());
        if (idx < 64 && currentRing > lastRing[idx]) {
            lastRing[idx] = currentRing;

            float radius = it->maxRadius * progress;
            int pressure = (progress < 0.3f) ? 2 : (progress < 0.7f) ? 1 : 0;

            for (int i = 0; i < it->pointsPerRing; i++) {
                float angle = (2.0f * 3.14159265f * i) / it->pointsPerRing;
                float px = it->cx + cosf(angle) * radius;
                float py = it->cy + sinf(angle) * radius;
                if (px >= 0.0f && px <= 1.0f && py >= 0.0f && py <= 1.0f)
                    // Use abstraction if available
                    if (auto* pmViz = dynamic_cast<ProjectMVisualizer*>(g_visualizer.get())) {
                        projectm_touch(pmViz->getHandle(), px, py, pressure, it->type);
                    }
            }
        }
        ++it;
    }
}

static void processVelocityTrail(int mx, int my)
{
    Uint32 now = SDL_GetTicks();
    float dt = (now - g_prevMouseMs) / 1000.0f;
    if (dt < 0.001f) dt = 0.001f;

    float dx = static_cast<float>(mx - g_prevMouseX);
    float dy = static_cast<float>(my - g_prevMouseY);
    float speed = sqrtf(dx * dx + dy * dy) / dt;

    g_prevMouseX = mx;
    g_prevMouseY = my;
    g_prevMouseMs = now;

    if (speed < 50.0f) return;

    float len = sqrtf(dx * dx + dy * dy);
    if (len < 1.0f) return;
    float ndx = dx / len, ndy = dy / len;

    float px = -ndy, py = ndx;

    int w, h;
    SDL_GetWindowSize(g_window, &w, &h);
    float cnx = static_cast<float>(mx) / w;
    float cny = static_cast<float>(my) / h;

    float spread = fminf(speed / 2000.0f, 0.08f);
    int trailPoints = (speed > 400.0f) ? 4 : 2;
    int pressure = (speed > 800.0f) ? 2 : 1;

    for (int i = -trailPoints; i <= trailPoints; i++) {
        if (i == 0) continue;
        float offset = spread * (static_cast<float>(i) / trailPoints);
        float tx = cnx + px * offset;
        float ty = cny + py * offset;
        if (tx >= 0.0f && tx <= 1.0f && ty >= 0.0f && ty <= 1.0f)
            if (auto* pmViz = dynamic_cast<ProjectMVisualizer*>(g_visualizer.get())) {
                projectm_touch(pmViz->getHandle(), tx, ty, pressure, g_touchTypes[g_touchTypeIndex]);
            }
    }
}

static const char* touchTypeName(projectm_touch_type t)
{
    switch (t) {
    case PROJECTM_TOUCH_TYPE_RANDOM:          return "Random";
    case PROJECTM_TOUCH_TYPE_CIRCLE:          return "Circle";
    case PROJECTM_TOUCH_TYPE_RADIAL_BLOB:     return "Radial Blob";
    case PROJECTM_TOUCH_TYPE_BLOB2:           return "Blob 2";
    case PROJECTM_TOUCH_TYPE_BLOB3:           return "Blob 3";
    case PROJECTM_TOUCH_TYPE_DERIVATIVE_LINE: return "Derivative Line";
    case PROJECTM_TOUCH_TYPE_BLOB5:           return "Blob 5";
    case PROJECTM_TOUCH_TYPE_LINE:            return "Line";
    case PROJECTM_TOUCH_TYPE_DOUBLE_LINE:     return "Double Line";
    default: return "???";
    }
}

static const char* logLevelStr(projectm_log_level level)
{
    switch (level) {
    case PROJECTM_LOG_LEVEL_TRACE: return "TRACE";
    case PROJECTM_LOG_LEVEL_DEBUG: return "DEBUG";
    case PROJECTM_LOG_LEVEL_INFO:  return "INFO";
    case PROJECTM_LOG_LEVEL_WARN:  return "WARN";
    case PROJECTM_LOG_LEVEL_ERROR: return "ERROR";
    case PROJECTM_LOG_LEVEL_FATAL: return "FATAL";
    default: return "???";
    }
}

bool g_lastPresetHadError = false;
bool g_captureProjectMErrors = false;
FILE* g_validationLogFile = nullptr;

static void projectmLogCallback(const char* message, projectm_log_level level, void* /*userData*/)
{
    fprintf(stderr, "[projectM/%s] %s\n", logLevelStr(level), message);
    if (g_validationLogFile) {
        fprintf(g_validationLogFile, "[projectM/%s] %s\n", logLevelStr(level), message);
        fflush(g_validationLogFile);
    }
    
    if (g_captureProjectMErrors && level == PROJECTM_LOG_LEVEL_ERROR) {
        g_lastPresetHadError = true;
    }
}

static void updateDebugTitle()
{
    if (!g_debug && !g_config.storyDebug) return;
    std::string preset = g_presets.currentPresetName();
    auto pos = preset.find_last_of("/\\");
    if (pos != std::string::npos)
        preset = preset.substr(pos + 1);

    char title[512];
    if (g_config.storyDebug) {
        const char* s = (g_storyteller.currentState() == StoryState::Chill) ? "CHILL" : 
                        (g_storyteller.currentState() == StoryState::Buildup) ? "BUILD" :
                        (g_storyteller.currentState() == StoryState::Drop) ? "DROP" : "SUSTAIN";
        snprintf(title, sizeof(title),
                 "Vibeus [STORY] | %.0f FPS | State: %s | Kick: %.2f | Thresh: %.2f | Flux: %.2f | Trg: %s | P[%u/%u] %s",
                 g_currentFps, s, 
                 g_audio.levelBeatBass(), g_storyteller.beatThreshold(), g_storyteller.spectralFlux(),
                 g_storyteller.beatActive() ? "YES" : " no",
                 g_presets.position() + 1, g_presets.count(), preset.c_str());
    } else {
        float rms = g_audio.levelRms();
        float peak = g_audio.levelPeak();
        snprintf(title, sizeof(title),
                 "Vibeus | %.0f FPS | %.2fx speed | %.0f%% gain | In %.2f pk %.2f | Touch: %s P%d | %s%s[%u/%u] %s",
                 g_currentFps, g_speedMultiplier, g_audioGain * 100.0f,
                 rms, peak,
                 touchTypeName(g_touchTypes[g_touchTypeIndex]), g_touchPressure,
                 g_flowMode ? "FLOW | " : "",
                 g_gamepad ? "GAMEPAD | " : "",
                 g_presets.position() + 1, g_presets.count(), preset.c_str());
    }
    SDL_SetWindowTitle(g_window, title);
}

static void adjustSpeed(double delta)
{
    g_speedMultiplier += delta;
    if (g_speedMultiplier < 0.05) g_speedMultiplier = 0.05;
    if (g_speedMultiplier > 4.0)  g_speedMultiplier = 4.0;
    fprintf(stderr, "[Vibeus] Speed: %.2fx\n", g_speedMultiplier);
}

static void resetSpeed()
{
    g_speedMultiplier = 1.0;
    fprintf(stderr, "[Vibeus] Speed reset to 1.0x\n");
}

static void updateVirtualTime(Uint32 nowTicks)
{
    double realDelta = (nowTicks - g_lastFrameTicks) / 1000.0;
    g_lastFrameTicks = nowTicks;
    if (realDelta > 0.1) realDelta = 0.1;
    double effectiveSpeed = g_speedMultiplier * (g_reducedMotion ? 0.5 : 1.0);
    if (effectiveSpeed < 0.05) effectiveSpeed = 0.05;
    g_virtualTime += realDelta * effectiveSpeed;
    if (g_visualizer) g_visualizer->setFrameTime(g_virtualTime);
}

static void enterPause()
{
    g_paused = true;
    g_pausedTime = g_virtualTime;
    g_pauseRenderCount = 0;
    g_menu.showScreen(UIScreen::PauseMenu);
    fprintf(stderr, "[Vibeus] Paused at virtual time %.3f\n", g_pausedTime);
}

static void resumeFromPause()
{
    g_paused = false;
    g_menu.hideAll();
    g_virtualTime = g_pausedTime;
    g_lastFrameTicks = SDL_GetTicks();
    fprintf(stderr, "[Vibeus] Resumed\n");
}

static void adjustAudioGain(float delta)
{
    g_audioGain += delta;
    if (g_audioGain < 0.0f) g_audioGain = 0.0f;
    if (g_audioGain > 3.0f) g_audioGain = 3.0f;
    fprintf(stderr, "[Vibeus] Audio gain: %.0f%%\n", g_audioGain * 100.0f);
}

static void resetAudioGain()
{
    g_audioGain = 1.0f;
    fprintf(stderr, "[Vibeus] Audio gain reset to 100%%\n");
}

static void screenToNormalized(int sx, int sy, float& nx, float& ny)
{
    int w, h;
    SDL_GetWindowSize(g_window, &w, &h);
    nx = static_cast<float>(sx) / static_cast<float>(w);
    ny = static_cast<float>(sy) / static_cast<float>(h);
}

static void tryOpenGamepad()
{
    if (g_gamepad) return;
    for (int i = 0; i < SDL_NumJoysticks(); i++) {
        if (SDL_IsGameController(i)) {
            g_gamepad = SDL_GameControllerOpen(i);
            if (g_gamepad) {
                fprintf(stderr, "[Vibeus] Gamepad connected: %s\n",
                        SDL_GameControllerName(g_gamepad));
                return;
            }
        }
    }
}

static float stickAxis(SDL_GameControllerAxis axis)
{
    if (!g_gamepad) return 0.0f;
    int raw = SDL_GameControllerGetAxis(g_gamepad, axis);
    if (abs(raw) < g_config.gamepadDeadzone) return 0.0f;
    return static_cast<float>(raw) / 32768.0f;
}

static void processGamepad()
{
    if (!g_gamepad) return;
    if (g_paused || g_appState != AppState::Visualizer) return;

    float lt = static_cast<float>(SDL_GameControllerGetAxis(g_gamepad, SDL_CONTROLLER_AXIS_TRIGGERLEFT)) / 32768.0f;
    float rt = static_cast<float>(SDL_GameControllerGetAxis(g_gamepad, SDL_CONTROLLER_AXIS_TRIGGERRIGHT)) / 32768.0f;
    if (lt > 0.1f || rt > 0.1f) {
        double targetSpeed = 1.0 + (rt - lt) * 1.5;
        g_speedMultiplier += (targetSpeed - g_speedMultiplier) * 0.08;
        if (g_speedMultiplier < 0.05) g_speedMultiplier = 0.05;
        if (g_speedMultiplier > 4.0)  g_speedMultiplier = 4.0;
    }

    if (false && g_config.touchEnabled) {
        float rx = stickAxis(SDL_CONTROLLER_AXIS_RIGHTX);
        float ry = stickAxis(SDL_CONTROLLER_AXIS_RIGHTY);
        if (fabsf(rx) > 0.0f || fabsf(ry) > 0.0f) {
            float nx = 0.5f + rx * 0.5f;
            float ny = 0.5f + ry * 0.5f;
            float magnitude = sqrtf(rx * rx + ry * ry);
            int pressure = (magnitude > 0.7f) ? 2 : (magnitude > 0.3f) ? 1 : 0;
            if (auto* pmViz = dynamic_cast<ProjectMVisualizer*>(g_visualizer.get())) {
                projectm_touch(pmViz->getHandle(), nx, ny, pressure, g_touchTypes[g_touchTypeIndex]);
            }
        }
    }

    float lx = stickAxis(SDL_CONTROLLER_AXIS_LEFTX);
    if (fabsf(lx) > 0.0f) {
        g_speedMultiplier += lx * 0.02;
        if (g_speedMultiplier < 0.05) g_speedMultiplier = 0.05;
        if (g_speedMultiplier > 4.0)  g_speedMultiplier = 4.0;
    }
}

static void processFlowMode()
{
    if (false || !g_flowMode || !g_config.touchEnabled || g_paused || g_appState != AppState::Visualizer) return;

    int mx, my;
    SDL_GetMouseState(&mx, &my);
    int w, h;
    SDL_GetWindowSize(g_window, &w, &h);

    float nx = static_cast<float>(mx) / static_cast<float>(w);
    float ny = static_cast<float>(my) / static_cast<float>(h);

    double targetSpeed = 0.2 + nx * 2.8;
    g_speedMultiplier += (targetSpeed - g_speedMultiplier) * 0.05;
    if (g_speedMultiplier < 0.05) g_speedMultiplier = 0.05;
    if (g_speedMultiplier > 4.0)  g_speedMultiplier = 4.0;

    int interval = static_cast<int>(15.0f - ny * 12.0f);
    if (interval < 2) interval = 2;
    static int flowFrameCount = 0;
    flowFrameCount++;
    if (flowFrameCount >= interval) {
        flowFrameCount = 0;
        int pressure = static_cast<int>(ny * 2.0f);
        if (pressure > 2) pressure = 2;
        if (auto* pmViz = dynamic_cast<ProjectMVisualizer*>(g_visualizer.get())) {
            projectm_touch(pmViz->getHandle(), nx, ny, pressure, g_touchTypes[g_touchTypeIndex]);
        }
    }
}

#ifdef _WIN32
static void attachConsole()
{
    if (AttachConsole(ATTACH_PARENT_PROCESS) || AllocConsole()) {
        FILE* dummy;
        freopen_s(&dummy, "CONOUT$", "w", stdout);
        freopen_s(&dummy, "CONOUT$", "w", stderr);
    }
}
#endif

static std::string getUserDataDir()
{
    char* p = SDL_GetPrefPath("VibeusProject", "Vibeus");
    if (!p) {
        const char* b = SDL_GetBasePath();
        return b ? fs::path(b).string() : std::string(".");
    }
    std::string result(p);
    SDL_free(p);
    return result;
}

static std::string findPresetsDir()
{
    const char* base = SDL_GetBasePath();
    auto exePath = fs::path(base ? base : "");
    std::string candidates[] = {
        (exePath / "presets").string(),
        (exePath / ".." / "presets").string(),
        (exePath / ".." / ".." / "presets").string(),
        "presets",
    };
    for (auto& path : candidates) {
        if (fs::exists(path) && fs::is_directory(path))
            return path;
    }
    return "presets";
}

static bool initSDL()
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0) {
        fprintf(stderr, "[Vibeus] SDL_Init failed: %s\n", SDL_GetError());
        return false;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    g_window = SDL_CreateWindow(
        "Vibeus",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        DEFAULT_WIDTH, DEFAULT_HEIGHT,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN);
    if (!g_window) {
        fprintf(stderr, "[Vibeus] SDL_CreateWindow failed: %s\n", SDL_GetError());
        return false;
    }

    g_glCtx = SDL_GL_CreateContext(g_window);
    if (!g_glCtx) {
        fprintf(stderr, "[Vibeus] SDL_GL_CreateContext failed: %s\n", SDL_GetError());
        return false;
    }

    SDL_GL_MakeCurrent(g_window, g_glCtx);
    SDL_GL_SetSwapInterval(1);

    return true;
}

// Updated init to use abstraction layer
static bool initVisualizer()
{
    // Default to ProjectM for full compatibility
    // To use custom backend: g_visualizer = std::make_unique<VibeusVisualizer>();
    g_visualizer = std::make_unique<ProjectMVisualizer>();
    
    if (!g_visualizer->initialize(DEFAULT_WIDTH, DEFAULT_HEIGHT)) {
        fprintf(stderr, "[Vibeus] Visualizer initialization failed\n");
        return false;
    }

    fprintf(stderr, "[Vibeus] %s initialized\n", g_visualizer->getBackendInfo().c_str());

    std::string presetsDir = findPresetsDir();

    uint32_t dbCount = g_presetDb.loadFromDirectory(presetsDir);
    fprintf(stderr, "[Vibeus] PresetDatabase loaded %u presets across %zu categories\n",
            dbCount, g_presetDb.categoryCount());

    const char* base = SDL_GetBasePath();
    auto exeDir = fs::path(base ? base : "");
    std::string texturesDir = (exeDir / "textures").string();

    std::vector<const char*> texPaths;
    if (fs::exists(texturesDir) && fs::is_directory(texturesDir))
        texPaths.push_back(texturesDir.c_str());
    texPaths.push_back(presetsDir.c_str());
    g_visualizer->setTextureSearchPaths(texPaths.data(), texPaths.size());

    return true;
}

static void toggleFullscreen()
{
    g_fullscreen = !g_fullscreen;
    SDL_SetWindowFullscreen(g_window,
        g_fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);

    int w, h;
    SDL_GetWindowSize(g_window, &w, &h);
    if (g_visualizer) g_visualizer->setWindowSize(w, h);
    fprintf(stderr, "[Vibeus] %s (%dx%d)\n",
            g_fullscreen ? "Fullscreen" : "Windowed", w, h);
}

static void toastInput(const char* msg, float durationSec = 0.8f)
{
    static Uint32 lastMs = 0;
    static std::string lastMsg;
    Uint32 now = SDL_GetTicks();

    if (msg && lastMsg == msg && (now - lastMs) < 250)
        return;

    lastMsg = msg ? msg : "";
    lastMs = now;

    if (msg && msg[0])
        g_menu.showToast(msg, durationSec);
}

static void handleKeyDown(SDL_Keysym key)
{
    if (g_debug)
        fprintf(stderr, "[Vibeus] Key: %s (0x%x)\n", SDL_GetKeyName(key.sym), key.sym);

    const SDL_Keycode sym = key.sym;

    if (sym == g_config.keyQuit) {
        toastInput("Exiting...");
        g_running = false;
        return;
    }

    if (sym == g_config.keyNextPreset || sym == SDLK_RIGHT) {
        g_presets.next(false);
        toastInput("Next preset");
        return;
    }
    if (sym == g_config.keyPrevPreset || sym == SDLK_LEFT) {
        g_presets.previous(false);
        toastInput("Previous preset");
        return;
    }
    if (sym == g_config.keyRandomPreset) {
        g_presets.next(true);
        toastInput("Random preset");
        return;
    }
    if (sym == g_config.keyHistory) {
        g_presets.last(false);
        toastInput("History back");
        return;
    }
    if (sym == g_config.keyShuffle) {
        g_presets.toggleShuffle();
        toastInput(g_presets.isShuffled() ? "Shuffle ON" : "Shuffle OFF");
        return;
    }

    if (sym == SDLK_b) {
        std::string removed = g_presets.blacklistCurrent(g_userBlacklistPath);
        if (!removed.empty()) {
            toastInput(("Blacklisted: " + removed).c_str());
        }
        return;
    }

    if (sym == SDLK_f) {
        uint32_t currentIdx = g_presets.position();
        g_menu.toggleFavorite(currentIdx);
        bool nowFav = g_menu.isFavorite(currentIdx);
        toastInput(nowFav ? "★ Added to Favorites" : "☆ Removed from Favorites");
        return;
    }

    if (sym == SDLK_q) {
        std::string removed = g_presets.blacklistCurrent(g_userBlacklistPath);
        if (!removed.empty()) {
            toastInput(("Quarantined (black screen): " + removed).c_str());
        }
        return;
    }

    if (sym == g_config.keyDebug) {
        g_debug = !g_debug;
        if (!g_debug && !g_config.storyDebug)
            SDL_SetWindowTitle(g_window, "Vibeus");
        else
            updateDebugTitle();
        fprintf(stderr, "[Vibeus] Debug display: %s\n", g_debug ? "ON" : "OFF");
        toastInput(g_debug ? "Debug ON" : "Debug OFF");
        return;
    }

    if (sym == SDLK_l) {
        g_config.storyDebug = !g_config.storyDebug;
        if (!g_debug && !g_config.storyDebug)
            SDL_SetWindowTitle(g_window, "Vibeus");
        else
            updateDebugTitle();
        fprintf(stderr, "[Story] Real-time telemetry: %s\n", g_config.storyDebug ? "ON" : "OFF");
        toastInput(g_config.storyDebug ? "Story Logs ON" : "Story Logs OFF");
        return;
    }

    if (sym == SDLK_i) {
        g_showBeatIndicator = !g_showBeatIndicator;
        toastInput(g_showBeatIndicator ? "Beat Indicator ON" : "Beat Indicator OFF");
        return;
    }

    if (sym == g_config.keySpeedDown) {
        adjustSpeed(-0.05);
        char buf[64];
        snprintf(buf, sizeof(buf), "Speed %.2fx", g_speedMultiplier);
        toastInput(buf, 0.7f);
        return;
    }
    if (sym == g_config.keySpeedUp) {
        adjustSpeed(+0.05);
        char buf[64];
        snprintf(buf, sizeof(buf), "Speed %.2fx", g_speedMultiplier);
        toastInput(buf, 0.7f);
        return;
    }
    if (sym == g_config.keySpeedReset) {
        resetSpeed();
        toastInput("Speed reset", 0.7f);
        return;
    }

    if (sym == g_config.keyAudioGainDown) {
        adjustAudioGain(-0.1f);
        char buf[64];
        snprintf(buf, sizeof(buf), "Audio gain %.0f%%", g_audioGain * 100.0f);
        toastInput(buf, 0.7f);
        return;
    }
    if (sym == g_config.keyAudioGainUp) {
        adjustAudioGain(+0.1f);
        char buf[64];
        snprintf(buf, sizeof(buf), "Audio gain %.0f%%", g_audioGain * 100.0f);
        toastInput(buf, 0.7f);
        return;
    }
    if (sym == g_config.keyAudioGainReset) {
        resetAudioGain();
        toastInput("Audio gain reset", 0.7f);
        return;
    }

    if (sym == g_config.keyFullscreen || sym == SDLK_F11) {
        toggleFullscreen();
        toastInput(g_fullscreen ? "Fullscreen" : "Windowed");
        return;
    }

    if (sym == SDLK_F1) {
        if (g_appState == AppState::Visualizer && g_paused)
            g_menu.setSettingsReturnScreen(UIScreen::PauseMenu);
        else
            g_menu.setSettingsReturnScreen(UIScreen::MainMenu);
        g_menu.jumpToSettingsTab(4);
        g_menu.showScreen(UIScreen::Settings);
        if (g_appState == AppState::Visualizer && !g_paused)
            g_paused = true;
        return;
    }

    if (sym == g_config.keyBeatSensUp) {
        float sens = g_visualizer ? g_visualizer->getBeatSensitivity() : 1.0f;
        sens = (sens < 4.9f) ? sens + 0.1f : 5.0f;
        setBeatSensitivity(sens, "key");
        fprintf(stderr, "[Vibeus] Beat sensitivity: %.1f\n", sens);
        char buf[64];
        snprintf(buf, sizeof(buf), "Beat sensitivity %.1f", sens);
        toastInput(buf, 0.7f);
        return;
    }

    if (sym == g_config.keyBeatSensDown) {
        float sens = g_visualizer ? g_visualizer->getBeatSensitivity() : 1.0f;
        sens = (sens > 0.1f) ? sens - 0.1f : 0.0f;
        setBeatSensitivity(sens, "key");
        fprintf(stderr, "[Vibeus] Beat sensitivity: %.1f\n", sens);
        char buf[64];
        snprintf(buf, sizeof(buf), "Beat sensitivity %.1f", sens);
        toastInput(buf, 0.7f);
        return;
    }
}

static void processEvents()
{
    SDL_Event event;
    if (g_lastPresetHadError && g_config.storyDebugRescue) {
        fprintf(stderr, "[Vibeus] Auto-skipping broken preset...\n");
        g_presets.next(false);
        g_lastPresetHadError = false;
    }
    while (SDL_PollEvent(&event)) {
        if (g_menu.isVisible())
            g_menu.processEvent(event);

        if (event.type == SDL_QUIT) {
            g_running = false;
            continue;
        }

        if (event.type == SDL_CONTROLLERDEVICEADDED) {
            tryOpenGamepad();
            continue;
        }
        if (event.type == SDL_CONTROLLERDEVICEREMOVED) {
            if (g_gamepad) {
                SDL_GameControllerClose(g_gamepad);
                g_gamepad = nullptr;
                fprintf(stderr, "[Vibeus] Gamepad disconnected\n");
            }
            continue;
        }

        if (g_appState == AppState::Splash || g_appState == AppState::MainMenu)
            continue;

        if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
            if (g_paused) {
                resumeFromPause();
                toastInput("Resumed");
            } else {
                enterPause();
                toastInput("Paused");
            }
            continue;
        }

        if (event.type == SDL_CONTROLLERBUTTONDOWN &&
            event.cbutton.button == SDL_CONTROLLER_BUTTON_START) {
            if (g_paused) {
                resumeFromPause();
                toastInput("Resumed");
            } else {
                enterPause();
                toastInput("Paused");
            }
            continue;
        }

        if (g_paused) continue;

        if (event.type == SDL_CONTROLLERBUTTONDOWN) {
            const int btn = event.cbutton.button;

            if (btn == g_config.gpNextPreset) {
                g_presets.next(false);
                toastInput("Next preset");
                continue;
            }
            if (btn == g_config.gpPrevPreset) {
                g_presets.previous(false);
                toastInput("Previous preset");
                continue;
            }
            if (btn == g_config.gpRandomPreset) {
                g_presets.next(true);
                toastInput("Random preset");
                continue;
            }
            if (btn == g_config.gpShuffle) {
                g_presets.toggleShuffle();
                toastInput(g_presets.isShuffled() ? "Shuffle ON" : "Shuffle OFF");
                continue;
            }
            if (btn == g_config.gpAudioGainDown) {
                adjustAudioGain(-0.1f);
                char buf[64];
                snprintf(buf, sizeof(buf), "Audio gain %.0f%%", g_audioGain * 100.0f);
                toastInput(buf, 0.7f);
                continue;
            }
            if (btn == g_config.gpAudioGainUp) {
                adjustAudioGain(+0.1f);
                char buf[64];
                snprintf(buf, sizeof(buf), "Audio gain %.0f%%", g_audioGain * 100.0f);
                toastInput(buf, 0.7f);
                continue;
            }

            switch (btn) {
            case SDL_CONTROLLER_BUTTON_DPAD_UP: {
                float sens = g_visualizer ? g_visualizer->getBeatSensitivity() : 1.0f;
                sens = (sens < 4.9f) ? sens + 0.2f : 5.0f;
                setBeatSensitivity(sens, "key");
                fprintf(stderr, "[Vibeus] Beat sensitivity: %.1f\n", sens);
                char buf[64];
                snprintf(buf, sizeof(buf), "Beat sensitivity %.1f", sens);
                toastInput(buf, 0.7f);
                break;
            }
            case SDL_CONTROLLER_BUTTON_DPAD_DOWN: {
                float sens = g_visualizer ? g_visualizer->getBeatSensitivity() : 1.0f;
                sens = (sens > 0.2f) ? sens - 0.2f : 0.0f;
                setBeatSensitivity(sens, "key");
                fprintf(stderr, "[Vibeus] Beat sensitivity: %.1f\n", sens);
                char buf[64];
                snprintf(buf, sizeof(buf), "Beat sensitivity %.1f", sens);
                toastInput(buf, 0.7f);
                break;
            }
            default:
                break;
            }
            continue;
        }

        switch (event.type) {
        case SDL_KEYDOWN:
            handleKeyDown(event.key.keysym);
            break;

        case SDL_MOUSEBUTTONDOWN:
            if (false && g_config.touchEnabled && event.button.button == SDL_BUTTON_LEFT) {
                g_mouseDown = true;
                float nx, ny;
                screenToNormalized(event.button.x, event.button.y, nx, ny);
                if (auto* pmViz = dynamic_cast<ProjectMVisualizer*>(g_visualizer.get())) {
                    projectm_touch(pmViz->getHandle(), nx, ny, g_touchPressure, g_touchTypes[g_touchTypeIndex]);
                }
                spawnRipple(nx, ny);
                g_prevMouseX  = event.button.x;
                g_prevMouseY  = event.button.y;
                g_prevMouseMs = SDL_GetTicks();
            } else if (false && g_config.touchEnabled && event.button.button == SDL_BUTTON_RIGHT) {
                g_touchTypeIndex = (g_touchTypeIndex + 1) % g_touchTypeCount;
                fprintf(stderr, "[Vibeus] Touch type: %s\n", touchTypeName(g_touchTypes[g_touchTypeIndex]));
            }
            break;

        case SDL_MOUSEMOTION:
            if (false && g_config.touchEnabled && g_mouseDown) {
                float nx, ny;
                screenToNormalized(event.motion.x, event.motion.y, nx, ny);
                if (auto* pmViz = dynamic_cast<ProjectMVisualizer*>(g_visualizer.get())) {
                    projectm_touch_drag(pmViz->getHandle(), nx, ny, g_touchPressure);
                }
                processVelocityTrail(event.motion.x, event.motion.y);
            }
            break;

        case SDL_MOUSEBUTTONUP:
            if (event.button.button == SDL_BUTTON_LEFT) {
                g_mouseDown = false;
            }
            break;

        case SDL_MOUSEWHEEL:
            if (false && g_config.touchEnabled && (SDL_GetModState() & KMOD_SHIFT)) {
                if (event.wheel.y > 0)
                    g_touchTypeIndex = (g_touchTypeIndex + 1) % g_touchTypeCount;
                else if (event.wheel.y < 0)
                    g_touchTypeIndex = (g_touchTypeIndex - 1 + g_touchTypeCount) % g_touchTypeCount;
                fprintf(stderr, "[Vibeus] Touch type: %s\n", touchTypeName(g_touchTypes[g_touchTypeIndex]));
            } else if (false) {
                int prevPressure = g_touchPressure;
                g_touchPressure += (event.wheel.y > 0) ? 1 : -1;
                if (g_touchPressure < 0) g_touchPressure = 0;
                if (g_touchPressure > 2) g_touchPressure = 2;
                if (g_touchPressure != prevPressure)
                    fprintf(stderr, "[Vibeus] Touch pressure: %d\n", g_touchPressure);
            }
            break;

        case SDL_WINDOWEVENT:
            if (event.window.event == SDL_WINDOWEVENT_RESIZED ||
                event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                int w = event.window.data1;
                int h = event.window.data2;
                if (g_visualizer) g_visualizer->setWindowSize(w, h);
            }
            break;

        default:
            break;
        }
    }
}

static void setBeatSensitivity(float value, const char* reason)
{
    if (!g_visualizer) return;

    const bool stasisReason = reason && std::strcmp(reason, "stasis") == 0;
    const float minValue = stasisReason ? 0.0f : 0.1f;
    float clamped = (std::max)(minValue, (std::min)(3.0f, value));
    if (clamped != value) {
        fprintf(stderr, "[Beat] Clamped sensitivity %.2f -> %.2f (%s)\n", value, clamped, reason);
    }

    g_visualizer->setBeatSensitivity(clamped);

    static float lastLoggedValue = -1.0f;
    static std::string lastLoggedReason;
    const std::string currentReason = reason ? reason : "unknown";
    if (g_config.storyDebug &&
        (clamped != lastLoggedValue || currentReason != lastLoggedReason)) {
        fprintf(stderr, "[Beat] setBeatSensitivity(%.2f) reason=%s\n", clamped, currentReason.c_str());
        lastLoggedValue = clamped;
        lastLoggedReason = currentReason;
    }
}

static void applyConfig(const VibeusConfig& cfg)
{
    g_audioGain = cfg.audioGain;
    if (g_visualizer) {
        float effectiveSensitivity = cfg.flashLimiter
            ? (cfg.beatSensitivity < 1.5f ? cfg.beatSensitivity : 1.5f)
            : cfg.beatSensitivity;
        setBeatSensitivity(effectiveSensitivity, "applyConfig");

        g_visualizer->setPresetDuration(cfg.presetDuration);
        g_visualizer->setSoftCutDuration(cfg.transitionTime);
        g_visualizer->setHardCutEnabled(cfg.autoAdvance && cfg.hardCutEnabled);
        g_visualizer->setHardCutSensitivity(cfg.hardCutSensitivity);
        g_visualizer->setHardCutDuration(cfg.hardCutDuration);
        g_visualizer->setEasterEgg(cfg.easterEgg);

        g_visualizer->setAspectCorrection(cfg.aspectCorrection);
        
        int meshW = static_cast<int>(cfg.meshDetail);
        int meshH = meshW * 3 / 4;
        g_visualizer->setMeshSize(meshW, meshH);

        g_visualizer->setPresetLocked(!cfg.autoAdvance || cfg.vibeLock);
    }

    g_flashLimiter   = cfg.flashLimiter;
    g_reducedMotion  = cfg.reducedMotion;

    if (cfg.shuffle != g_presets.isShuffled())
        g_presets.toggleShuffle();

    if (cfg.fullscreen != g_fullscreen)
        toggleFullscreen();

    g_debug = cfg.showFps;
    if (!g_debug)
        SDL_SetWindowTitle(g_window, "Vibeus");

    g_speedMultiplier = cfg.speedMultiplier;
    g_flowMode = cfg.flowMode;

    switch (cfg.perfMode) {
        case PerfMode::BatterySaver:
            SDL_GL_SetSwapInterval(0);
            break;
        case PerfMode::Balanced:
        case PerfMode::Quality:
        default:
            SDL_GL_SetSwapInterval(1);
            break;
    }

    ImGui::GetIO().FontGlobalScale = 1.35f * cfg.fontScale;
}

static std::string normalizePresetPathForCompare(const std::string& path)
{
    if (path.empty()) return {};

    try {
        fs::path p(path);
        if (fs::exists(p))
            return fs::weakly_canonical(p).string();
        return p.lexically_normal().string();
    } catch (...) {
        return path;
    }
}

static bool findPlaylistPositionByPath(projectm_playlist_handle playlist,
                                       const std::string& selectedPath,
                                       uint32_t& outPosition)
{
    if (!playlist || selectedPath.empty()) return false;

    const std::string target = normalizePresetPathForCompare(selectedPath);
    const uint32_t total = g_presets.count();

    for (uint32_t i = 0; i < total; ++i) {
        char* item = projectm_playlist_item(playlist, i);
        if (!item) continue;

        std::string itemPath(item);
        projectm_playlist_free_string(item);

        if (itemPath == selectedPath || normalizePresetPathForCompare(itemPath) == target) {
            outPosition = i;
            return true;
        }
    }

    return false;
}

static void handleMenuAction(MenuAction action)
{
    switch (action) {
    case MenuAction::BackToMenu:
        g_appState = AppState::MainMenu;
        g_menu.showScreen(UIScreen::MainMenu);
        break;

    case MenuAction::StartVisualizer:
        g_appState = AppState::Visualizer;
        g_menu.hideAll();
        g_paused = false;
        g_lastFrameTicks = SDL_GetTicks();
        fprintf(stderr, "[Vibeus] Starting visualizer\n");
        break;

    case MenuAction::Resume:
        resumeFromPause();
        g_menu.hideAll();
        break;

    case MenuAction::BrowsePresets:
        g_menu.loadPresetList(g_presets.handle());
        g_menu.setPresetDatabase(&g_presetDb);

        {
            g_menu.loadFavorites(g_favoritesPath);

            for (const auto& path : g_config.favoritePresetPaths) {
                for (uint32_t i = 0; i < 20000; ++i) {
                    const ::PresetEntry* pe = g_presetDb.getPreset(i);
                    if (!pe) break;
                    if (pe->fullPath == path) {
                        g_menu.toggleFavorite(i);
                        break;
                    }
                }
            }
        }
        g_menu.showScreen(UIScreen::PresetBrowser);
        break;

    case MenuAction::PlayPreset: {
        const std::string selectedPath = g_menu.selectedPresetPath();
        uint32_t playlistPos = 0;

        if (!findPlaylistPositionByPath(g_presets.handle(), selectedPath, playlistPos)) {
            fprintf(stderr,
                    "[PresetBrowser] WARNING: Could not reconcile selected preset path to live playlist: %s\n",
                    selectedPath.empty() ? "<empty>" : selectedPath.c_str());
            g_menu.showToast("Preset unavailable (filtered or quarantined)", 3.0f);
            break;
        }

        // Use abstraction for preset change
        if (g_visualizer) {
            g_visualizer->setPreset(playlistPos, true);
        }
        fprintf(stderr, "[Vibeus] Playing preset #%u via path: %s\n", playlistPos, selectedPath.c_str());
        if (g_appState == AppState::MainMenu) {
            g_appState = AppState::Visualizer;
            g_menu.hideAll();
            g_paused = false;
            g_lastFrameTicks = SDL_GetTicks();
        }
        else if (g_paused) {
            resumeFromPause();
            g_menu.hideAll();
        }
        break;
    }

    case MenuAction::BackToPause:
        {
            g_menu.saveFavorites(g_favoritesPath);
        }
        g_menu.showScreen(UIScreen::PauseMenu);
        break;

    case MenuAction::ShowControls:
        if (g_appState == AppState::Visualizer && g_paused)
            g_menu.setSettingsReturnScreen(UIScreen::PauseMenu);
        else
            g_menu.setSettingsReturnScreen(UIScreen::MainMenu);
        g_menu.jumpToSettingsTab(4);
        g_menu.showScreen(UIScreen::Settings);
        break;

    case MenuAction::Settings:
        if (g_appState == AppState::Visualizer && g_paused)
            g_menu.setSettingsReturnScreen(UIScreen::PauseMenu);
        else
            g_menu.setSettingsReturnScreen(UIScreen::MainMenu);
        g_menu.showScreen(UIScreen::Settings);
        break;

    case MenuAction::Record:
        fprintf(stderr, "[Vibeus] Recording not yet implemented\n");
        break;

    case MenuAction::ExitToDesktop:
        {
            g_menu.saveFavorites(g_favoritesPath);
        }
        saveConfig(g_config, g_configPath);
        g_running = false;
        break;

    case MenuAction::ApplySettings:
        applyConfig(g_config);
        g_menu.showToast("Settings applied");
        break;

    case MenuAction::ApplyTransitionSettings:
        applyConfig(g_config);
        {
            StorySettings ss{};
            ss.transitionStyle     = g_config.storyTransitionStyle;
            ss.dropSensitivity     = g_config.storyDropSensitivity;
            ss.bassReactivity      = g_config.storyBassReactivity;
            ss.sustainMaxSec       = g_config.storySustainMax;
            ss.buildupLockPreset   = g_config.storyBuildupLock;
            ss.chillGateSeconds    = g_config.storyChillGateSec;
            ss.buildupMinSeconds   = g_config.storyBuildupMinSec;
            ss.buildupRatioGate    = g_config.storyBuildupRatio;
            ss.buildupFluxGate     = g_config.storyBuildupFlux;
            ss.dropRatioGate       = g_config.storyDropRatio;
            ss.dropBassGate        = g_config.storyDropBass;
            ss.dropFluxGate        = g_config.storyDropFlux;
            g_storyteller.applySettings(ss);
        }
        g_menu.showToast("Transition settings applied");
        break;

    case MenuAction::BackFromSettings:
        applyConfig(g_config);
        saveConfig(g_config, g_configPath);
        g_menu.showToast("Settings saved");
        {
            UIScreen returnTo = g_menu.settingsReturnScreen();
            g_menu.showScreen(returnTo);
            if (returnTo == UIScreen::PauseMenu)
                g_pauseRenderCount = 0;
        }
        break;

    default:
        break;
    }

    if (action == MenuAction::BackToMenu &&
        g_menu.currentScreen() == UIScreen::MainMenu) {
        g_menu.saveFavorites(g_favoritesPath);
    }
}

int main(int argc, char* argv[])
{
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--debug") == 0 || strcmp(argv[i], "-d") == 0)
            g_debug = true;
    }

#ifdef _WIN32
    if (g_debug)
        attachConsole();
#endif

    g_userDataDir = getUserDataDir();
    g_configPath = (fs::path(g_userDataDir) / "vibeus_config.json").string();
    g_favoritesPath = (fs::path(g_userDataDir) / "favorites.txt").string();
    g_brokenPresetBlacklistPath = (fs::path(g_userDataDir) / "broken_presets.txt").string();
    g_userBlacklistPath = (fs::path(g_userDataDir) / "blacklisted_presets.txt").string();

    if (!g_debug) {
        std::string logPath = (fs::path(g_userDataDir) / "vibeus_debug.log").string();
        FILE* dummy;
        freopen_s(&dummy, logPath.c_str(), "a", stderr);
        freopen_s(&dummy, logPath.c_str(), "a", stdout);
        setvbuf(stderr, nullptr, _IONBF, 0);
        setvbuf(stdout, nullptr, _IONBF, 0);
    }

    fprintf(stderr, "=== Vibeus v0.2.3-dev (ABSTRACTION MIGRATION) ===\n");
    if (g_debug)
        fprintf(stderr, "[DEBUG MODE ENABLED]\n");
    fprintf(stderr, "[Vibeus] User data dir: %s\n\n", g_userDataDir.c_str());

    g_config = loadConfig(g_configPath);
    if (!fs::exists(g_configPath))
        saveConfig(g_config, g_configPath);

    if (!initSDL()) return 1;
    fprintf(stderr, "[Vibeus] SDL2 + OpenGL 3.3 context ready\n");

    if (g_debug) {
        if (auto* pmViz = dynamic_cast<ProjectMVisualizer*>(g_visualizer.get())) {
            // Will be set after init
        }
    }

    if (!initVisualizer()) {
        SDL_GL_DeleteContext(g_glCtx);
        SDL_DestroyWindow(g_window);
        SDL_Quit();
        return 1;
    }

    if (!g_audio.init()) {
        fprintf(stderr, "[Vibeus] WARNING: Audio capture failed - visuals won't react to music\n");
    }

    std::string presetsDir = findPresetsDir();
    fprintf(stderr, "[Vibeus] Presets directory: %s\n", presetsDir.c_str());
    g_presets.init(g_visualizer ? g_visualizer->getHandle() : nullptr, presetsDir);

    g_presets.applyBlacklist(g_brokenPresetBlacklistPath);
    g_presets.applyBlacklist(g_userBlacklistPath);

    fprintf(stderr, "\n[Vibeus] Ready! %u presets loaded. Shuffle: %s\n",
            g_presets.count(), g_presets.isShuffled() ? "ON" : "OFF");

    g_storyteller.init(g_visualizer ? g_visualizer->getHandle() : nullptr, &g_presets);

    if (g_config.validatePresetsOnStartup) {
        fprintf(stderr, "\n[Vibeus] Validating presets for shader errors...\n");
        fprintf(stderr, "[Vibeus] This may take a few minutes on first run.\n");
        fprintf(stderr, "[Vibeus] Press ESC to skip validation.\n");
        try {
            const std::string validationLogPath = (fs::path(g_userDataDir) / "preset_validation.log").string();
            bool validationSkipped = false;
            uint32_t removed = g_presets.validateAndFilter(g_userDataDir,
                                                           g_brokenPresetBlacklistPath,
                                                           validationLogPath,
                                                           &validationSkipped);
            if (validationSkipped) {
                fprintf(stderr, "[Vibeus] Preset validation skipped by user.\n");
                fprintf(stderr, "[Vibeus] Validation log: %s\n", validationLogPath.c_str());
            } else if (removed > 0) {
                fprintf(stderr, "\n[Vibeus] Removed %u broken presets (blacklist: %s)\n", removed, g_brokenPresetBlacklistPath.c_str());
                fprintf(stderr, "[Vibeus] Final count: %u working presets\n", g_presets.count());
                fprintf(stderr, "[Vibeus] Validation log: %s\n", validationLogPath.c_str());
            } else {
                fprintf(stderr, "[Vibeus] All presets validated successfully!\n");
                fprintf(stderr, "[Vibeus] Validation log: %s\n", validationLogPath.c_str());
            }
        } catch (const std::exception& e) {
            fprintf(stderr, "[Vibeus] WARNING: Preset validation failed: %s\n", e.what());
        }
    }

    fprintf(stderr, "\n[Vibeus] Controls:\n");
    fprintf(stderr, "  N/Right=next  P/Left=prev  R=random  H=history  S=shuffle\n");
    fprintf(stderr, "  F/F11=fullscreen  D=debug  Up/Down=beat sensitivity\n");
    fprintf(stderr, "  [/]=speed down/up  Backspace=reset speed\n");
    fprintf(stderr, "  -/==audio gain down/up  0=reset audio\n");
    fprintf(stderr, "  Click=touch waveform  Drag=move  RightClick=cycle type\n");
    fprintf(stderr, "  Scroll=pressure  Shift+Scroll=cycle type  C=clear waveforms\n");
    fprintf(stderr, "  Tab=flow mode  Esc=pause menu  Q=quit\n");
    fprintf(stderr, "  Gamepad: A=next B=prev X=random Y=shuffle Start=pause\n\n");

    if (!g_menu.init(g_window, g_glCtx)) {
        fprintf(stderr, "[Vibeus] WARNING: Menu overlay init failed\n");
    }
    g_menu.setConfigPtr(&g_config);
    g_menu.setUserDataDir(g_userDataDir);

    {
        bool savedFullscreen = g_config.fullscreen;
        g_config.fullscreen = g_fullscreen;
        applyConfig(g_config);
        g_config.fullscreen = savedFullscreen;
        if (savedFullscreen && !g_fullscreen)
            toggleFullscreen();
    }

    tryOpenGamepad();

    g_appState = AppState::Splash;
    g_menu.showScreen(UIScreen::Splash);

    Uint32 frameDelay = 1000 / TARGET_FPS;
    g_fpsTimer = SDL_GetTicks();
    g_lastFrameTicks = g_fpsTimer;

    while (g_running) {
        Uint32 frameStart = SDL_GetTicks();

        processEvents();

        static auto pfnBindFramebuffer = reinterpret_cast<void(APIENTRY*)(GLenum, GLuint)>(
            SDL_GL_GetProcAddress("glBindFramebuffer"));
        if (pfnBindFramebuffer)
            pfnBindFramebuffer(0x8D40 /*GL_FRAMEBUFFER*/, 0);
        glDisable(GL_SCISSOR_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        switch (g_appState) {
        case AppState::Splash:
        case AppState::MainMenu:
            if (g_appState == AppState::MainMenu || g_appState == AppState::Splash) {
                static double menuBgTime = 0.0;
                menuBgTime += 0.016 * 0.3;
                if (g_visualizer) g_visualizer->setFrameTime(menuBgTime);
                g_audio.feedAudio(g_visualizer.get(), 0.3f);
                if (g_visualizer) g_visualizer->renderFrame();
            }
            break;

        case AppState::Visualizer: {
            bool settingsLive = g_paused &&
                                g_menu.currentScreen() == UIScreen::Settings;
            if (!g_paused || settingsLive) {
                processGamepad();

                static float silenceTimer = 0.0f;
                const float rms = g_audio.levelRms();
                const float deltaTime = 1.0f / (g_currentFps > 10.0f ? g_currentFps : 60.0f);

                if (g_config.stasisEnabled) {
                    if (rms < g_config.stasisThreshold) {
                        silenceTimer += deltaTime;
                        if (silenceTimer >= g_config.stasisFadeTime && !g_inStasis) {
                            g_inStasis = true;
                            fprintf(stderr,
                                    "[Stasis] Entered silence freeze (rms=%.4f threshold=%.4f delay=%.2fs)\n",
                                    rms, g_config.stasisThreshold, g_config.stasisFadeTime);
                        }
                    } else {
                        if (g_inStasis) {
                            fprintf(stderr,
                                    "[Stasis] Exited silence freeze (rms=%.4f threshold=%.4f)\n",
                                    rms, g_config.stasisThreshold);
                            silenceTimer = 0.0f;
                            g_inStasis = false;
                        }
                        silenceTimer = 0.0f;
                    }
                } else {
                    if (g_inStasis) {
                        fprintf(stderr, "[Stasis] Disabled while active — exiting silence freeze\n");
                    }
                    silenceTimer = 0.0f;
                    g_inStasis = false;
                }

                if (g_inStasis) {
                    g_lastFrameTicks = SDL_GetTicks();
                    if (g_visualizer) g_visualizer->setFrameTime(g_virtualTime);
                } else {
                    updateVirtualTime(SDL_GetTicks());
                }

                if (g_config.storytellingEnabled && !g_inStasis) {
                    StorySettings ss;
                    ss.dropSensitivity   = g_config.storyDropSensitivity;
                    ss.bassReactivity    = g_config.storyBassReactivity;
                    ss.sustainMaxSec     = g_config.storySustainMax;
                    ss.transitionStyle   = g_config.storyTransitionStyle;
                    ss.buildupLockPreset = g_config.storyBuildupLock;
                    ss.debugMode         = g_config.storyDebug;
                    ss.chillGateSeconds  = g_config.storyChillGateSec;
                    ss.buildupMinSeconds = g_config.storyBuildupMinSec;
                    ss.buildupRatioGate  = g_config.storyBuildupRatio;
                    ss.buildupFluxGate   = g_config.storyBuildupFlux;
                    ss.dropRatioGate     = g_config.storyDropRatio;
                    ss.dropBassGate      = g_config.storyDropBass;
                    ss.dropFluxGate      = g_config.storyDropFlux;
                    ss.beatReactivity    = g_config.beatReactivity;
                    g_storyteller.applySettings(ss);

                    g_storyteller.update(g_audio.levelPeak(),
                                         g_audio.levelBass(),
                                         g_audio.levelBeatBass(),
                                         g_audio.levelBeatMid(),
                                         g_audio.levelBeatHigh(),
                                         deltaTime);
                } else if (g_inStasis) {
                    if (g_visualizer) g_visualizer->setBeatSensitivity(0.0f);
                }

                float currentSens = g_visualizer ? g_visualizer->getBeatSensitivity() : 1.0f;
                if (currentSens > 3.0f) {
                    setBeatSensitivity(2.0f, "safety-reset");
                    fprintf(stderr, "[Safety] Runaway sensitivity detected (%.1f) — reset to 2.0\n", currentSens);
                }

                float effectiveGain = (g_flashLimiter && g_audioGain > 1.0f) ? 1.0f : g_audioGain;
                g_audio.feedAudio(g_visualizer.get(), effectiveGain);

                if (!g_inStasis && !g_config.adaptiveBeat) {
                    float fixedSens = g_config.beatSensitivity;
                    setBeatSensitivity(fixedSens, "non-adaptive");
                }
                if (g_visualizer) g_visualizer->renderFrame();

                const float bassEnergy = g_audio.levelBeatBass();
                const float rmsEnergy  = g_audio.levelRms();

                static float bassAvg = 0.0f;
                static float fluxAvg = 0.0f;
                static float prevBass = 0.0f;
                static int   beatCooldown = 0;

                if (beatCooldown > 0)
                    beatCooldown--;

                static int lastHoldFrames = 0;
                int targetHoldFrames = static_cast<int>(g_config.beatHoldTime * (g_currentFps > 10.0f ? g_currentFps : 60.0f));
                if (targetHoldFrames != lastHoldFrames) {
                    lastHoldFrames = targetHoldFrames;
                }

                float flux = (std::max)(0.0f, bassEnergy - prevBass);
                prevBass = bassEnergy;

                bassAvg = bassAvg * 0.96f + bassEnergy * 0.04f;
                fluxAvg = fluxAvg * 0.90f + flux * 0.10f;

                const float dynamicFloor     = (std::max)(0.01f, bassAvg * 0.8f + rmsEnergy * 0.05f);
                const float fluxGate         = 0.0015f + fluxAvg * 0.20f;
                const float strongFluxGate   = fluxAvg * 1.6f + 0.010f;
                const float dynamicThreshold = dynamicFloor * 1.15f + fluxAvg * 0.65f + 0.010f;

                const bool fluxHit     = (flux >= fluxGate);
                const bool fluxStrong  = (flux >= strongFluxGate);
                const bool bassHit     = (bassEnergy >= dynamicThreshold);
                const bool bassJump    = (bassEnergy >= dynamicFloor * 2.0f);
                const bool fluxOnlyHit = fluxStrong && (bassEnergy >= dynamicFloor * 0.8f);

                if (beatCooldown == 0 && ( (bassHit && fluxHit) || bassJump || fluxOnlyHit )) {
                    g_beatIndicatorAlpha = 1.0f;

                    float holdSec = (g_config.beatHoldTime > 0.01f) ? g_config.beatHoldTime : 0.18f;
                    beatCooldown = static_cast<int>((g_currentFps > 10.0f ? g_currentFps : 60.0f) * holdSec);
                    if (beatCooldown < 6) beatCooldown = 6;
                }

                if (g_beatIndicatorAlpha > 0.0f) {
                    float deltaTime = 1.0f / (g_currentFps > 0.0f ? g_currentFps : 60.0f);
                    g_beatIndicatorAlpha -= deltaTime * 4.0f;
                    if (g_beatIndicatorAlpha < 0.0f) g_beatIndicatorAlpha = 0.0f;
                }
            } else {
                if (g_pauseRenderCount < 3) {
                    if (g_visualizer) {
                        g_visualizer->setFrameTime(g_pausedTime);
                        g_visualizer->renderFrame();
                    }
                    g_pauseRenderCount++;
                }
            }
            break;
        }
        }

        if (g_menu.isVisible()) {
            const char* stateStr = (g_storyteller.currentState() == StoryState::Chill)   ? "CHILL" :
                                   (g_storyteller.currentState() == StoryState::Buildup) ? "BUILD" :
                                   (g_storyteller.currentState() == StoryState::Drop)    ? "DROP"  : "SUSTAIN";
            std::string presetName = g_presets.currentPresetName();
            g_menu.setLiveStatus(presetName, stateStr, g_storyteller.lastBeatSensitivity(), g_inStasis);

            MenuAction action = g_menu.render();
            if (action != MenuAction::None)
                handleMenuAction(action);
        }

        if (g_appState == AppState::Visualizer && g_showBeatIndicator && g_beatIndicatorAlpha > 0.0f) {
            g_menu.renderBeatIndicator(g_beatIndicatorAlpha);
        }

        if (g_appState == AppState::Visualizer && g_config.storytellingEnabled && g_config.storyDebug) {
            const char* s = (g_storyteller.currentState() == StoryState::Chill)   ? "CHILL" :
                            (g_storyteller.currentState() == StoryState::Buildup) ? "BUILD" :
                            (g_storyteller.currentState() == StoryState::Drop)    ? "DROP"  : "SUSTAIN";
            g_menu.renderStoryOverlay(s, g_storyteller.lastEnergyRatio(), g_storyteller.spectralFlux());
        }

        g_menu.renderToasts();

        SDL_GL_SwapWindow(g_window);

        g_frameCount++;
        Uint32 now = SDL_GetTicks();
        if (now - g_fpsTimer >= 1000) {
            g_currentFps = g_frameCount * 1000.0f / (now - g_fpsTimer);
            g_frameCount = 0;
            g_fpsTimer = now;
            if (g_appState == AppState::Visualizer)
                updateDebugTitle();
        }

        Uint32 elapsed = SDL_GetTicks() - frameStart;
        if (elapsed < frameDelay) {
            SDL_Delay(frameDelay - elapsed);
        }
    }

    fprintf(stderr, "\n[Vibeus] Shutting down...\n");

    g_menu.saveFavorites(g_favoritesPath);
    saveConfig(g_config, g_configPath);

    g_menu.shutdown();
    g_audio.shutdown();
    if (g_gamepad) {
        SDL_GameControllerClose(g_gamepad);
        g_gamepad = nullptr;
    }
    if (g_visualizer) {
        g_visualizer->shutdown();
        g_visualizer.reset();
    }

    SDL_GL_DeleteContext(g_glCtx);
    SDL_DestroyWindow(g_window);
    SDL_Quit();

    return 0;
}
