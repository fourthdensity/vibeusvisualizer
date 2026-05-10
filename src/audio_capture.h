#pragma once

#ifdef WASAPI_LOOPBACK

#include <windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <vector>

#ifdef USE_PROJECTM_BACKEND
#include <projectM-4/types.h>
#endif

class IVisualizer;  // forward declaration for abstraction

/// WASAPI loopback audio capture - captures system audio output ("what you hear")
class AudioCapture {
public:
    AudioCapture() = default;
    ~AudioCapture();

    /// Initialize WASAPI loopback capture on the default render device.
    /// Returns true on success.
    bool init();

#ifdef USE_PROJECTM_BACKEND
    /// Drain any available audio packets and feed them to projectM (legacy).
    void feedAudio(projectm_handle pm, float gain = 1.0f);
#endif

    /// New: Feed to any IVisualizer backend (abstraction layer)
    void feedAudio(IVisualizer* viz, float gain = 1.0f);

    float levelRms()      const { return m_levelRms; }
    float levelPeak()     const { return m_levelPeak; }
    float levelBass()     const { return m_levelBass; }
    float levelBeatBass() const { return m_levelBeatBass; }
    
    float levelBeatLow()  const { return m_levelBeatLow; }
    float levelBeatMid()  const { return m_levelBeatMid; }
    float levelBeatHigh() const { return m_levelBeatHigh; }
    
    bool onsetLow()  { bool v = m_onsetLow;  m_onsetLow  = false; return v; }
    bool onsetMid()  { bool v = m_onsetMid;  m_onsetMid  = false; return v; }
    bool onsetHigh() { bool v = m_onsetHigh; m_onsetHigh = false; return v; }

    void shutdown();

    bool isInitialized() const { return m_initialized; }

private:
    IAudioCaptureClient* m_captureClient = nullptr;
    IAudioClient*        m_audioClient   = nullptr;
    IMMDevice*           m_device        = nullptr;

    UINT16               m_channels      = 2;
    UINT16               m_bitsPerSample = 32;
    UINT16               m_validBitsPerSample = 32;
    uint32_t             m_sampleRate    = 44100;
    bool                 m_isFloat       = true;

    UINT32               m_blockAlign    = 0;
    bool                 m_initialized   = false;
    bool                 m_comInit       = false;

    std::vector<float>   m_stereoBuffer;

    float m_levelRms      = 0.0f;
    float m_levelPeak     = 0.0f;
    float m_levelBass     = 0.0f;
    float m_levelBeatBass = 0.0f;
    
    float m_levelBeatLow  = 0.0f;
    float m_levelBeatMid  = 0.0f;
    float m_levelBeatHigh = 0.0f;
    
    bool m_onsetLow  = false;
    bool m_onsetMid  = false;
    bool m_onsetHigh = false;
    float m_fluxLow  = 0.0f;
    float m_fluxMid  = 0.0f;
    float m_fluxHigh = 0.0f;
    float m_avgShortLow  = 0.0f;
    float m_avgShortMid  = 0.0f;
    float m_avgShortHigh = 0.0f;
    float m_avgLongLow   = 0.0f;
    float m_avgLongMid   = 0.0f;
    float m_avgLongHigh  = 0.0f;
    int m_cooldownLow  = 0;
    int m_cooldownMid  = 0;
    int m_cooldownHigh = 0;

    float m_bassLpState = 0.0f;
    double m_bqLowX1 = 0.0, m_bqLowX2 = 0.0;
    double m_bqLowY1 = 0.0, m_bqLowY2 = 0.0;
    double m_bqLowA1 = 0.0, m_bqLowA2 = 0.0;
    double m_bqLowB0 = 1.0, m_bqLowB1 = 0.0, m_bqLowB2 = 0.0;
    
    double m_bqMidX1 = 0.0, m_bqMidX2 = 0.0;
    double m_bqMidY1 = 0.0, m_bqMidY2 = 0.0;
    double m_bqMidA1 = 0.0, m_bqMidA2 = 0.0;
    double m_bqMidB0 = 1.0, m_bqMidB1 = 0.0, m_bqMidB2 = 0.0;
    
    double m_bqHighX1 = 0.0, m_bqHighX2 = 0.0;
    double m_bqHighY1 = 0.0, m_bqHighY2 = 0.0;
    double m_bqHighA1 = 0.0, m_bqHighA2 = 0.0;
    double m_bqHighB0 = 1.0, m_bqHighB1 = 0.0, m_bqHighB2 = 0.0;

    void convertPacketToStereoFloat(const BYTE* data, UINT32 numFrames, DWORD flags, float gain);
    void accumulateMetering(UINT32 numFrames, double& sumSq, float& peak) const;
    void processMultiBandBeats(UINT32 numFrames);
    void processBiquadBand(double mono,
                           double& x1, double& x2, double& y1, double& y2,
                           double b0, double b1, double b2, double a1, double a2,
                           float& peak, float safetyCeil);
    void updateBeatLevels(float lowPeak, float midPeak, float highPeak);
    void checkOnsets();
    void decayLevels();
};

#else // !WASAPI_LOOPBACK

class IVisualizer;

class AudioCapture {
public:
    bool init() { return false; }
#ifdef USE_PROJECTM_BACKEND
    void feedAudio(projectm_handle, float = 1.0f) {}
#endif
    void feedAudio(IVisualizer*, float = 1.0f) {}
    float levelRms()  const { return 0.0f; }
    float levelPeak() const { return 0.0f; }
    float levelBass() const { return 0.0f; }
    void shutdown() {}
    bool isInitialized() const { return false; }
};

#endif // WASAPI_LOOPBACK
