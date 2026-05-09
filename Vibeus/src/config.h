#pragma once

#include <string>
#include <vector>
#include <SDL_keycode.h>
#include <SDL_gamecontroller.h>

// Mood presets: composite parameter bundles
enum class MoodPreset { Chill, Party, Focus, Psychedelic, Custom };

// Performance mode tiers
enum class PerfMode { BatterySaver, Balanced, Quality };

struct VibeusConfig {
    // ── Audio ──
    float audioGain         = 1.0f;     // 0.0 – 3.0
    float beatSensitivity   = 1.0f;     // 0.0 – 5.0
    float beatReactivity    = 1.0f;     // 0.5 – 2.0 multiplier for storyteller dynamic sensitivity
    bool  adaptiveBeat      = true;     // allow storyteller to dynamically adjust beat sensitivity
    float beatHoldTime      = 0.15f;    // seconds - minimum time between beat triggers (hysteresis)

    // ── Presets ──
    bool  autoAdvance       = true;
    float presetDuration    = 30.0f;    // 10 – 120 seconds
    bool  shuffle           = true;
    float transitionTime    = 3.0f;     // 0.0 – 10.0 seconds (soft cut)
    bool  hardCutEnabled    = false;    // beat-triggered instant transitions
    float hardCutSensitivity= 2.0f;    // 0.5 – 4.0 (lower = more frequent)
    float hardCutDuration   = 20.0f;   // min seconds before hard cut allowed
    bool  transitionCompositorEnabled = true; // Vibeus-owned post-processing pulses
    float transitionCrossfadeDuration = 0.75f; // 0.5 - 1.2 seconds, cinematic post-FX opacity blend
    float easterEgg         = 0.5f;     // 0.0 – 1.0 (preset variety/randomness)

    // ── Display ──
    bool  fullscreen        = false;
    // uiScale removed - was never implemented
    bool  showFps           = false;
    PerfMode perfMode       = PerfMode::Quality;
    float meshDetail        = 128.0f;   // 32.0 – 128.0 (warp mesh resolution)
    bool  aspectCorrection  = true;     // fix ultrawide stretching

    // ── Safety / Accessibility ──
    bool  flashLimiter      = false;
    bool  reducedMotion     = false;
    float fontScale         = 1.0f;     // 0.75 – 2.0

    // ── Mood ──
    MoodPreset mood         = MoodPreset::Custom;
    bool  vibeLock          = false;

    // ── Advanced: Motion ──
    float speedMultiplier   = 1.0f;     // 0.05 – 4.0

    // ── Advanced: Input ──
    int   gamepadDeadzone   = 8000;     // 2000 – 16000
    bool  flowMode          = false;
    bool  storytellingEnabled = true;  // Intelligent audio-reactive storytelling (default ON)

    // ── Storytelling Tuning ──
    float storyDropSensitivity = 2.0f;  // 1.0 – 4.0 (lower = drops trigger more easily)
    float storyBassReactivity  = 1.0f;  // 0.2 – 3.0 (how much bass/kicks influence detection)
    float storySustainMax      = 12.0f; // 4.0 – 30.0 (max seconds to hold Sustain before returning to Chill)
    int   storyTransitionStyle = 0;     // 0=Cycling, 1..18=explicit drop transition profiles
    bool  storyBuildupLock     = true;  // Lock preset during buildup (prevents jarring mid-buildup switches)
    bool  storyDebug           = false; // Verbose per-frame storyteller telemetry
    bool  storyDebugRescue     = true;  // Auto-skip broken presets detected at runtime
    // Storytelling advanced gates/thresholds
    float storyChillGateSec    = 1.5f;  // Time to linger in Chill before buildup allowed (prev: 3.0)
    float storyBuildupMinSec   = 0.8f;  // Minimum time in Buildup before a Drop can fire (prev: 1.0)
    float storyBuildupRatio    = 1.02f; // Short/long energy ratio needed to start buildup (prev: 1.02)
    float storyBuildupFlux     = 0.05f; // Flux required to help start buildup (prev: 0.05)
    float storyDropRatio       = 1.03f; // Ratio required to fire drop (prev: 1.05)
    float storyDropBass        = 1.15f; // Bass ratio required to fire drop (prev: 1.25)
    float storyDropFlux        = 0.06f; // Flux required to fire drop (prev: 0.08)

    // ── Stasis (freeze visuals when audio is silent / paused) ──
    bool  stasisEnabled     = true;     // When ON, visuals freeze after prolonged silence
    float stasisThreshold   = 0.008f;   // RMS level below this = considered silence
    float stasisFadeTime    = 0.75f;    // seconds of continuous silence before entering stasis

    bool  touchEnabled      = false;    // mouse/gamepad touch waveforms (currently disabled in dev builds)

    // ── Preset Management ──
    std::vector<std::string> favoritePresetPaths;   // persisted absolute paths to favorited .milk files
    // Off by default to avoid long first launch / "Not responding" reports on installed builds.
    bool  validatePresetsOnStartup = false; // Scan can be enabled in Settings → Advanced

    // ── Overlay ──
    float overlayOpacity    = 0.65f;    // 0.1 – 0.95 (glass transparency)

    // ── Control Bindings (Keyboard) ──
    int keyNextPreset       = SDLK_n;
    int keyPrevPreset       = SDLK_p;
    int keyRandomPreset     = SDLK_r;
    int keyHistory          = SDLK_h;
    int keyShuffle          = SDLK_s;
    int keyFullscreen       = SDLK_f;
    int keyDebug            = SDLK_d;
    int keyQuit             = SDLK_q;
    int keyBeatSensUp       = SDLK_UP;
    int keyBeatSensDown     = SDLK_DOWN;
    int keySpeedUp          = SDLK_RIGHTBRACKET;
    int keySpeedDown        = SDLK_LEFTBRACKET;
    int keySpeedReset       = SDLK_BACKSPACE;
    int keyAudioGainUp      = SDLK_EQUALS;
    int keyAudioGainDown    = SDLK_MINUS;
    int keyAudioGainReset   = SDLK_0;

    // ── Control Bindings (Gamepad) ──
    int gpNextPreset        = SDL_CONTROLLER_BUTTON_A;
    int gpPrevPreset        = SDL_CONTROLLER_BUTTON_B;
    int gpRandomPreset      = SDL_CONTROLLER_BUTTON_X;
    int gpShuffle           = SDL_CONTROLLER_BUTTON_Y;
    int gpAudioGainUp       = SDL_CONTROLLER_BUTTON_RIGHTSHOULDER;
    int gpAudioGainDown     = SDL_CONTROLLER_BUTTON_LEFTSHOULDER;
};

// Load config from JSON file. Returns defaults if file doesn't exist or is corrupt.
VibeusConfig loadConfig(const std::string& path);

// Save config to JSON file. Returns true on success.
bool saveConfig(const VibeusConfig& cfg, const std::string& path);

// Apply a mood preset to a config (overwrites relevant fields).
void applyMoodPreset(VibeusConfig& cfg, MoodPreset mood);
