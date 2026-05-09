#include "storyteller.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <projectM-4/parameters.h>
#include <projectM-4/playlist.h>

// ──────────────────────────────────────────────────────────────────────────────
//  Helpers
// ──────────────────────────────────────────────────────────────────────────────

static inline float emaStep(float current, float target, float tau, float dt) {
    // One-pole low-pass: current += (target - current) * (1 - exp(-dt/tau))
    return current + (target - current) * (1.0f - std::exp(-dt / tau));
}

static inline float safeRatio(float shortV, float longV, float floor = 0.02f) {
    float denom = std::max(longV, floor);
    float r = shortV / denom;
    return std::isnan(r) ? 1.0f : r;
}

struct TransitionProfile {
    const char* name;
    float softCutSeconds;
    float sensitivityBoost;
    float cooldownSeconds;
    bool hardAdvance;
    bool cleanStart;
};

constexpr TransitionProfile kDropTransitions[] = {
    { "Flash Cut",      0.25f, 0.25f, 2.6f, true,  false },
    { "Slow Morph",     3.00f, 0.00f, 3.2f, false, false },
    { "Quick Blend",    1.00f, 0.10f, 2.8f, false, false },
    { "Glitch Cut",     0.15f, 0.35f, 2.2f, true,  true  },
    { "Zoom Burst",     3.50f, 0.15f, 3.4f, false, false },
    { "Energy Flash",   0.60f, 0.45f, 2.4f, true,  false },
    { "Snap Fade",      0.08f, 0.25f, 2.0f, true,  false },
    { "Long Dissolve",  6.00f, 0.00f, 4.0f, false, false },
    { "Pulse Blend",    1.25f, 0.20f, 2.6f, false, false },
    { "Bass Slam",      0.18f, 0.55f, 2.2f, true,  false },
    { "Breathing Fade", 7.00f, 0.00f, 4.2f, false, false },
    { "Clean Slate",    0.35f, 0.10f, 2.6f, true,  true  },
    { "Liquid Drift",   4.80f, 0.05f, 3.8f, false, false },
    { "Ambient Wash",   8.50f, 0.00f, 4.5f, false, false },
    { "Spark Jump",     0.70f, 0.35f, 2.4f, true,  false },
    { "Deep Bloom",     2.80f, 0.25f, 3.2f, false, false },
    { "Afterimage",     3.80f, 0.10f, 3.5f, false, false },
    { "Drop Smash",     0.05f, 0.65f, 2.0f, true,  true  },
};

constexpr int kDropTransitionCount = sizeof(kDropTransitions) / sizeof(kDropTransitions[0]);

// ──────────────────────────────────────────────────────────────────────────────
//  Init
// ──────────────────────────────────────────────────────────────────────────────

void Storyteller::init(projectm_handle pm, PresetManager* presets) {
    m_pm      = pm;
    m_presets = presets;

    m_state     = StoryState::Chill;
    m_stateTimer = 0.0f;
    m_cooldown   = 0.0f;

    m_bassShort = m_midShort = m_highShort = 0.0f;
    m_bassLong  = m_midLong  = m_highLong  = 0.0f;
    m_shortTermEnergy = m_longTermEnergy   = 0.0f;

    m_spectralFlux = 0.0f;
    m_lastPeak     = 0.0f;

    m_lastEnergyRatio        = 1.0f;
    m_beatThreshold          = 0.05f;
    m_beatActive             = false;
    m_currentBeatSensitivity = 1.0f;
    m_buildupIntensity       = 0.0f;
    m_dropCount              = 0;

    m_lowOnsetActivity = m_midOnsetActivity = m_highOnsetActivity = 0.0f;

    if (m_pm)
        m_baseSoftCut = static_cast<float>(projectm_get_soft_cut_duration(m_pm));
}

// ──────────────────────────────────────────────────────────────────────────────
//  Main per-frame update
// ──────────────────────────────────────────────────────────────────────────────

void Storyteller::update(
        float peak,
        float bass,   float mid,   float high,
        float beatBass, float beatMid, float beatHigh,
        bool  onsetLow, bool onsetMid_, bool onsetHigh_,
        float deltaTime)
{
    if (!m_pm || !m_presets) return;
    if (deltaTime <= 0.0f || deltaTime > 1.0f) return;

    // ── Timers ────────────────────────────────────────────────────────────────
    if (m_cooldown > 0.0f) m_cooldown -= deltaTime;
    m_stateTimer += deltaTime;

    // ── Per-band EMAs ─────────────────────────────────────────────────────────
    // Short windows: fast attack so we catch transients
    // Long windows: slow baseline so ratios have meaning
    const float shortTauBass = 0.10f;  // ~100 ms
    const float shortTauMid  = 0.08f;  // ~80 ms  (mids / snares respond faster)
    const float shortTauHigh = 0.06f;  // ~60 ms  (hi-hats are very brief)
    const float longTau      = 3.50f;  // ~3.5 s  same for all bands

    m_bassShort = emaStep(m_bassShort, beatBass, shortTauBass, deltaTime);
    m_midShort  = emaStep(m_midShort,  beatMid,  shortTauMid,  deltaTime);
    m_highShort = emaStep(m_highShort, beatHigh, shortTauHigh, deltaTime);

    m_bassLong = emaStep(m_bassLong, beatBass, longTau, deltaTime);
    m_midLong  = emaStep(m_midLong,  beatMid,  longTau, deltaTime);
    m_highLong = emaStep(m_highLong, beatHigh, longTau, deltaTime);

    // Wideband (for gate checks that don't care which band)
    m_shortTermEnergy = emaStep(m_shortTermEnergy, peak, 0.15f, deltaTime);
    m_longTermEnergy  = emaStep(m_longTermEnergy,  peak, 4.00f, deltaTime);

    // ── Spectral Flux (wideband onset strength) ───────────────────────────────
    float flux = std::max(0.0f, peak - m_lastPeak);
    m_spectralFlux = emaStep(m_spectralFlux, flux, 0.05f, deltaTime);
    m_lastPeak = peak;

    // ── Per-band ratios ───────────────────────────────────────────────────────
    float bassRatio = safeRatio(m_bassShort, m_bassLong, 0.02f);
    float midRatio  = safeRatio(m_midShort,  m_midLong,  0.01f);
    float highRatio = safeRatio(m_highShort, m_highLong, 0.005f);
    float wideRatio = safeRatio(m_shortTermEnergy, m_longTermEnergy, 0.03f);

    // Combined "energy surge" - weighted average shaped by the user-facing
    // bass reactivity control. Higher values make kicks/subs dominate drop
    // detection; lower values let mids/highs carry more of the story.
    float bassWeight = std::clamp(0.45f * m_settings.bassReactivity, 0.09f, 1.35f);
    float midWeight  = 0.35f;
    float highWeight = 0.20f;
    float combinedRatio = (bassWeight * bassRatio + midWeight * midRatio + highWeight * highRatio) /
                          (bassWeight + midWeight + highWeight);

    float dropSensitivity = std::clamp(m_settings.dropSensitivity, 1.0f, 4.0f);
    float dropGateScale = 0.88f + ((dropSensitivity - 1.0f) / 3.0f) * 0.32f;
    float bassGateScale = dropGateScale / std::clamp(std::sqrt(m_settings.bassReactivity), 0.75f, 1.35f);

    m_lastEnergyRatio = wideRatio;

    // Legacy threshold kept for the HUD display only
    m_beatThreshold = emaStep(m_beatThreshold, beatBass, 12.5f, deltaTime); // ~τ=80 ms
    m_beatThreshold = std::max(m_beatThreshold, 0.01f);

    // ── Onset activity accumulators ───────────────────────────────────────────
    // Each onset fires a brief "activity spike" that decays over ~0.4 s.
    // This smooths out single-frame noise and lets the state machine react to
    // a pattern rather than a single transient.
    const float onsetDecay = 0.40f; // tau in seconds
    if (onsetLow)   m_lowOnsetActivity  = 1.0f;
    if (onsetMid_)  m_midOnsetActivity  = 1.0f;
    if (onsetHigh_) m_highOnsetActivity = 1.0f;
    m_lowOnsetActivity  = emaStep(m_lowOnsetActivity,  0.0f, onsetDecay, deltaTime);
    m_midOnsetActivity  = emaStep(m_midOnsetActivity,  0.0f, onsetDecay, deltaTime);
    m_highOnsetActivity = emaStep(m_highOnsetActivity, 0.0f, onsetDecay, deltaTime);

    m_beatActive = (onsetLow || onsetMid_ || onsetHigh_);

    // ── Dynamic Sensitivity ───────────────────────────────────────────────────
    // Target is driven by state, then shaped by the dominant active band.
    float targetSens;
    float decayTau;
    switch (m_state) {
        case StoryState::Chill:
            targetSens = m_settings.sensChill;
            decayTau   = 0.4f;
            break;
        case StoryState::Buildup:
            // Sensitivity rises proportionally to how long and how intensely
            // the buildup has been going — feels like tension increasing.
            targetSens = m_settings.sensBuildup + m_buildupIntensity * 1.2f;
            decayTau   = 0.8f;
            break;
        case StoryState::Drop:
            targetSens = m_settings.sensDrop;
            decayTau   = 1.2f;
            break;
        case StoryState::Sustain:
            targetSens = m_settings.sensSustain;
            decayTau   = 1.5f;
            break;
        default:
            targetSens = 1.0f;
            decayTau   = 0.4f;
            break;
    }

    // Organic shaping: nudge sensitivity slightly toward whichever band is
    // most active right now (adds subtle pulse even within a state).
    float bandBoost = std::max({m_lowOnsetActivity, m_midOnsetActivity, m_highOnsetActivity});
    bandBoost = std::clamp(bandBoost * 0.4f, 0.0f, 0.4f);

    float desired = (targetSens + bandBoost) * m_settings.beatReactivity;
    float sensAlpha = 1.0f - std::exp(-deltaTime / (m_settings.sensitivityDecay * decayTau));
    m_currentBeatSensitivity += (desired - m_currentBeatSensitivity) * sensAlpha;
    m_currentBeatSensitivity = std::clamp(m_currentBeatSensitivity,
                                          m_settings.minSensitivity,
                                          m_settings.maxSensitivity);
    projectm_set_beat_sensitivity(m_pm, m_currentBeatSensitivity);

    // ── State Machine ─────────────────────────────────────────────────────────
    if (m_cooldown > 0.0f) {
        // Still in post-transition cooldown — update diagnostics then return
        goto diagnostics;
    }

    switch (m_state) {
    // ── CHILL ──────────────────────────────────────────────────────────────────
    // Enter Buildup when any band's energy rises above baseline, or there's a
    // notable onset burst from mid/high (hi-hat rolls, synth risers).
    case StoryState::Chill: {
        if (m_stateTimer < m_settings.chillGateSeconds) break;

        // Bass kick onset with energy swell
        bool bassBuildup = (onsetLow && bassRatio > m_settings.buildupRatioGate * bassGateScale);
        // Mid/high pattern (snare rolls, hats, synth rising) — no kick needed
        bool midBuildup  = (midRatio  > m_settings.buildupRatioGate * 1.10f
                            && m_midOnsetActivity > 0.3f);
        bool highBuildup = (highRatio > m_settings.buildupRatioGate * 1.25f
                            && m_highOnsetActivity > 0.4f);
        // Wideband flux surge (handles dense music with no clear kick)
        bool fluxBuildup = (m_spectralFlux > m_settings.buildupFluxGate
                            && wideRatio > m_settings.buildupRatioGate);

        if (bassBuildup || midBuildup || highBuildup || fluxBuildup)
            transitionTo(StoryState::Buildup);
        break;
    }

    // ── BUILDUP ────────────────────────────────────────────────────────────────
    // Intensity accumulates. Drop triggers on a strong multi-band surge OR a
    // clear kick + bass swell. Falls back to Chill if energy drops.
    case StoryState::Buildup: {
        m_buildupIntensity = std::min(m_buildupIntensity + deltaTime / 5.0f, 1.0f);

        if (m_stateTimer < m_settings.buildupMinSeconds) break;

        // Primary drop: strong kick onset + combined-band surge
        bool kickDrop = (onsetLow && combinedRatio > m_settings.dropRatioGate * dropGateScale);
        // Bass-heavy drop (sub-bass swell even without sharp kick transient)
        bool bassDrop = (bassRatio > m_settings.dropBassGate * bassGateScale);
        // Mid+high drop: big snare hit or synth stab on top of building energy
        bool midHighDrop = (onsetMid_ && midRatio > m_settings.dropRatioGate * dropGateScale * 0.9f
                            && m_buildupIntensity > 0.25f);
        // Flux surge (catches dense drops that hit all bands simultaneously)
        bool fluxDrop = (m_spectralFlux > m_settings.dropFluxGate
                         && combinedRatio > m_settings.dropRatioGate * dropGateScale * 0.85f);

        if (kickDrop || bassDrop || midHighDrop || fluxDrop) {
            transitionTo(StoryState::Drop);
        } else if (combinedRatio < 0.88f && m_stateTimer > 4.0f) {
            // Energy fell away — false buildup, back to chill
            transitionTo(StoryState::Chill);
        }
        break;
    }

    // ── DROP ───────────────────────────────────────────────────────────────────
    // Hold for a minimum period, then decide whether we're sustaining or chilling.
    case StoryState::Drop: {
        const float dropMinHold = 6.0f;
        if (m_stateTimer < dropMinHold) break;

        // Sustain if the energy is still elevated AND there's ongoing activity
        bool stillHot = (combinedRatio > 1.10f && (m_lowOnsetActivity > 0.1f || m_midOnsetActivity > 0.1f));
        transitionTo(stillHot ? StoryState::Sustain : StoryState::Chill);
        break;
    }

    // ── SUSTAIN ────────────────────────────────────────────────────────────────
    // Re-drop on strong multi-band beats. Chill when energy subsides.
    case StoryState::Sustain: {
        // Re-drop: kick + combined surge (keeps the energy loop going)
        if (m_stateTimer > 3.0f && onsetLow && combinedRatio > 1.08f)
            transitionTo(StoryState::Drop);
        // Quiet down: low combined activity
        else if (m_stateTimer > 5.0f && combinedRatio < 0.93f)
            transitionTo(StoryState::Chill);
        // Hard timeout
        else if (m_stateTimer > m_settings.sustainMaxSec)
            transitionTo(StoryState::Chill);
        break;
    }
    }

diagnostics:
    // ── Debug Telemetry ───────────────────────────────────────────────────────
    if (m_settings.debugMode) {
        static uint32_t fc = 0;
        if (fc++ % 10 == 0) {
            const char* s = (m_state == StoryState::Chill)     ? "CHILL"
                          : (m_state == StoryState::Buildup)   ? "BUILD"
                          : (m_state == StoryState::Drop)      ? "DROP "
                                                               : "SUST ";
            fprintf(stderr,
                "[Story] %-5s t=%5.2f | bRatio=%4.2f mRatio=%4.2f hRatio=%4.2f comb=%4.2f"
                " | onL=%s onM=%s onH=%s"
                " | act(L=%.2f M=%.2f H=%.2f)"
                " | flux=%.3f | sens=%.2f bldI=%.2f\n",
                s, m_stateTimer,
                bassRatio, midRatio, highRatio, combinedRatio,
                onsetLow ? "Y" : ".", onsetMid_ ? "Y" : ".", onsetHigh_ ? "Y" : ".",
                m_lowOnsetActivity, m_midOnsetActivity, m_highOnsetActivity,
                m_spectralFlux, m_currentBeatSensitivity, m_buildupIntensity);
        }
    }
}

// ──────────────────────────────────────────────────────────────────────────────
//  State Transitions
// ──────────────────────────────────────────────────────────────────────────────

void Storyteller::transitionTo(StoryState newState) {
    if (!m_pm || !m_presets) return;

    StoryState prev = m_state;
    m_state      = newState;
    m_stateTimer = 0.0f;

    switch (newState) {
    // ── → CHILL ───────────────────────────────────────────────────────────────
    case StoryState::Chill:
        fprintf(stderr, "[Storyteller] → CHILL (prev=%d ratio=%.2f)\n",
                (int)prev, m_lastEnergyRatio);
        projectm_set_preset_locked(m_pm, false);
        projectm_set_preset_start_clean(m_pm, false);
        if (prev == StoryState::Drop || prev == StoryState::Sustain) {
            // Coming down from high energy — long dreamy crossfade
            setSoftCutDuration(5.0f);
            m_currentBeatSensitivity = std::clamp(
                1.4f * m_settings.beatReactivity,
                m_settings.minSensitivity, m_settings.maxSensitivity);
        } else {
            setSoftCutDuration(m_baseSoftCut);
            m_currentBeatSensitivity = std::clamp(
                0.9f * m_settings.beatReactivity,
                m_settings.minSensitivity, m_settings.maxSensitivity);
        }
        projectm_set_beat_sensitivity(m_pm, m_currentBeatSensitivity);
        m_buildupIntensity  = 0.0f;
        m_lowOnsetActivity  = 0.0f;
        m_midOnsetActivity  = 0.0f;
        m_highOnsetActivity = 0.0f;
        m_cooldown = 2.0f;
        break;

    // ── → BUILDUP ─────────────────────────────────────────────────────────────
    case StoryState::Buildup:
        fprintf(stderr, "[Storyteller] → BUILDUP\n");
        projectm_set_preset_locked(m_pm, m_settings.buildupLockPreset);
        projectm_set_preset_start_clean(m_pm, false);
        setSoftCutDuration(4.0f);
        m_currentBeatSensitivity = std::clamp(
            1.4f * m_settings.beatReactivity,
            m_settings.minSensitivity, m_settings.maxSensitivity);
        projectm_set_beat_sensitivity(m_pm, m_currentBeatSensitivity);
        m_buildupIntensity = 0.0f;
        m_cooldown = 0.6f;
        break;

    // ── → DROP ────────────────────────────────────────────────────────────────
    case StoryState::Drop: {
        // Determine transition style (cycling for variety)
        int style;
        if (m_settings.transitionStyle == 0) {
            style = m_dropCount % kDropTransitionCount;
            m_dropCount++;
        } else {
            style = std::clamp(m_settings.transitionStyle - 1, 0, kDropTransitionCount - 1);
        }
        const TransitionProfile& profile = kDropTransitions[style];
        fprintf(stderr, "[Storyteller] → DROP (%s style=%d buildup=%.2f)\n",
                profile.name, style, m_buildupIntensity);

        // How built-up were we? Scale transition speed accordingly.
        float intensityScale = std::clamp((1.0f - m_buildupIntensity) * 0.8f, 0.0f, 0.8f);
        float softCut = profile.softCutSeconds;
        if (softCut > 1.0f) {
            softCut += intensityScale;
        } else if (softCut > 0.25f) {
            softCut += intensityScale * 0.25f;
        }

        projectm_set_preset_start_clean(m_pm, profile.cleanStart);
        setSoftCutDuration(softCut);

        m_presets->next(m_settings.externalTransitionCompositor || profile.hardAdvance);
        projectm_set_preset_locked(m_pm, false);

        // Earned drops hit harder when buildup was longer/stronger
        float dropSens = (2.0f + m_buildupIntensity * 1.5f + profile.sensitivityBoost) * m_settings.beatReactivity;
        m_currentBeatSensitivity = std::clamp(dropSens,
                                              m_settings.minSensitivity,
                                              m_settings.maxSensitivity);
        projectm_set_beat_sensitivity(m_pm, m_currentBeatSensitivity);
        m_cooldown = profile.cooldownSeconds;
        break;
    }

    // ── → SUSTAIN ─────────────────────────────────────────────────────────────
    case StoryState::Sustain:
        fprintf(stderr, "[Storyteller] → SUSTAIN (ratio=%.2f)\n", m_lastEnergyRatio);
        projectm_set_preset_locked(m_pm, true);
        projectm_set_preset_start_clean(m_pm, false);
        setSoftCutDuration(2.5f);
        m_currentBeatSensitivity = std::clamp(
            2.0f * m_settings.beatReactivity,
            m_settings.minSensitivity, m_settings.maxSensitivity);
        projectm_set_beat_sensitivity(m_pm, m_currentBeatSensitivity);
        m_cooldown = 2.0f;
        break;
    }
}

void Storyteller::setSoftCutDuration(float seconds)
{
    if (!m_pm)
        return;

    // When the Vibeus-owned compositor is enabled, projectM must switch presets
    // immediately and let the post-FX layer perform the visible opacity blend.
    // Leaving projectM's internal crossfade active creates competing fades.
    float applied = m_settings.externalTransitionCompositor ? 0.05f : seconds;
    projectm_set_soft_cut_duration(m_pm, applied);
}

// ──────────────────────────────────────────────────────────────────────────────
//  Public API
// ──────────────────────────────────────────────────────────────────────────────

void Storyteller::forceState(StoryState state) { transitionTo(state); }
void Storyteller::applySettings(const StorySettings& settings) { m_settings = settings; }
