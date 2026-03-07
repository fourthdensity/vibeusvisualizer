/**
 * Vibeus - A custom music visualizer frontend powered by projectM
 *
 * Controls:
 *   N / Right       - Next preset
 *   P / Left        - Previous preset
 *   R               - Random preset (hard cut)
 *   H               - Go back in history
 *   S               - Toggle shuffle
 *   F / F11         - Toggle fullscreen
 *   Up/Down         - Adjust beat sensitivity
 *   D               - Toggle debug overlay
 *   [ / ]           - Slow down / speed up visualization
 *   Backspace       - Reset speed to 1.0x
 *   - / =           - Audio reactivity down / up
 *   0               - Reset audio reactivity to 1.0x
 *   Mouse click     - Create touch waveform
 *   Mouse drag      - Move touch waveform
 *   Right-click     - Cycle waveform type
 *   Scroll wheel    - Adjust touch pressure
 *   Shift+Scroll    - Cycle waveform type
 *   C               - Clear all touch waveforms
 *   Tab             - Toggle flow mode (mouse controls speed + waveforms)
 *   Escape          - Pause menu (during viz) / Back (in menus)
 *   Q               - Quit
 *
 * Gamepad (if connected):
 *   Left stick      - Speed control (X) + waveform position (Y)
 *   Right stick     - Touch waveform creation & movement
 *   A / Cross       - Next preset
 *   B / Circle      - Previous preset
 *   X / Square      - Random preset
 *   Y / Triangle    - Toggle shuffle
 *   Start           - Pause menu
 *   LB / RB         - Audio gain down / up
 *   LT / RT         - Speed down / up (analog)
 *   D-Pad Up/Down   - Beat sensitivity
 */

#ifdef _WIN32
#include <windows.h>
#endif

#include <SDL.h>
#include <SDL_opengl.h>

#include <projectM-4/projectM.h>
#include <projectM-4/playlist.h>

#include "audio_capture.h"
#include "preset_manager.h"
#include "menu_overlay.h"
#include "config.h"

#include <imgui.h>

#include <cstdio>
#include <cstdlib>
#include <string>
#include <cstring>
#include <filesystem>

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
static projectm_handle g_pm     = nullptr;
static bool          g_running  = true;
static bool          g_fullscreen = false;
static bool          g_debug    = false;

static AudioCapture  g_audio;
static PresetManager g_presets;
static MenuOverlay   g_menu;

// Configuration (persisted to JSON)
static VibeusConfig  g_config;
static std::string   g_configPath;

// App state machine: Splash → MainMenu → Visualizer (with pause overlay)
enum class AppState { Splash, MainMenu, Visualizer };
static AppState g_appState = AppState::Splash;

// Pause state
static bool          g_paused    = false;
static double        g_pausedTime = 0.0;
static int           g_pauseRenderCount = 0;

// Debug stats
static Uint32 g_frameCount  = 0;
static Uint32 g_fpsTimer    = 0;
static float  g_currentFps  = 0.0f;

// Speed control - always uses virtual clock for consistent pause/resume
static double g_speedMultiplier = 1.0;    // 0.05x to 4.0x
static double g_virtualTime     = 0.0;    // accumulated virtual seconds
static Uint32 g_lastFrameTicks  = 0;

// Audio dampening - scales PCM amplitude before feeding to projectM
static float  g_audioGain       = 1.0f;   // 0.0 (muted) to 3.0 (boosted)

// Mouse / touch waveform state
static bool   g_mouseDown       = false;
static int    g_touchPressure   = 1;      // 0 to 2 (low / medium / high)
static int    g_touchTypeIndex  = 0;      // index into touch type list
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

// Gamepad
static SDL_GameController* g_gamepad = nullptr;
static const int STICK_DEADZONE = 8000;  // out of 32768

// Flow mode: mouse position controls speed + creates waveforms
static bool  g_flowMode = false;

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

// ----- Debug -----

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

static void projectmLogCallback(const char* message, projectm_log_level level, void* /*userData*/)
{
    fprintf(stderr, "[projectM/%s] %s\n", logLevelStr(level), message);
}

static void updateDebugTitle()
{
    if (!g_debug) return;
    std::string preset = g_presets.currentPresetName();
    // Extract just the filename
    auto pos = preset.find_last_of("/\\");
    if (pos != std::string::npos)
        preset = preset.substr(pos + 1);
    char title[512];
    snprintf(title, sizeof(title),
             "Vibeus | %.0f FPS | %.2fx speed | %.0f%% audio | Touch: %s P%d | %s%s[%u/%u] %s",
             g_currentFps, g_speedMultiplier, g_audioGain * 100.0f,
             touchTypeName(g_touchTypes[g_touchTypeIndex]), g_touchPressure,
             g_flowMode ? "FLOW | " : "",
             g_gamepad ? "GAMEPAD | " : "",
             g_presets.position() + 1, g_presets.count(), preset.c_str());
    SDL_SetWindowTitle(g_window, title);
}

// ----- Speed Control -----

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
    // Cap delta to prevent jumps (e.g. after resume or stall)
    if (realDelta > 0.1) realDelta = 0.1;
    g_virtualTime += realDelta * g_speedMultiplier;
    projectm_set_frame_time(g_pm, g_virtualTime);
}

// ----- Pause -----

static void enterPause()
{
    g_paused = true;
    g_pausedTime = g_virtualTime;
    g_pauseRenderCount = 0; // reset so we render a few frozen frames
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

// ----- Audio Gain -----

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

// ----- Mouse → Touch -----

static void screenToNormalized(int sx, int sy, float& nx, float& ny)
{
    int w, h;
    SDL_GetWindowSize(g_window, &w, &h);
    nx = static_cast<float>(sx) / static_cast<float>(w);
    ny = static_cast<float>(sy) / static_cast<float>(h);
}

// ----- Gamepad -----

static void tryOpenGamepad()
{
    if (g_gamepad) return; // already have one
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
    if (abs(raw) < STICK_DEADZONE) return 0.0f;
    return static_cast<float>(raw) / 32768.0f;
}

static void processGamepad()
{
    if (!g_gamepad) return;
    if (g_paused || g_appState != AppState::Visualizer) return;

    // Left trigger = slow down, Right trigger = speed up (analog)
    float lt = static_cast<float>(SDL_GameControllerGetAxis(g_gamepad, SDL_CONTROLLER_AXIS_TRIGGERLEFT)) / 32768.0f;
    float rt = static_cast<float>(SDL_GameControllerGetAxis(g_gamepad, SDL_CONTROLLER_AXIS_TRIGGERRIGHT)) / 32768.0f;
    if (lt > 0.1f || rt > 0.1f) {
        // Map triggers: LT slows (toward 0.1x), RT speeds (toward 4x)
        double targetSpeed = 1.0 + (rt - lt) * 1.5;
        // Smoothly interpolate toward target
        g_speedMultiplier += (targetSpeed - g_speedMultiplier) * 0.08;
        if (g_speedMultiplier < 0.05) g_speedMultiplier = 0.05;
        if (g_speedMultiplier > 4.0)  g_speedMultiplier = 4.0;
    }

    // Right stick = touch waveform control
    float rx = stickAxis(SDL_CONTROLLER_AXIS_RIGHTX);
    float ry = stickAxis(SDL_CONTROLLER_AXIS_RIGHTY);
    if (fabsf(rx) > 0.0f || fabsf(ry) > 0.0f) {
        float nx = 0.5f + rx * 0.5f;  // map -1..1 to 0..1
        float ny = 0.5f + ry * 0.5f;
        float magnitude = sqrtf(rx * rx + ry * ry);
        int pressure = (magnitude > 0.7f) ? 2 : (magnitude > 0.3f) ? 1 : 0;
        projectm_touch(g_pm, nx, ny, pressure, g_touchTypes[g_touchTypeIndex]);
    }

    // Left stick X = fine speed adjustment
    float lx = stickAxis(SDL_CONTROLLER_AXIS_LEFTX);
    if (fabsf(lx) > 0.0f) {
        g_speedMultiplier += lx * 0.02;
        if (g_speedMultiplier < 0.05) g_speedMultiplier = 0.05;
        if (g_speedMultiplier > 4.0)  g_speedMultiplier = 4.0;
    }
}

// ----- Flow Mode -----

static void processFlowMode()
{
    if (!g_flowMode || g_paused || g_appState != AppState::Visualizer) return;

    int mx, my;
    SDL_GetMouseState(&mx, &my);
    int w, h;
    SDL_GetWindowSize(g_window, &w, &h);

    // Normalized mouse position: 0..1
    float nx = static_cast<float>(mx) / static_cast<float>(w);
    float ny = static_cast<float>(my) / static_cast<float>(h);

    // Horizontal position maps to speed: left=0.2x, center=1.0x, right=3.0x
    double targetSpeed = 0.2 + nx * 2.8;
    g_speedMultiplier += (targetSpeed - g_speedMultiplier) * 0.05;
    if (g_speedMultiplier < 0.05) g_speedMultiplier = 0.05;
    if (g_speedMultiplier > 4.0)  g_speedMultiplier = 4.0;

    // Vertical position creates subtle waveforms at mouse location
    // Lower = more waveforms (every N frames)
    int interval = static_cast<int>(15.0f - ny * 12.0f); // top=15 frames, bottom=3 frames
    if (interval < 2) interval = 2;
    static int flowFrameCount = 0;
    flowFrameCount++;
    if (flowFrameCount >= interval) {
        flowFrameCount = 0;
        int pressure = static_cast<int>(ny * 2.0f); // top=0, bottom=2
        if (pressure > 2) pressure = 2;
        projectm_touch(g_pm, nx, ny, pressure, g_touchTypes[g_touchTypeIndex]);
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

// ----- Helpers -----

static std::string findPresetsDir()
{
    // Check common locations relative to the executable
    auto exePath = fs::path(SDL_GetBasePath());
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
    return "presets"; // fallback
}

static bool initSDL()
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0) {
        fprintf(stderr, "[Vibeus] SDL_Init failed: %s\n", SDL_GetError());
        return false;
    }

    // Request OpenGL 3.3 Core
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
    SDL_GL_SetSwapInterval(1); // vsync

    return true;
}

static bool initProjectM()
{
    g_pm = projectm_create();
    if (!g_pm) {
        fprintf(stderr, "[Vibeus] projectm_create() failed - check OpenGL context\n");
        return false;
    }

    // Print version
    int major, minor, patch;
    projectm_get_version_components(&major, &minor, &patch);
    fprintf(stderr, "[Vibeus] projectM %d.%d.%d initialized\n", major, minor, patch);

    // Configure
    projectm_set_window_size(g_pm, DEFAULT_WIDTH, DEFAULT_HEIGHT);
    projectm_set_preset_duration(g_pm, PRESET_DURATION);
    projectm_set_soft_cut_duration(g_pm, SOFT_CUT_DURATION);
    projectm_set_hard_cut_duration(g_pm, HARD_CUT_DURATION);
    projectm_set_hard_cut_enabled(g_pm, true);
    projectm_set_beat_sensitivity(g_pm, BEAT_SENSITIVITY);

    // Set texture search paths (for preset textures)
    std::string presetsDir = findPresetsDir();
    const char* texPaths[] = { presetsDir.c_str() };
    projectm_set_texture_search_paths(g_pm, texPaths, 1);

    return true;
}

static void toggleFullscreen()
{
    g_fullscreen = !g_fullscreen;
    SDL_SetWindowFullscreen(g_window,
        g_fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);

    int w, h;
    SDL_GetWindowSize(g_window, &w, &h);
    projectm_set_window_size(g_pm, w, h);
    fprintf(stderr, "[Vibeus] %s (%dx%d)\n",
            g_fullscreen ? "Fullscreen" : "Windowed", w, h);
}

static void handleKeyDown(SDL_Keysym key)
{
    if (g_debug)
        fprintf(stderr, "[Vibeus] Key: %s (0x%x)\n", SDL_GetKeyName(key.sym), key.sym);

    switch (key.sym) {
    case SDLK_q:
        g_running = false;
        break;

    case SDLK_n:
    case SDLK_RIGHT:
        g_presets.next(false);
        break;

    case SDLK_p:
    case SDLK_LEFT:
        g_presets.previous(false);
        break;

    case SDLK_r:
        g_presets.next(true); // hard cut = random jump
        break;

    case SDLK_h:
        g_presets.last(false);
        break;

    case SDLK_s:
        g_presets.toggleShuffle();
        break;

    case SDLK_d:
        g_debug = !g_debug;
        if (!g_debug)
            SDL_SetWindowTitle(g_window, "Vibeus");
        fprintf(stderr, "[Vibeus] Debug display: %s\n", g_debug ? "ON" : "OFF");
        break;

    // Speed control
    case SDLK_LEFTBRACKET:  adjustSpeed(-0.05); break;
    case SDLK_RIGHTBRACKET: adjustSpeed(+0.05); break;
    case SDLK_BACKSPACE:    resetSpeed(); break;

    // Audio gain
    case SDLK_MINUS:  adjustAudioGain(-0.1f); break;
    case SDLK_EQUALS: adjustAudioGain(+0.1f); break;
    case SDLK_0:      resetAudioGain(); break;

    // Touch waveforms
    case SDLK_c:
        projectm_touch_destroy_all(g_pm);
        fprintf(stderr, "[Vibeus] Cleared all touch waveforms\n");
        break;

    // Flow mode toggle
    case SDLK_TAB:
        g_flowMode = !g_flowMode;
        fprintf(stderr, "[Vibeus] Flow mode: %s\n", g_flowMode ? "ON" : "OFF");
        if (!g_flowMode) {
            // Reset speed when exiting flow mode
            g_speedMultiplier = 1.0;
        }
        break;

    case SDLK_f:
    case SDLK_F11:
        toggleFullscreen();
        break;

    case SDLK_UP: {
        float sens = projectm_get_beat_sensitivity(g_pm);
        sens = (sens < 4.9f) ? sens + 0.1f : 5.0f;
        projectm_set_beat_sensitivity(g_pm, sens);
        fprintf(stderr, "[Vibeus] Beat sensitivity: %.1f\n", sens);
        break;
    }

    case SDLK_DOWN: {
        float sens = projectm_get_beat_sensitivity(g_pm);
        sens = (sens > 0.1f) ? sens - 0.1f : 0.0f;
        projectm_set_beat_sensitivity(g_pm, sens);
        fprintf(stderr, "[Vibeus] Beat sensitivity: %.1f\n", sens);
        break;
    }

    default:
        break;
    }
}

static void processEvents()
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        // Only forward events to ImGui when UI is visible
        if (g_menu.isVisible())
            g_menu.processEvent(event);

        // Always handle window close
        if (event.type == SDL_QUIT) {
            g_running = false;
            continue;
        }

        // Gamepad hotplug
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

        // --- State-specific input handling ---

        // During splash/main menu, all input goes to the UI
        if (g_appState == AppState::Splash || g_appState == AppState::MainMenu)
            continue;

        // In visualizer state:

        // ESC toggles pause menu
        if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
            if (g_paused) resumeFromPause();
            else enterPause();
            continue;
        }

        // Gamepad Start = pause menu
        if (event.type == SDL_CONTROLLERBUTTONDOWN &&
            event.cbutton.button == SDL_CONTROLLER_BUTTON_START) {
            if (g_paused) resumeFromPause();
            else enterPause();
            continue;
        }

        // When paused/in browser, suppress normal visualizer input
        if (g_paused) continue;

        // Gamepad buttons (during active visualization)
        if (event.type == SDL_CONTROLLERBUTTONDOWN) {
            switch (event.cbutton.button) {
            case SDL_CONTROLLER_BUTTON_A:
                g_presets.next(false);
                break;
            case SDL_CONTROLLER_BUTTON_B:
                g_presets.previous(false);
                break;
            case SDL_CONTROLLER_BUTTON_X:
                g_presets.next(true); // hard cut
                break;
            case SDL_CONTROLLER_BUTTON_Y:
                g_presets.toggleShuffle();
                break;
            case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
                adjustAudioGain(-0.1f);
                break;
            case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
                adjustAudioGain(+0.1f);
                break;
            case SDL_CONTROLLER_BUTTON_DPAD_UP: {
                float sens = projectm_get_beat_sensitivity(g_pm);
                sens = (sens < 4.9f) ? sens + 0.2f : 5.0f;
                projectm_set_beat_sensitivity(g_pm, sens);
                fprintf(stderr, "[Vibeus] Beat sensitivity: %.1f\n", sens);
                break;
            }
            case SDL_CONTROLLER_BUTTON_DPAD_DOWN: {
                float sens = projectm_get_beat_sensitivity(g_pm);
                sens = (sens > 0.2f) ? sens - 0.2f : 0.0f;
                projectm_set_beat_sensitivity(g_pm, sens);
                fprintf(stderr, "[Vibeus] Beat sensitivity: %.1f\n", sens);
                break;
            }
            default: break;
            }
            continue;
        }

        switch (event.type) {
        case SDL_KEYDOWN:
            handleKeyDown(event.key.keysym);
            break;

        case SDL_MOUSEBUTTONDOWN:
            if (event.button.button == SDL_BUTTON_LEFT) {
                g_mouseDown = true;
                float nx, ny;
                screenToNormalized(event.button.x, event.button.y, nx, ny);
                projectm_touch(g_pm, nx, ny, g_touchPressure, g_touchTypes[g_touchTypeIndex]);
            } else if (event.button.button == SDL_BUTTON_RIGHT) {
                // Cycle touch waveform type
                g_touchTypeIndex = (g_touchTypeIndex + 1) % g_touchTypeCount;
                fprintf(stderr, "[Vibeus] Touch type: %s\n", touchTypeName(g_touchTypes[g_touchTypeIndex]));
            }
            break;

        case SDL_MOUSEMOTION:
            if (g_mouseDown) {
                float nx, ny;
                screenToNormalized(event.motion.x, event.motion.y, nx, ny);
                projectm_touch_drag(g_pm, nx, ny, g_touchPressure);
            }
            break;

        case SDL_MOUSEBUTTONUP:
            if (event.button.button == SDL_BUTTON_LEFT) {
                g_mouseDown = false;
                // Waveforms persist — use C key or right-click to manage
            }
            break;

        case SDL_MOUSEWHEEL:
            if (SDL_GetModState() & KMOD_SHIFT) {
                // Shift+scroll = cycle touch type
                if (event.wheel.y > 0)
                    g_touchTypeIndex = (g_touchTypeIndex + 1) % g_touchTypeCount;
                else if (event.wheel.y < 0)
                    g_touchTypeIndex = (g_touchTypeIndex - 1 + g_touchTypeCount) % g_touchTypeCount;
                fprintf(stderr, "[Vibeus] Touch type: %s\n", touchTypeName(g_touchTypes[g_touchTypeIndex]));
            } else {
                // Scroll = adjust touch pressure (0–2)
                g_touchPressure += (event.wheel.y > 0) ? 1 : -1;
                if (g_touchPressure < 0) g_touchPressure = 0;
                if (g_touchPressure > 2) g_touchPressure = 2;
                fprintf(stderr, "[Vibeus] Touch pressure: %d\n", g_touchPressure);
            }
            break;

        case SDL_WINDOWEVENT:
            if (event.window.event == SDL_WINDOWEVENT_RESIZED ||
                event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                int w = event.window.data1;
                int h = event.window.data2;
                projectm_set_window_size(g_pm, w, h);
            }
            break;

        default:
            break;
        }
    }
}

// ----- Apply Configuration -----

static void applyConfig(const VibeusConfig& cfg)
{
    // Audio
    g_audioGain = cfg.audioGain;
    if (g_pm) {
        projectm_set_beat_sensitivity(g_pm, cfg.beatSensitivity);

        // Presets
        projectm_set_preset_duration(g_pm, cfg.presetDuration);
        projectm_set_soft_cut_duration(g_pm, cfg.transitionTime);
        projectm_set_hard_cut_enabled(g_pm, cfg.autoAdvance);
    }

    // Shuffle
    if (cfg.shuffle != g_presets.isShuffled())
        g_presets.toggleShuffle();

    // Display
    if (cfg.fullscreen != g_fullscreen)
        toggleFullscreen();

    g_debug = cfg.showFps;
    if (!g_debug)
        SDL_SetWindowTitle(g_window, "Vibeus");

    // Motion
    g_speedMultiplier = cfg.speedMultiplier;
    g_flowMode = cfg.flowMode;

    // UI scale (base 1.35 * user factor)
    ImGui::GetIO().FontGlobalScale = 1.35f * cfg.fontScale;
}

// ----- Handle Menu Actions -----

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
        // Try to load favorites
        {
            auto exePath = fs::path(SDL_GetBasePath());
            std::string favPath = (exePath / "favorites.txt").string();
            g_menu.loadFavorites(favPath);
        }
        g_menu.showScreen(UIScreen::PresetBrowser);
        break;

    case MenuAction::PlayPreset: {
        uint32_t idx = g_menu.selectedPresetIndex();
        projectm_playlist_set_position(g_presets.handle(), idx, true);
        fprintf(stderr, "[Vibeus] Playing preset #%u\n", idx);
        // If we were in main menu, start the visualizer
        if (g_appState == AppState::MainMenu) {
            g_appState = AppState::Visualizer;
            g_menu.hideAll();
            g_paused = false;
            g_lastFrameTicks = SDL_GetTicks();
        }
        // If paused, resume
        else if (g_paused) {
            resumeFromPause();
            g_menu.hideAll();
        }
        break;
    }

    case MenuAction::BackToPause:
        // Save favorites before leaving browser
        {
            auto exePath = fs::path(SDL_GetBasePath());
            std::string favPath = (exePath / "favorites.txt").string();
            g_menu.saveFavorites(favPath);
        }
        g_menu.showScreen(UIScreen::PauseMenu);
        break;

    case MenuAction::Settings:
        // Remember where we came from so Back returns correctly
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
        // Save favorites on exit
        {
            auto exePath = fs::path(SDL_GetBasePath());
            std::string favPath = (exePath / "favorites.txt").string();
            g_menu.saveFavorites(favPath);
        }
        saveConfig(g_config, g_configPath);
        g_running = false;
        break;

    case MenuAction::ApplySettings:
        applyConfig(g_config);
        saveConfig(g_config, g_configPath);
        break;

    case MenuAction::BackFromSettings:
        applyConfig(g_config);
        saveConfig(g_config, g_configPath);
        // Return to the screen that opened settings
        {
            UIScreen returnTo = g_menu.currentScreen(); // still Settings
            (void)returnTo;
        }
        // The overlay tracks m_settingsReturnTo internally
        g_menu.showScreen(g_menu.settingsReturnScreen());
        break;

    default:
        break;
    }

    // Also save favorites when returning to main menu from browser
    if (action == MenuAction::BackToMenu &&
        g_menu.currentScreen() == UIScreen::MainMenu) {
        auto exePath = fs::path(SDL_GetBasePath());
        std::string favPath = (exePath / "favorites.txt").string();
        g_menu.saveFavorites(favPath);
    }
}

// ----- Main -----

int main(int argc, char* argv[])
{
    // Check for --debug flag
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--debug") == 0 || strcmp(argv[i], "-d") == 0)
            g_debug = true;
    }

#ifdef _WIN32
    if (g_debug)
        attachConsole();
#endif

    fprintf(stderr, "=== Vibeus v0.2.0 ===\n");
    if (g_debug)
        fprintf(stderr, "[DEBUG MODE ENABLED]\n");
    fprintf(stderr, "\n");

    // 1. Initialize SDL + OpenGL
    if (!initSDL()) return 1;
    fprintf(stderr, "[Vibeus] SDL2 + OpenGL 3.3 context ready\n");

    // 2. Set up projectM logging
    if (g_debug) {
        projectm_set_log_level(PROJECTM_LOG_LEVEL_DEBUG, false);
    } else {
        projectm_set_log_level(PROJECTM_LOG_LEVEL_WARN, false);
    }
    projectm_set_log_callback(projectmLogCallback, false, nullptr);

    // 3. Initialize projectM
    if (!initProjectM()) {
        SDL_GL_DeleteContext(g_glCtx);
        SDL_DestroyWindow(g_window);
        SDL_Quit();
        return 1;
    }

    // 4. Initialize audio capture
    if (!g_audio.init()) {
        fprintf(stderr, "[Vibeus] WARNING: Audio capture failed - visuals won't react to music\n");
    }

    // 5. Load presets
    std::string presetsDir = findPresetsDir();
    fprintf(stderr, "[Vibeus] Presets directory: %s\n", presetsDir.c_str());
    g_presets.init(g_pm, presetsDir);

    fprintf(stderr, "\n[Vibeus] Ready! %u presets loaded. Shuffle: %s\n",
            g_presets.count(), g_presets.isShuffled() ? "ON" : "OFF");
    fprintf(stderr, "[Vibeus] Controls:\n");
    fprintf(stderr, "  N/Right=next  P/Left=prev  R=random  H=history  S=shuffle\n");
    fprintf(stderr, "  F/F11=fullscreen  D=debug  Up/Down=beat sensitivity\n");
    fprintf(stderr, "  [/]=speed down/up  Backspace=reset speed\n");
    fprintf(stderr, "  -/==audio gain down/up  0=reset audio\n");
    fprintf(stderr, "  Click=touch waveform  Drag=move  RightClick=cycle type\n");
    fprintf(stderr, "  Scroll=pressure  Shift+Scroll=cycle type  C=clear waveforms\n");
    fprintf(stderr, "  Tab=flow mode  Esc=pause menu  Q=quit\n");
    fprintf(stderr, "  Gamepad: A=next B=prev X=random Y=shuffle Start=pause\n\n");

    // 6. Initialize menu overlay
    if (!g_menu.init(g_window, g_glCtx)) {
        fprintf(stderr, "[Vibeus] WARNING: Menu overlay init failed\n");
    }

    // 7. Load user configuration
    g_configPath = (fs::path(SDL_GetBasePath()) / "vibeus_config.json").string();
    g_config = loadConfig(g_configPath);
    g_menu.setConfigPtr(&g_config);

    // Apply saved config to all systems (except fullscreen on startup — handled by config value)
    {
        // Sync config → globals without triggering fullscreen toggle at launch
        bool savedFullscreen = g_config.fullscreen;
        g_config.fullscreen = g_fullscreen; // pretend it matches current state
        applyConfig(g_config);
        g_config.fullscreen = savedFullscreen; // restore for future use
        // If user wants fullscreen on startup, toggle it now
        if (savedFullscreen && !g_fullscreen)
            toggleFullscreen();
    }
    fprintf(stderr, "[Vibeus] Config loaded from %s\n", g_configPath.c_str());

    // 8. Try to open gamepad
    tryOpenGamepad();

    // 9. Start with epilepsy splash screen
    g_appState = AppState::Splash;
    g_menu.showScreen(UIScreen::Splash);

    // 10. Main render loop
    Uint32 frameDelay = 1000 / TARGET_FPS;
    g_fpsTimer = SDL_GetTicks();
    g_lastFrameTicks = g_fpsTimer;

    while (g_running) {
        Uint32 frameStart = SDL_GetTicks();

        // Process input
        processEvents();

        // State-specific rendering
        switch (g_appState) {
        case AppState::Splash:
        case AppState::MainMenu:
            // Render a single projectM frame as eye-candy background
            if (g_appState == AppState::MainMenu || g_appState == AppState::Splash) {
                // Slowly advance time for subtle background animation
                static double menuBgTime = 0.0;
                menuBgTime += 0.016 * 0.3; // 30% speed
                projectm_set_frame_time(g_pm, menuBgTime);
                g_audio.feedAudio(g_pm, 0.3f); // low audio reactivity for bg
                projectm_opengl_render_frame(g_pm);
            }
            break;

        case AppState::Visualizer:
            if (!g_paused) {
                // Process analog gamepad input
                processGamepad();
                // Process flow mode (mouse-driven manipulation)
                processFlowMode();
                // Update virtual time for speed control
                updateVirtualTime(SDL_GetTicks());
                // Capture audio and feed to projectM (with gain applied)
                g_audio.feedAudio(g_pm, g_audioGain);
                // Render visualization frame
                projectm_opengl_render_frame(g_pm);
            } else {
                // While paused, render a few frozen frames then stop
                if (g_pauseRenderCount < 3) {
                    projectm_set_frame_time(g_pm, g_pausedTime);
                    projectm_opengl_render_frame(g_pm);
                    g_pauseRenderCount++;
                }
            }
            break;
        }

        // Render UI overlay if visible (draws on top of everything)
        if (g_menu.isVisible()) {
            MenuAction action = g_menu.render();
            if (action != MenuAction::None)
                handleMenuAction(action);
        }

        SDL_GL_SwapWindow(g_window);

        // FPS tracking
        g_frameCount++;
        Uint32 now = SDL_GetTicks();
        if (now - g_fpsTimer >= 1000) {
            g_currentFps = g_frameCount * 1000.0f / (now - g_fpsTimer);
            g_frameCount = 0;
            g_fpsTimer = now;
            if (g_appState == AppState::Visualizer)
                updateDebugTitle();
        }

        // Frame rate limiter
        Uint32 elapsed = SDL_GetTicks() - frameStart;
        if (elapsed < frameDelay) {
            SDL_Delay(frameDelay - elapsed);
        }
    }

    // 11. Cleanup
    fprintf(stderr, "\n[Vibeus] Shutting down...\n");
    g_menu.shutdown();
    g_audio.shutdown();
    if (g_gamepad) {
        SDL_GameControllerClose(g_gamepad);
        g_gamepad = nullptr;
    }
    projectm_destroy(g_pm);
    g_pm = nullptr;

    SDL_GL_DeleteContext(g_glCtx);
    SDL_DestroyWindow(g_window);
    SDL_Quit();

    return 0;
}
