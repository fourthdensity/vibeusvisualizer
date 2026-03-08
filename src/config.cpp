#include "config.h"

#include <nlohmann/json.hpp>
#include <fstream>

using json = nlohmann::json;

VibeusConfig loadConfig(const std::string& path)
{
    VibeusConfig cfg;
    std::ifstream f(path);
    if (!f.is_open()) return cfg;

    try {
        json j = json::parse(f);

        auto get = [&](const char* key, auto& field) {
            if (j.contains(key)) field = j[key].get<std::remove_reference_t<decltype(field)>>();
        };

        get("audioGain",       cfg.audioGain);
        get("beatSensitivity", cfg.beatSensitivity);
        get("autoAdvance",     cfg.autoAdvance);
        get("presetDuration",  cfg.presetDuration);
        get("shuffle",         cfg.shuffle);
        get("transitionTime",  cfg.transitionTime);
        get("hardCutEnabled",   cfg.hardCutEnabled);
        get("hardCutSensitivity", cfg.hardCutSensitivity);
        get("hardCutDuration",  cfg.hardCutDuration);
        get("fullscreen",      cfg.fullscreen);
        get("uiScale",         cfg.uiScale);
        get("showFps",         cfg.showFps);
        get("flashLimiter",    cfg.flashLimiter);
        get("reducedMotion",   cfg.reducedMotion);
        get("fontScale",       cfg.fontScale);
        get("vibeLock",        cfg.vibeLock);
        get("speedMultiplier", cfg.speedMultiplier);
        get("gamepadDeadzone", cfg.gamepadDeadzone);
        get("flowMode",        cfg.flowMode);
        get("overlayOpacity",  cfg.overlayOpacity);

        // Enums stored as ints
        if (j.contains("perfMode")) {
            int v = j["perfMode"].get<int>();
            if (v >= 0 && v <= 2) cfg.perfMode = static_cast<PerfMode>(v);
        }
        if (j.contains("mood")) {
            int v = j["mood"].get<int>();
            if (v >= 0 && v <= 4) cfg.mood = static_cast<MoodPreset>(v);
        }
    } catch (...) {
        // Corrupt file → use defaults
    }

    return cfg;
}

bool saveConfig(const VibeusConfig& cfg, const std::string& path)
{
    json j;

    j["audioGain"]       = cfg.audioGain;
    j["beatSensitivity"] = cfg.beatSensitivity;
    j["autoAdvance"]     = cfg.autoAdvance;
    j["presetDuration"]  = cfg.presetDuration;
    j["shuffle"]         = cfg.shuffle;
    j["transitionTime"]  = cfg.transitionTime;
    j["hardCutEnabled"]   = cfg.hardCutEnabled;
    j["hardCutSensitivity"] = cfg.hardCutSensitivity;
    j["hardCutDuration"]  = cfg.hardCutDuration;
    j["fullscreen"]      = cfg.fullscreen;
    j["uiScale"]         = cfg.uiScale;
    j["showFps"]         = cfg.showFps;
    j["perfMode"]        = static_cast<int>(cfg.perfMode);
    j["flashLimiter"]    = cfg.flashLimiter;
    j["reducedMotion"]   = cfg.reducedMotion;
    j["fontScale"]       = cfg.fontScale;
    j["mood"]            = static_cast<int>(cfg.mood);
    j["vibeLock"]        = cfg.vibeLock;
    j["speedMultiplier"] = cfg.speedMultiplier;
    j["gamepadDeadzone"] = cfg.gamepadDeadzone;
    j["flowMode"]        = cfg.flowMode;
    j["overlayOpacity"]  = cfg.overlayOpacity;

    std::ofstream f(path);
    if (!f.is_open()) return false;
    f << j.dump(2);
    return f.good();
}

void applyMoodPreset(VibeusConfig& cfg, MoodPreset mood)
{
    switch (mood) {
    case MoodPreset::Chill:
        cfg.audioGain       = 0.80f;
        cfg.beatSensitivity = 0.5f;
        cfg.presetDuration  = 45.0f;
        cfg.transitionTime  = 5.0f;
        cfg.speedMultiplier = 0.8f;
        cfg.reducedMotion   = false;
        cfg.flashLimiter    = false;
        cfg.hardCutEnabled  = false;
        break;
    case MoodPreset::Party:
        cfg.audioGain       = 1.50f;
        cfg.beatSensitivity = 2.0f;
        cfg.presetDuration  = 20.0f;
        cfg.transitionTime  = 1.5f;
        cfg.speedMultiplier = 1.2f;
        cfg.hardCutEnabled  = true;
        cfg.hardCutSensitivity = 1.5f;
        cfg.hardCutDuration = 10.0f;
        break;
    case MoodPreset::Focus:
        cfg.audioGain       = 0.60f;
        cfg.beatSensitivity = 0.3f;
        cfg.presetDuration  = 60.0f;
        cfg.transitionTime  = 8.0f;
        cfg.speedMultiplier = 0.6f;
        cfg.reducedMotion   = true;
        cfg.hardCutEnabled  = false;
        break;
    case MoodPreset::Psychedelic:
        cfg.audioGain       = 1.20f;
        cfg.beatSensitivity = 1.5f;
        cfg.presetDuration  = 25.0f;
        cfg.transitionTime  = 3.0f;
        cfg.speedMultiplier = 1.0f;
        cfg.hardCutEnabled  = true;
        cfg.hardCutSensitivity = 2.0f;
        cfg.hardCutDuration = 15.0f;
        break;
    case MoodPreset::Custom:
        break; // leave everything as-is
    }
}
