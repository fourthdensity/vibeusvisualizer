/**
 * Vibeus - A custom music visualizer frontend powered by projectM
 *
 * Barebones skeleton: SDL2 window + OpenGL + WASAPI loopback + projectM rendering
 *
 * Controls:
 *   N / Right     - Next preset
 *   P / Left      - Previous preset
 *   R             - Random preset (hard cut)
 *   H             - Go back in history
 *   S             - Toggle shuffle
 *   F / F11       - Toggle fullscreen
 *   Up/Down       - Adjust beat sensitivity
 *   D             - Toggle debug overlay (FPS + preset in title)
 *   [ / ]         - Slow down / speed up visualization
 *   Backspace     - Reset speed to 1.0x
 *   - / =         - Audio reactivity down / up
 *   0             - Reset audio reactivity to 1.0x
 *   Mouse click   - Create touch waveform (persists after release)
 *   Mouse drag    - Move touch waveform
 *   Right-click   - Cycle waveform type
 *   Scroll wheel  - Adjust touch pressure (0-2)
 *   Shift+Scroll  - Cycle waveform type
 *   C             - Clear all touch waveforms
 *   Escape / Q    - Quit
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

// Debug stats
static Uint32 g_frameCount  = 0;
static Uint32 g_fpsTimer    = 0;
static float  g_currentFps  = 0.0f;

// Speed control - virtual clock fed to projectM
static double g_speedMultiplier = 1.0;    // 0.05x to 4.0x
static double g_virtualTime     = 0.0;    // accumulated virtual seconds
static bool   g_useVirtualTime  = false;  // false = real-time (default)
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
             "Vibeus | %.0f FPS | %.2fx speed | %.0f%% audio | Touch: %s P%d | [%u/%u] %s",
             g_currentFps, g_speedMultiplier, g_audioGain * 100.0f,
             touchTypeName(g_touchTypes[g_touchTypeIndex]), g_touchPressure,
             g_presets.position() + 1, g_presets.count(), preset.c_str());
    SDL_SetWindowTitle(g_window, title);
}

// ----- Speed Control -----

static void adjustSpeed(double delta)
{
    g_speedMultiplier += delta;
    // Clamp to [0.05, 4.0]
    if (g_speedMultiplier < 0.05) g_speedMultiplier = 0.05;
    if (g_speedMultiplier > 4.0)  g_speedMultiplier = 4.0;

    if (g_speedMultiplier == 1.0) {
        // Back to real-time: disable virtual clock
        g_useVirtualTime = false;
        projectm_set_frame_time(g_pm, -1.0);
    } else {
        g_useVirtualTime = true;
    }
    fprintf(stderr, "[Vibeus] Speed: %.2fx%s\n",
            g_speedMultiplier, g_useVirtualTime ? "" : " (real-time)");
}

static void resetSpeed()
{
    g_speedMultiplier = 1.0;
    g_useVirtualTime = false;
    projectm_set_frame_time(g_pm, -1.0);
    fprintf(stderr, "[Vibeus] Speed reset to 1.0x (real-time)\n");
}

static void updateVirtualTime(Uint32 nowTicks)
{
    if (!g_useVirtualTime) {
        g_lastFrameTicks = nowTicks;
        return;
    }
    double realDelta = (nowTicks - g_lastFrameTicks) / 1000.0;
    g_lastFrameTicks = nowTicks;
    g_virtualTime += realDelta * g_speedMultiplier;
    projectm_set_frame_time(g_pm, g_virtualTime);
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
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
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
    switch (key.sym) {
    case SDLK_ESCAPE:
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
        switch (event.type) {
        case SDL_QUIT:
            g_running = false;
            break;

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

    fprintf(stderr, "=== Vibeus v0.1.0 ===\n");
    if (g_debug)
        fprintf(stderr, "[DEBUG MODE ENABLED]\n");
    fprintf(stderr, "\n");

    // 1. Initialize SDL + OpenGL
    if (!initSDL()) return 1;
    fprintf(stderr, "[Vibeus] SDL2 + OpenGL 3.3 context ready\n");

    // 2. Set up projectM logging (before create so we see init messages)
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
    fprintf(stderr, "  Q/Esc=quit\n\n");

    // 6. Main render loop
    Uint32 frameDelay = 1000 / TARGET_FPS;
    g_fpsTimer = SDL_GetTicks();
    g_lastFrameTicks = g_fpsTimer;

    while (g_running) {
        Uint32 frameStart = SDL_GetTicks();

        // Process input
        processEvents();

        // Update virtual time for speed control
        updateVirtualTime(SDL_GetTicks());

        // Capture audio and feed to projectM (with gain applied)
        g_audio.feedAudio(g_pm, g_audioGain);

        // Render frame
        projectm_opengl_render_frame(g_pm);
        SDL_GL_SwapWindow(g_window);

        // FPS tracking
        g_frameCount++;
        Uint32 now = SDL_GetTicks();
        if (now - g_fpsTimer >= 1000) {
            g_currentFps = g_frameCount * 1000.0f / (now - g_fpsTimer);
            g_frameCount = 0;
            g_fpsTimer = now;
            updateDebugTitle();
        }

        // Frame rate limiter
        Uint32 elapsed = SDL_GetTicks() - frameStart;
        if (elapsed < frameDelay) {
            SDL_Delay(frameDelay - elapsed);
        }
    }

    // 7. Cleanup
    fprintf(stderr, "\n[Vibeus] Shutting down...\n");
    g_audio.shutdown();
    // PresetManager and projectM cleaned up by destructors / explicit destroy
    projectm_destroy(g_pm);
    g_pm = nullptr;

    SDL_GL_DeleteContext(g_glCtx);
    SDL_DestroyWindow(g_window);
    SDL_Quit();

    return 0;
}
