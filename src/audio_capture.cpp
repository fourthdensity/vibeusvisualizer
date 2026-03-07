#include "audio_capture.h"

#ifdef WASAPI_LOOPBACK

#include <functiondiscoverykeys_devpkey.h>
#include <projectM-4/audio.h>
#include <cstdio>
#include <cstring>
#include <algorithm>
#include <vector>

AudioCapture::~AudioCapture()
{
    shutdown();
}

bool AudioCapture::init()
{
    HRESULT hr;

    hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (SUCCEEDED(hr)) {
        m_comInit = true;
    } else if (hr == RPC_E_CHANGED_MODE) {
        // COM already initialized with different threading model - that's OK
        m_comInit = false;
    } else {
        fprintf(stderr, "[AudioCapture] CoInitializeEx failed: 0x%08lx\n", hr);
        return false;
    }

    // Get default audio render device (speakers/headphones)
    IMMDeviceEnumerator* enumerator = nullptr;
    hr = CoCreateInstance(
        __uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
        __uuidof(IMMDeviceEnumerator), reinterpret_cast<void**>(&enumerator));
    if (FAILED(hr)) {
        fprintf(stderr, "[AudioCapture] Failed to create device enumerator: 0x%08lx\n", hr);
        return false;
    }

    hr = enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &m_device);
    enumerator->Release();
    if (FAILED(hr)) {
        fprintf(stderr, "[AudioCapture] Failed to get default audio endpoint: 0x%08lx\n", hr);
        return false;
    }

    // Activate audio client
    hr = m_device->Activate(
        __uuidof(IAudioClient), CLSCTX_ALL, nullptr,
        reinterpret_cast<void**>(&m_audioClient));
    if (FAILED(hr)) {
        fprintf(stderr, "[AudioCapture] Failed to activate audio client: 0x%08lx\n", hr);
        return false;
    }

    // Get mix format
    WAVEFORMATEX* mixFormat = nullptr;
    hr = m_audioClient->GetMixFormat(&mixFormat);
    if (FAILED(hr)) {
        fprintf(stderr, "[AudioCapture] Failed to get mix format: 0x%08lx\n", hr);
        return false;
    }

    m_blockAlign = mixFormat->nBlockAlign;

    // Initialize in loopback mode
    hr = m_audioClient->Initialize(
        AUDCLNT_SHAREMODE_SHARED,
        AUDCLNT_STREAMFLAGS_LOOPBACK,
        0, 0, mixFormat, nullptr);
    CoTaskMemFree(mixFormat);
    if (FAILED(hr)) {
        fprintf(stderr, "[AudioCapture] Failed to initialize loopback: 0x%08lx\n", hr);
        return false;
    }

    // Get capture client
    hr = m_audioClient->GetService(
        __uuidof(IAudioCaptureClient),
        reinterpret_cast<void**>(&m_captureClient));
    if (FAILED(hr)) {
        fprintf(stderr, "[AudioCapture] Failed to get capture client: 0x%08lx\n", hr);
        return false;
    }

    // Start capturing
    hr = m_audioClient->Start();
    if (FAILED(hr)) {
        fprintf(stderr, "[AudioCapture] Failed to start capture: 0x%08lx\n", hr);
        return false;
    }

    m_initialized = true;
    fprintf(stderr, "[AudioCapture] WASAPI loopback initialized successfully\n");
    return true;
}

void AudioCapture::feedAudio(projectm_handle pm, float gain)
{
    if (!m_initialized || !pm)
        return;

    HRESULT hr;
    UINT32 packetSize;

    for (hr = m_captureClient->GetNextPacketSize(&packetSize);
         SUCCEEDED(hr) && packetSize > 0;
         hr = m_captureClient->GetNextPacketSize(&packetSize))
    {
        BYTE*  data = nullptr;
        UINT32 numFrames = 0;
        DWORD  flags = 0;

        hr = m_captureClient->GetBuffer(&data, &numFrames, &flags, nullptr, nullptr);
        if (FAILED(hr))
            break;

        if (!(flags & AUDCLNT_BUFFERFLAGS_SILENT) && data) {
            const float* src = reinterpret_cast<const float*>(data);
            const UINT32 totalSamples = numFrames * 2; // stereo

            if (gain == 1.0f) {
                // No scaling needed — pass through directly
                projectm_pcm_add_float(pm, src, numFrames, PROJECTM_STEREO);
            } else {
                // Apply gain scaling
                m_gainBuffer.resize(totalSamples);
                for (UINT32 i = 0; i < totalSamples; i++) {
                    m_gainBuffer[i] = src[i] * gain;
                }
                projectm_pcm_add_float(pm, m_gainBuffer.data(), numFrames, PROJECTM_STEREO);
            }
        }

        m_captureClient->ReleaseBuffer(numFrames);
    }
}

void AudioCapture::shutdown()
{
    if (m_audioClient) {
        m_audioClient->Stop();
    }
    if (m_captureClient) {
        m_captureClient->Release();
        m_captureClient = nullptr;
    }
    if (m_audioClient) {
        m_audioClient->Release();
        m_audioClient = nullptr;
    }
    if (m_device) {
        m_device->Release();
        m_device = nullptr;
    }
    if (m_comInit) {
        CoUninitialize();
        m_comInit = false;
    }
    m_initialized = false;
}

#endif // WASAPI_LOOPBACK
