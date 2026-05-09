#ifndef STORYTELLER_H
#define STORYTELLER_H

#include <projectM-4/projectM.h>
#include "preset_manager.h"

enum class StoryState { Chill, Buildup, Drop, Sustain };

struct StorySettings {
    float dropSensitivity   = 2.0f;
    float bassReactivity    = 1.0f;
    float sustainMaxSec     = 12.0f;
    int   transitionStyle   = 0;
    bool  buildupLockPreset = true;
    bool  debugMode         = false;
    bool  externalTransitionCompositor = false;

    // State-gate thresholds
    float chillGateSeconds   = 1.5f;
    float buildupMinSeconds  = 0.8f;
    float buildupRatioGate   = 1.05f;  // any-band short/long ratio to enter buildup
    float buildupFluxGate    = 0.04f;  // spectral flux gate for buildup
    float dropRatioGate      = 1.15f;  // combined-band ratio to trigger drop
    float dropBassGate       = 1.20f;  // bass-band ratio specifically for drop
    float dropFluxGate       = 0.07f;  // spectral flux gate for drop

    // Per-state sensitivity targets
    float sensChill   = 0.9f;
    float sensBuildup = 1.4f;
    float sensDrop    = 2.8f;
    float sensSustain = 2.0f;

    // Safety limits
    float minSensitivity   = 0.5f;
    float maxSensitivity   = 2.8f;
    float sensitivityDecay = 0.1f;

    // Global multiplier applied to all targets (non-compounding)
    float beatReactivity = 1.0f;
};

class Storyteller {
public:
    void init(projectm_handle pm, PresetManager* presets);

    // All three band energies + their onset flags (consumed from AudioCapture)
    void update(float peak,
                float bass,   float mid,   float high,
                float beatBass, float beatMid, float beatHigh,
                bool  onsetLow, bool onsetMid, bool onsetHigh,
                float deltaTime);

    void applySettings(const StorySettings& settings);
    void forceState(StoryState state);

    // Accessors for HUD / debug overlay
    StoryState currentState()        const { return m_state; }
    float lastEnergyRatio()          const { return m_lastEnergyRatio; }
    float beatThreshold()            const { return m_beatThreshold; }
    float spectralFlux()             const { return m_spectralFlux; }
    bool  beatActive()               const { return m_beatActive; }
    float lastBeatSensitivity()      const { return m_currentBeatSensitivity; }

private:
    void transitionTo(StoryState newState);
    void setSoftCutDuration(float seconds);

    projectm_handle  m_pm      = nullptr;
    PresetManager*   m_presets = nullptr;

    StoryState m_state     = StoryState::Chill;
    StorySettings m_settings;

    // Timers
    float m_stateTimer  = 0.0f;
    float m_cooldown    = 0.0f;

    // Per-band short-term EMAs (fast envelope)
    float m_bassShort = 0.0f, m_midShort = 0.0f, m_highShort = 0.0f;
    // Per-band long-term EMAs (rolling baseline)
    float m_bassLong  = 0.0f, m_midLong  = 0.0f, m_highLong  = 0.0f;
    // Wideband
    float m_shortTermEnergy = 0.0f, m_longTermEnergy = 0.0f;

    // Spectral flux (wideband)
    float m_spectralFlux = 0.0f;
    float m_lastPeak     = 0.0f;

    // Diagnostics
    float m_lastEnergyRatio        = 1.0f;
    float m_beatThreshold          = 0.05f; // legacy, kept for HUD
    bool  m_beatActive             = false;
    float m_currentBeatSensitivity = 1.0f;
    float m_buildupIntensity       = 0.0f;
    int   m_dropCount              = 0;

    // Onset latch: each onset is consumed once per frame but we keep a
    // smoothed "recent activity" counter per band so transitions feel organic
    float m_lowOnsetActivity  = 0.0f;  // decays over ~0.4 s
    float m_midOnsetActivity  = 0.0f;
    float m_highOnsetActivity = 0.0f;

    float m_baseSoftCut = 3.0f;
};

#endif // STORYTELLER_H
