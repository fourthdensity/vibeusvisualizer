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
        get("beatReactivity",  cfg.beatReactivity);
        get("adaptiveBeat",    cfg.adaptiveBeat);
        get("beatHoldTime",    cfg.beatHoldTime);
        get("autoAdvance",     cfg.autoAdvance);
        get("presetDuration",  cfg.presetDuration);
        get("shuffle",         cfg.shuffle);
        get("transitionTime",  cfg.transitionTime);
        get("favoritePresetPaths", cfg.favoritePresetPaths);
        get("hardCutEnabled",   cfg.hardCutEnabled);
        get("hardCutSensitivity", cfg.hardCutSensitivity);
        get("hardCutDuration",  cfg.hardCutDuration);
        get("transitionCompositorEnabled", cfg.transitionCompositorEnabled);
        get("transitionCrossfadeDuration", cfg.transitionCrossfadeDuration);
        get("easterEgg",       cfg.easterEgg);
        get("fullscreen",      cfg.fullscreen);
        // uiScale removed - was never implemented
        get("showFps",         cfg.showFps);
        get("meshDetail",      cfg.meshDetail);
        get("aspectCorrection", cfg.aspectCorrection);
        get("flashLimiter",    cfg.flashLimiter);
        get("reducedMotion",   cfg.reducedMotion);
        get("fontScale",       cfg.fontScale);
        get("vibeLock",        cfg.vibeLock);
        get("speedMultiplier", cfg.speedMultiplier);
        get("gamepadDeadzone", cfg.gamepadDeadzone);
        get("flowMode",        cfg.flowMode);
        get("storytellingEnabled", cfg.storytellingEnabled);
        get("storyDropSensitivity", cfg.storyDropSensitivity);
        get("storyBassReactivity",  cfg.storyBassReactivity);
        get("storySustainMax",      cfg.storySustainMax);
        get("storyTransitionStyle", cfg.storyTransitionStyle);
        get("storyBuildupLock",     cfg.storyBuildupLock);
        get("storyDebug",           cfg.storyDebug);
        get("storyDebugRescue",     cfg.storyDebugRescue);
        get("storyChillGateSec",    cfg.storyChillGateSec);
        get("storyBuildupMinSec",   cfg.storyBuildupMinSec);
        get("storyBuildupRatio",    cfg.storyBuildupRatio);
        get("storyBuildupFlux",     cfg.storyBuildupFlux);
        get("storyDropRatio",       cfg.storyDropRatio);
        get("storyDropBass",        cfg.storyDropBass);
        get("storyDropFlux",        cfg.storyDropFlux);

        // Stasis (freeze when silent)
        get("stasisEnabled",        cfg.stasisEnabled);
        get("stasisThreshold",      cfg.stasisThreshold);
        get("stasisFadeTime",       cfg.stasisFadeTime);

        get("touchEnabled",         cfg.touchEnabled);
        get("validatePresetsOnStartup", cfg.validatePresetsOnStartup);
        get("overlayOpacity",  cfg.overlayOpacity);

        // Control bindings (keyboard) - with defaults if missing
        if (j.contains("keyNextPreset")) cfg.keyNextPreset = j["keyNextPreset"].get<int>();
        if (j.contains("keyPrevPreset")) cfg.keyPrevPreset = j["keyPrevPreset"].get<int>();
        if (j.contains("keyRandomPreset")) cfg.keyRandomPreset = j["keyRandomPreset"].get<int>();
        if (j.contains("keyHistory")) cfg.keyHistory = j["keyHistory"].get<int>();
        if (j.contains("keyShuffle")) cfg.keyShuffle = j["keyShuffle"].get<int>();
        if (j.contains("keyFullscreen")) cfg.keyFullscreen = j["keyFullscreen"].get<int>();
        if (j.contains("keyDebug")) cfg.keyDebug = j["keyDebug"].get<int>();
        if (j.contains("keyQuit")) cfg.keyQuit = j["keyQuit"].get<int>();
        if (j.contains("keyBeatSensUp")) cfg.keyBeatSensUp = j["keyBeatSensUp"].get<int>();
        if (j.contains("keyBeatSensDown")) cfg.keyBeatSensDown = j["keyBeatSensDown"].get<int>();
        if (j.contains("keySpeedUp")) cfg.keySpeedUp = j["keySpeedUp"].get<int>();
        if (j.contains("keySpeedDown")) cfg.keySpeedDown = j["keySpeedDown"].get<int>();
        if (j.contains("keySpeedReset")) cfg.keySpeedReset = j["keySpeedReset"].get<int>();
        if (j.contains("keyAudioGainUp")) cfg.keyAudioGainUp = j["keyAudioGainUp"].get<int>();
        if (j.contains("keyAudioGainDown")) cfg.keyAudioGainDown = j["keyAudioGainDown"].get<int>();
        if (j.contains("keyAudioGainReset")) cfg.keyAudioGainReset = j["keyAudioGainReset"].get<int>();

        // Control bindings (gamepad)
        if (j.contains("gpNextPreset")) cfg.gpNextPreset = j["gpNextPreset"].get<int>();
        if (j.contains("gpPrevPreset")) cfg.gpPrevPreset = j["gpPrevPreset"].get<int>();
        if (j.contains("gpRandomPreset")) cfg.gpRandomPreset = j["gpRandomPreset"].get<int>();
        if (j.contains("gpShuffle")) cfg.gpShuffle = j["gpShuffle"].get<int>();
        if (j.contains("gpAudioGainUp")) cfg.gpAudioGainUp = j["gpAudioGainUp"].get<int>();
        if (j.contains("gpAudioGainDown")) cfg.gpAudioGainDown = j["gpAudioGainDown"].get<int>();

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
    j["beatReactivity"]  = cfg.beatReactivity;
    j["adaptiveBeat"]    = cfg.adaptiveBeat;
    j["beatHoldTime"]    = cfg.beatHoldTime;
    j["autoAdvance"]     = cfg.autoAdvance;
    j["presetDuration"]  = cfg.presetDuration;
    j["shuffle"]         = cfg.shuffle;
    j["transitionTime"]  = cfg.transitionTime;
    j["favoritePresetPaths"] = cfg.favoritePresetPaths;
    j["hardCutEnabled"]   = cfg.hardCutEnabled;
    j["hardCutSensitivity"] = cfg.hardCutSensitivity;
    j["hardCutDuration"]  = cfg.hardCutDuration;
    j["transitionCompositorEnabled"] = cfg.transitionCompositorEnabled;
    j["transitionCrossfadeDuration"] = cfg.transitionCrossfadeDuration;
    j["easterEgg"]       = cfg.easterEgg;
    j["fullscreen"]      = cfg.fullscreen;
    // uiScale removed - was never implemented
    j["showFps"]         = cfg.showFps;
    j["perfMode"]        = static_cast<int>(cfg.perfMode);
    j["meshDetail"]      = cfg.meshDetail;
    j["aspectCorrection"] = cfg.aspectCorrection;
    j["flashLimiter"]    = cfg.flashLimiter;
    j["reducedMotion"]   = cfg.reducedMotion;
    j["fontScale"]       = cfg.fontScale;
    j["mood"]            = static_cast<int>(cfg.mood);
    j["vibeLock"]        = cfg.vibeLock;
    j["speedMultiplier"] = cfg.speedMultiplier;
    j["gamepadDeadzone"] = cfg.gamepadDeadzone;
    j["flowMode"]        = cfg.flowMode;
    j["storytellingEnabled"] = cfg.storytellingEnabled;
    j["storyDropSensitivity"] = cfg.storyDropSensitivity;
    j["storyBassReactivity"]  = cfg.storyBassReactivity;
    j["storySustainMax"]      = cfg.storySustainMax;
    j["storyTransitionStyle"] = cfg.storyTransitionStyle;
    j["storyBuildupLock"]     = cfg.storyBuildupLock;
    j["storyDebug"]           = cfg.storyDebug;
    j["storyDebugRescue"]     = cfg.storyDebugRescue;
    j["storyChillGateSec"]    = cfg.storyChillGateSec;
    j["storyBuildupMinSec"]   = cfg.storyBuildupMinSec;
    j["storyBuildupRatio"]    = cfg.storyBuildupRatio;
    j["storyBuildupFlux"]     = cfg.storyBuildupFlux;
    j["storyDropRatio"]       = cfg.storyDropRatio;
    j["storyDropBass"]        = cfg.storyDropBass;
    j["storyDropFlux"]        = cfg.storyDropFlux;

    // Stasis
    j["stasisEnabled"]        = cfg.stasisEnabled;
    j["stasisThreshold"]      = cfg.stasisThreshold;
    j["stasisFadeTime"]       = cfg.stasisFadeTime;

    j["touchEnabled"]         = cfg.touchEnabled;
    j["validatePresetsOnStartup"] = cfg.validatePresetsOnStartup;
    j["overlayOpacity"]  = cfg.overlayOpacity;

    // Control bindings (keyboard)
    j["keyNextPreset"]      = cfg.keyNextPreset;
    j["keyPrevPreset"]      = cfg.keyPrevPreset;
    j["keyRandomPreset"]    = cfg.keyRandomPreset;
    j["keyHistory"]         = cfg.keyHistory;
    j["keyShuffle"]         = cfg.keyShuffle;
    j["keyFullscreen"]      = cfg.keyFullscreen;
    j["keyDebug"]           = cfg.keyDebug;
    j["keyQuit"]            = cfg.keyQuit;
    j["keyBeatSensUp"]      = cfg.keyBeatSensUp;
    j["keyBeatSensDown"]    = cfg.keyBeatSensDown;
    j["keySpeedUp"]         = cfg.keySpeedUp;
    j["keySpeedDown"]       = cfg.keySpeedDown;
    j["keySpeedReset"]      = cfg.keySpeedReset;
    j["keyAudioGainUp"]     = cfg.keyAudioGainUp;
    j["keyAudioGainDown"]   = cfg.keyAudioGainDown;
    j["keyAudioGainReset"]  = cfg.keyAudioGainReset;

    // Control bindings (gamepad)
    j["gpNextPreset"]       = cfg.gpNextPreset;
    j["gpPrevPreset"]       = cfg.gpPrevPreset;
    j["gpRandomPreset"]     = cfg.gpRandomPreset;
    j["gpShuffle"]          = cfg.gpShuffle;
    j["gpAudioGainUp"]      = cfg.gpAudioGainUp;
    j["gpAudioGainDown"]    = cfg.gpAudioGainDown;

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
