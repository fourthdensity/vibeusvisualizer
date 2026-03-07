#pragma once

#ifdef WASAPI_LOOPBACK

#include <windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <projectM-4/types.h>
#include <vector>

/// WASAPI loopback audio capture - captures system audio output ("what you hear")
class AudioCapture {
public:
    AudioCapture() = default;
    ~AudioCapture();

    /// Initialize WASAPI loopback capture on the default render device.
    /// Returns true on success.
    bool init();

    /// Drain any available audio packets and feed them to projectM.
    /// gain: amplitude multiplier (1.0 = normal, 0.5 = half, 0 = silent)
    /// Call this once per frame.
    void feedAudio(projectm_handle pm, float gain = 1.0f);

    /// Shut down capture and release COM resources.
    void shutdown();

    bool isInitialized() const { return m_initialized; }

private:
    IAudioCaptureClient* m_captureClient = nullptr;
    IAudioClient*        m_audioClient   = nullptr;
    IMMDevice*           m_device        = nullptr;
    UINT32               m_blockAlign    = 0;
    bool                 m_initialized   = false;
    bool                 m_comInit       = false;
    std::vector<float>   m_gainBuffer;          // reused buffer for gain scaling
};

#else // !WASAPI_LOOPBACK

#include <projectM-4/types.h>

/// Stub for non-Windows platforms - uses SDL audio capture instead
class AudioCapture {
public:
    bool init() { return false; }
    void feedAudio(projectm_handle, float = 1.0f) {}
    void shutdown() {}
    bool isInitialized() const { return false; }
};

#endif // WASAPI_LOOPBACK
