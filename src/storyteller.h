#ifndef STORYTELLER_H
#define STORYTELLER_H

#include <projectM-4/projectM.h>
#include "preset_manager.h"

enum class StoryState { Chill, Buildup, Drop, Sustain };

struct StorySettings {
    float dropSensitivity = 2.0f;
    float bassReactivity = 1.0f;
    float sustainMaxSec = 12.0f;
    int transitionStyle = 0;
    bool buildupLockPreset = true;
    bool debugMode = false;
    // Advanced gates/thresholds (exposed in settings)
    float chillGateSeconds = 1.5f;
    float buildupMinSeconds = 0.8f;
    float buildupRatioGate = 1.02f;
    float buildupFluxGate = 0.05f;
    float dropRatioGate = 1.03f;
    float dropBassGate = 1.15f;
    float dropFluxGate = 0.06f;
    float sensChill = 1.0f;
    float sensBuildup = 1.3f;
    float sensDrop = 3.0f;
    float sensSustain = 2.0f;
    float minSensitivity = 0.5f;
    float maxSensitivity = 2.8f;   // lowered from 3.5 to reduce "SUPER fast" feeling
    float sensitivityDecay = 0.1f;
    float beatReactivity = 1.0f;   // global multiplier applied to all sensitivity targets (safe, non-compounding)
};

class Storyteller {
public:
    void init(projectm_handle pm, PresetManager* presets);
    void update(float peak, float bass, float beatBass, float midEnergy, float highEnergy, float deltaTime);
    void applySettings(const StorySettings& settings);
    void forceState(StoryState state);
    StoryState currentState() const { return m_state; }
    float lastEnergyRatio() const { return m_lastEnergyRatio; }
    float beatThreshold() const { return m_beatThreshold; }
    float spectralFlux() const { return m_spectralFlux; }
    bool beatActive() const { return m_beatActive; }
    float lastBeatSensitivity() const { return m_currentBeatSensitivity; }
private:
    void transitionTo(StoryState newState);
    projectm_handle m_pm = nullptr;
    PresetManager* m_presets = nullptr;
    StoryState m_state = StoryState::Chill;
    StorySettings m_settings;
    float m_stateTimer = 0.0f, m_cooldown = 0.0f, m_beatCooldown = 0.0f;
    float m_shortTermEnergy = 0.0f, m_longTermEnergy = 0.0f, m_spectralFlux = 0.0f;
    float m_lastPeak = 0.0f, m_lastBass = 0.0f, m_lastBeatBass = 0.0f;
    float m_bassEnergy = 0.0f, m_bassLongTerm = 0.0f, m_beatThreshold = 0.05f;
    float m_lastEnergyRatio = 1.0f;
    bool m_beatActive = false;
    float m_currentBeatSensitivity = 1.0f, m_buildupIntensity = 0.0f;
    int m_dropCount = 0, m_historyIdx = 0;
    static constexpr int HISTORY_SIZE = 60;
    float m_peakHistory[HISTORY_SIZE] = {0};
    float m_baseSoftCut = 3.0f;
};

#endif
