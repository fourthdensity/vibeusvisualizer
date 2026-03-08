#pragma once

#include <string>

// Mood presets: composite parameter bundles
enum class MoodPreset { Chill, Party, Focus, Psychedelic, Custom };

// Performance mode tiers
enum class PerfMode { BatterySaver, Balanced, Quality };

struct VibeusConfig {
    // ── Audio ──
    float audioGain         = 1.0f;     // 0.0 – 3.0
    float beatSensitivity   = 1.0f;     // 0.0 – 5.0

    // ── Presets ──
    bool  autoAdvance       = true;
    float presetDuration    = 30.0f;    // 10 – 120 seconds
    bool  shuffle           = true;
    float transitionTime    = 3.0f;     // 0.0 – 10.0 seconds (soft cut)
    bool  hardCutEnabled    = false;    // beat-triggered instant transitions
    float hardCutSensitivity= 2.0f;    // 0.5 – 4.0 (lower = more frequent)
    float hardCutDuration   = 20.0f;   // min seconds before hard cut allowed

    // ── Display ──
    bool  fullscreen        = false;
    float uiScale           = 1.0f;     // 0.75 – 2.0
    bool  showFps           = false;
    PerfMode perfMode       = PerfMode::Quality;

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

    // ── Overlay ──
    float overlayOpacity    = 0.65f;    // 0.1 – 0.95 (glass transparency)
};

// Load config from JSON file. Returns defaults if file doesn't exist or is corrupt.
VibeusConfig loadConfig(const std::string& path);

// Save config to JSON file. Returns true on success.
bool saveConfig(const VibeusConfig& cfg, const std::string& path);

// Apply a mood preset to a config (overwrites relevant fields).
void applyMoodPreset(VibeusConfig& cfg, MoodPreset mood);
