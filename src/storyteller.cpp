#include "storyteller.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <projectM-4/parameters.h>
#include <projectM-4/playlist.h>


// --- Beat Onset Detection (refractory + EMA) ---
// 90 ms refractory prevents double-triggering on the same kick transient.
// EMA provides adaptive threshold without the runaway behavior of the old WMA.
static constexpr float kBeatRefractorySec = 0.090f; // 90 ms

struct BeatDetector {
  float ema = 0.0f;
  float lastBeatTime = -1.0f;

  bool isBeatOnset(float currentBass, float now, float refractorySec = kBeatRefractorySec) {
    // Fast-attack EMA (alpha tuned for kick transients)
    const float alpha = 0.35f;
    ema = (alpha * currentBass) + ((1.0f - alpha) * ema);

    // Primary: sharp onset (1.35x is more forgiving than 1.75x for real music)
    bool onset = (currentBass > 1.35f * ema) &&
                 (now - lastBeatTime > refractorySec);

    // Secondary: sustained strong bass (helps when there are no sharp transients)
    // If currentBass is significantly above its own EMA, treat as a beat event.
    if (!onset && currentBass > 2.2f * ema && (now - lastBeatTime > refractorySec * 0.6f)) {
      onset = true;
    }

    if (onset) {
      lastBeatTime = now;
    }
    return onset;
  }
};

static BeatDetector g_beatDetector;
void Storyteller::init(projectm_handle pm, PresetManager *presets) {
  m_pm = pm;
  m_presets = presets;
  m_state = StoryState::Chill;
  m_shortTermEnergy = 0.0f;
  m_longTermEnergy = 0.0f;
  m_spectralFlux = 0.0f;
  m_lastPeak = 0.0f;
  m_lastBass = 0.0f;
  m_lastBeatBass = 0.0f;
  m_bassEnergy = 0.0f;
  m_bassLongTerm = 0.0f;
  m_beatThreshold = 0.05f;
  m_beatActive = false;
  m_stateTimer = 0.0f;
  m_cooldown = 0.0f;
  m_beatCooldown = 0.0f;
  m_currentBeatSensitivity = 1.0f;
  m_buildupIntensity = 0.0f;
  m_dropCount = 0;
  m_historyIdx = 0;
  std::memset(m_peakHistory, 0, sizeof(m_peakHistory));
  if (m_pm) {
    m_baseSoftCut = static_cast<float>(projectm_get_soft_cut_duration(m_pm));
  }
  // Reset EMA detector state on re-init
  g_beatDetector = BeatDetector{};
}

void Storyteller::update(float peak, float bass, float beatBass, float midEnergy, float highEnergy,
                         float deltaTime) {
  if (!m_pm || !m_presets)
    return;
  if (deltaTime <= 0.0f || deltaTime > 1.0f)
    return;

  const float dropSens = m_settings.dropSensitivity;
  const float bassWeight = m_settings.bassReactivity;
  const float sustainMax = m_settings.sustainMaxSec;
  const float chillGate = std::clamp(m_settings.chillGateSeconds, 0.1f, 10.0f);
  const float buildupMin = std::clamp(m_settings.buildupMinSeconds, 0.1f, 5.0f);
  const float buildRatioGate = std::clamp(m_settings.buildupRatioGate, 1.0f, 2.0f);
  const float buildFluxGate = std::clamp(m_settings.buildupFluxGate, 0.0f, 0.5f);
  const float dropRatioGate = std::clamp(m_settings.dropRatioGate, 1.0f, 3.0f);
  const float dropBassGate = std::clamp(m_settings.dropBassGate, 1.0f, 3.0f);
  const float dropFluxGate = std::clamp(m_settings.dropFluxGate, 0.0f, 0.5f);

  // --- Timers ---
  if (m_cooldown > 0.0f)
    m_cooldown -= deltaTime;
  if (m_beatCooldown > 0.0f)
    m_beatCooldown -= deltaTime;
  m_stateTimer += deltaTime;

  // --- EMA Beat Onset (replaces WMA + Schmitt) ---
  // Uses 90 ms refractory + fast-attack EMA to detect kick transients cleanly.
  // This is the single source of truth for "beat happened right now".
  bool triggered = g_beatDetector.isBeatOnset(beatBass, m_stateTimer);

  // Lightweight diagnostic telemetry — only active when the user enables
  // storyteller debug logs, otherwise this would spam the normal app log.
  static int diagCounter = 0;
  if (m_settings.debugMode && ++diagCounter % 10 == 0) {
    fprintf(stderr, "[StoryDiag] beatBass=%.5f ema=%.5f triggered=%s state=%d timer=%.2f\n",
            beatBass, g_beatDetector.ema, triggered ? "YES" : "no", (int)m_state, m_stateTimer);
  }

  // Legacy threshold kept for debug telemetry only (not used for triggering)
  m_beatThreshold = (m_beatThreshold * 0.92f) + (beatBass * 0.08f);
  m_beatThreshold = std::max(m_beatThreshold, 0.01f);

  // --- Spectral Flux (Onset Detection) ---
  // Measure rectified change in full-spectrum energy
  float flux = std::max(0.0f, peak - m_lastPeak);
  m_spectralFlux = (m_spectralFlux * 0.8f) + (flux * 0.2f); // smoothed flux

  // --- Energy Tracking (EMA) ---
  float shortAlpha = 1.0f - std::exp(-deltaTime / 0.15f);
  float longAlpha = 1.0f - std::exp(-deltaTime / 4.0f);
  m_shortTermEnergy += (peak - m_shortTermEnergy) * shortAlpha;
  m_longTermEnergy += (peak - m_longTermEnergy) * longAlpha;

  float bassShortAlpha = 1.0f - std::exp(-deltaTime / 0.1f);
  float bassLongAlpha = 1.0f - std::exp(-deltaTime / 3.0f);
  m_bassEnergy += (bass - m_bassEnergy) * bassShortAlpha;
  m_bassLongTerm += (bass - m_bassLongTerm) * bassLongAlpha;

  m_lastPeak = peak;
  m_lastBass = bass;
  m_lastBeatBass = beatBass;

  // --- Derived Signals ---
  float baseline = std::max(m_longTermEnergy, 0.03f);
  float ratio = m_shortTermEnergy / baseline;
  float bassBaseline = std::max(m_bassLongTerm, 0.02f);
  float bassRatio = (m_bassEnergy / bassBaseline) * bassWeight;

  // --- Dynamic Sensitivity (driven by state) ---
  float targetSens = 1.0f;
  float decayTau = 0.3f;

  // NOTE: We no longer directly scale by beatBass/m_beatThreshold here.
  // That path was causing runaway sensitivity. The state machine + decay
  // below is the only place that sets targetSens.

  switch (m_state) {
  case StoryState::Chill:
    targetSens = m_settings.sensChill;
    decayTau = 0.4f;
    break;
  case StoryState::Buildup:
    targetSens = m_settings.sensBuildup + m_buildupIntensity * 1.5f;
    decayTau = 0.8f;
    break;
  case StoryState::Drop:
    targetSens = m_settings.sensDrop;
    decayTau = 1.2f;
    break;
  case StoryState::Sustain:
    targetSens = m_settings.sensSustain;
    decayTau = 1.5f;
    break;
  }

  // Exponential decay toward target (always respect beatReactivity and hard limits)
  float decayAlpha = 1.0f - std::exp(-deltaTime / (m_settings.sensitivityDecay * decayTau));
  float desired = targetSens * m_settings.beatReactivity;
  m_currentBeatSensitivity += (desired - m_currentBeatSensitivity) * decayAlpha;

  // Hard safety clamp every frame (prevents any runaway from previous bugs or bad config)
  m_currentBeatSensitivity = std::clamp(m_currentBeatSensitivity, m_settings.minSensitivity, m_settings.maxSensitivity);
  projectm_set_beat_sensitivity(m_pm, m_currentBeatSensitivity);

  // --- State Machine ---
  if (m_cooldown <= 0.0f) {
    // Prevent NaN propagation in signal analysis
    if (std::isnan(ratio))
      ratio = 1.0f;
    if (std::isnan(bassRatio))
      bassRatio = 1.0f;
    if (std::isnan(m_spectralFlux))
      m_spectralFlux = 0.0f;

    m_lastEnergyRatio = ratio;

    switch (m_state) {
    case StoryState::Chill:
      // Enter buildup on energy swell OR clean kick + moderate energy
      if ((triggered && ratio > buildRatioGate) ||
          (m_spectralFlux > buildFluxGate &&
           ratio > (buildRatioGate + 0.02f))) {
        if (m_stateTimer > chillGate) {
          transitionTo(StoryState::Buildup);
        }
      }
      break;

    case StoryState::Buildup:
      m_buildupIntensity =
          std::min(m_buildupIntensity + deltaTime / 6.0f, 1.0f);
      // Drop triggers on a strong kick, heavy bass swell, or huge flux
      if ((triggered && ratio > dropRatioGate) || bassRatio > dropBassGate ||
          m_spectralFlux > dropFluxGate) {
        if (m_stateTimer > buildupMin) { // Build up needs at least some time
          transitionTo(StoryState::Drop);
        }
      } else if (ratio < 0.90f && m_stateTimer > 4.0f) {
        transitionTo(StoryState::Chill);
      }
      break;

    case StoryState::Drop: {
      const float dropMinHold = 8.0f; // extend hold to let transitions land
      if (m_stateTimer > dropMinHold) {
        transitionTo(ratio > 1.15f ? StoryState::Sustain : StoryState::Chill);
      }
      break;
    }

    case StoryState::Sustain:
      // Re-trigger drop on strong beats
      if (triggered && ratio > 1.05f && m_stateTimer > 3.0f) {
        transitionTo(StoryState::Drop);
      } else if (ratio < 0.95f && m_stateTimer > 5.0f) {
        transitionTo(StoryState::Chill);
      } else if (m_stateTimer > sustainMax) {
        transitionTo(StoryState::Chill);
      }
      break;
    }
  }

  // --- Verbose Telemetry ---
  if (m_settings.debugMode) {
    static uint32_t frameCounter = 0;
    if (frameCounter++ % 10 == 0) {
      const char *s = (m_state == StoryState::Chill)     ? "CHILL"
                      : (m_state == StoryState::Buildup) ? "BUILD"
                      : (m_state == StoryState::Drop)    ? "DROP"
                                                         : "SUSTAIN";
      fprintf(stderr,
              "[Story] %-7s t=%.2f | bb:%4.2f ema:%4.2f | ratio:%4.2f (gate bld %.2f drop %.2f) | bass:%4.2f (gate %.2f) | flux:%4.2f (bld %.2f drop %.2f) | trig:%s | sens:%3.1f\n",
              s, m_stateTimer, beatBass, g_beatDetector.ema, ratio,
              buildRatioGate, dropRatioGate, bassRatio, dropBassGate,
              m_spectralFlux, buildFluxGate, dropFluxGate,
              triggered ? "YES" : " no", m_currentBeatSensitivity);
    }
  }
}

void Storyteller::transitionTo(StoryState newState) {
  if (!m_pm || !m_presets)
    return;

  StoryState previousState = m_state;
  m_state = newState;
  m_stateTimer = 0.0f;

  switch (newState) {
  case StoryState::Chill:
    fprintf(stderr, "[Storyteller] → CHILL (last ratio=%.2f)\n",
            m_lastEnergyRatio);
    projectm_set_preset_locked(m_pm, false);

    // Gentle sensitivity wind-down (don't snap to 1.0 — let the organic decay
    // handle it)
    if (previousState == StoryState::Drop ||
        previousState == StoryState::Sustain) {
      // Coming from high energy: use a long, dreamy crossfade
      m_currentBeatSensitivity = 1.5f * m_settings.beatReactivity;
      projectm_set_soft_cut_duration(m_pm, 5.0);
    } else {
      // Normal chill entry: restore default
      m_currentBeatSensitivity = 1.0f * m_settings.beatReactivity;
      projectm_set_soft_cut_duration(m_pm, m_baseSoftCut);
    }
    m_currentBeatSensitivity = std::clamp(m_currentBeatSensitivity, m_settings.minSensitivity, m_settings.maxSensitivity);
    projectm_set_beat_sensitivity(m_pm, m_currentBeatSensitivity);
    m_buildupIntensity = 0.0f;
    m_cooldown = 2.0f;
    break;

  case StoryState::Buildup:
    fprintf(stderr, "[Storyteller] → BUILDUP\n");
    projectm_set_preset_locked(m_pm, m_settings.buildupLockPreset);

    // Slightly lift sensitivity to make visuals feel more alive during buildup
    m_currentBeatSensitivity = 1.3f * m_settings.beatReactivity;
    projectm_set_beat_sensitivity(m_pm, std::clamp(m_currentBeatSensitivity, m_settings.minSensitivity, m_settings.maxSensitivity));

    // Use a medium-long crossfade so any auto-advance during buildup feels
    // smooth
    projectm_set_soft_cut_duration(m_pm, 4.0);

    m_buildupIntensity = 0.0f;
    m_cooldown = 0.8f;
    break;

  case StoryState::Drop: {
    // Determine transition style
    int style;
    if (m_settings.transitionStyle == 0) {
      // Cycling: guaranteed variety (now 6 styles)
      style = m_dropCount % 6;
      m_dropCount++;
    } else {
      style = std::clamp(m_settings.transitionStyle - 1, 0, 5);
    }

    fprintf(stderr, "[Storyteller] → DROP!!! (style %d, buildup=%.2f)\n", style,
            m_buildupIntensity);

    // Scale transition speed by buildup intensity (higher buildup = faster)
    float intensityScale = std::clamp((1.0f - m_buildupIntensity) * 0.8f, 0.0f, 0.8f);

    switch (style) {
    case 0: // Flash Cut - instant or near-instant
      projectm_set_soft_cut_duration(m_pm, 0.25f);
      projectm_set_beat_sensitivity(m_pm, std::min(2.8f * m_settings.beatReactivity, m_settings.maxSensitivity));
      break;
    case 1: // Slow Morph - more relaxed visual handoff
      projectm_set_soft_cut_duration(m_pm, 2.0f + 1.0f * intensityScale); // ~2.0-2.8s
      break;
    case 2: // Quick Blend - punchy but smooth
      projectm_set_soft_cut_duration(m_pm, 1.0f + 0.4f * intensityScale); // ~1.0-1.3s
      break;
    case 3: // Glitch Cut - very fast + sensitivity spike (feels broken in a cool way)
      projectm_set_soft_cut_duration(m_pm, 0.15f);
      projectm_set_beat_sensitivity(m_pm, std::min(2.6f * m_settings.beatReactivity, m_settings.maxSensitivity));
      break;
    case 4: // Zoom Burst - longer, more dramatic handoff
      projectm_set_soft_cut_duration(m_pm, 3.5f + 1.5f * intensityScale);
      projectm_set_beat_sensitivity(m_pm, std::min((2.2f + m_buildupIntensity) * m_settings.beatReactivity, m_settings.maxSensitivity));
      break;
    case 5: // Energy Flash - short but high energy emphasis
      projectm_set_soft_cut_duration(m_pm, 0.6f);
      projectm_set_beat_sensitivity(m_pm, std::min(2.5f * m_settings.beatReactivity, m_settings.maxSensitivity));
      break;
    default:
      projectm_set_soft_cut_duration(m_pm, 1.5f);
      break;
    }

    m_presets->next(false);
    projectm_set_preset_locked(m_pm, false);

    // Sensitivity scales with buildup intensity — earned drops hit harder
    m_currentBeatSensitivity = (2.5f + m_buildupIntensity * 1.5f) * m_settings.beatReactivity;
    projectm_set_beat_sensitivity(m_pm, std::clamp(m_currentBeatSensitivity, m_settings.minSensitivity, m_settings.maxSensitivity));

    m_cooldown = 3.0f;
    break;
  }

  case StoryState::Sustain:
    fprintf(stderr, "[Storyteller] → SUSTAIN (last ratio=%.2f)\n",
            m_lastEnergyRatio);
    projectm_set_preset_locked(m_pm, true);

    // Medium crossfade for any transition within sustain
    projectm_set_soft_cut_duration(m_pm, 2.5);

    // Hold elevated sensitivity but not as aggressive as the Drop peak
    m_currentBeatSensitivity = 2.0f * m_settings.beatReactivity;
    m_currentBeatSensitivity = std::clamp(m_currentBeatSensitivity, m_settings.minSensitivity, m_settings.maxSensitivity);
    projectm_set_beat_sensitivity(m_pm, m_currentBeatSensitivity);

    m_cooldown = 2.0f;
    break;
  }
}

void Storyteller::forceState(StoryState state) { transitionTo(state); }

void Storyteller::applySettings(const StorySettings& settings) {
  m_settings = settings;
}
