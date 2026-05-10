#include "audio_capture.h"

#ifdef WASAPI_LOOPBACK

#include <functiondiscoverykeys_devpkey.h>
#include <projectM-4/audio.h>
#include <cstdio>
#include <cstring>
#include <algorithm>
#include <vector>
#include <cmath>

#include <mmreg.h>
#include <ksmedia.h>

#include "IVisualizer.h"

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
        m_comInit = false;
    } else {
        fprintf(stderr, "[AudioCapture] CoInitializeEx failed: 0x%08lx\n", hr);
        return false;
    }

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

    hr = m_device->Activate(
        __uuidof(IAudioClient), CLSCTX_ALL, nullptr,
        reinterpret_cast<void**>(&m_audioClient));
    if (FAILED(hr)) {
        fprintf(stderr, "[AudioCapture] Failed to activate audio client: 0x%08lx\n", hr);
        return false;
    }

    WAVEFORMATEX* mixFormat = nullptr;
    hr = m_audioClient->GetMixFormat(&mixFormat);
    if (FAILED(hr)) {
        fprintf(stderr, "[AudioCapture] Failed to get mix format: 0x%08lx\n", hr);
        return false;
    }

    m_channels = mixFormat->nChannels;
    m_bitsPerSample = mixFormat->wBitsPerSample;
    m_validBitsPerSample = mixFormat->wBitsPerSample;
    m_isFloat = (mixFormat->wFormatTag == WAVE_FORMAT_IEEE_FLOAT);

    if (mixFormat->wFormatTag == WAVE_FORMAT_EXTENSIBLE) {
        auto* ex = reinterpret_cast<WAVEFORMATEXTENSIBLE*>(mixFormat);
        m_channels = ex->Format.nChannels;
        m_bitsPerSample = ex->Format.wBitsPerSample;
        m_validBitsPerSample = ex->Samples.wValidBitsPerSample ? ex->Samples.wValidBitsPerSample : ex->Format.wBitsPerSample;
        if (ex->SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT)
            m_isFloat = true;
        else
            m_isFloat = false;
    }

    m_blockAlign = mixFormat->nBlockAlign;
    m_sampleRate = mixFormat->nSamplesPerSec;

    fprintf(stderr, "[AudioCapture] Mix format: %u ch, %u/%u bits, %u Hz, %s\n",
            static_cast<unsigned>(m_channels),
            static_cast<unsigned>(m_validBitsPerSample),
            static_cast<unsigned>(m_bitsPerSample),
            static_cast<unsigned>(m_sampleRate),
            m_isFloat ? "float" : "pcm");

    {
        double fc = 150.0;
        double fs = static_cast<double>(m_sampleRate);
        if (fs > 0.0) {
            double omega = 2.0 * 3.14159265358979323846 * fc / fs;
            double sn = std::sin(omega);
            double cs = std::cos(omega);
            double alpha = sn / (2.0 * 0.70710678118654752440);

            double a0 = 1.0 + alpha;
            m_bqLowB0 = ((1.0 - cs) / 2.0) / a0;
            m_bqLowB1 = (1.0 - cs) / a0;
            m_bqLowB2 = ((1.0 - cs) / 2.0) / a0;
            m_bqLowA1 = (-2.0 * cs) / a0;
            m_bqLowA2 = (1.0 - alpha) / a0;
        }
    }
    
    {
        double fc = 1500.0;
        double fs = static_cast<double>(m_sampleRate);
        if (fs > 0.0) {
            double omega = 2.0 * 3.14159265358979323846 * fc / fs;
            double sn = std::sin(omega);
            double cs = std::cos(omega);
            double alpha = sn / (2.0 * 0.70710678118654752440);

            double a0 = 1.0 + alpha;
            m_bqMidB0 = alpha / a0;
            m_bqMidB1 = 0.0;
            m_bqMidB2 = -alpha / a0;
            m_bqMidA1 = (-2.0 * cs) / a0;
            m_bqMidA2 = (1.0 - alpha) / a0;
        }
    }
    
    {
        double fc = 8000.0;
        double fs = static_cast<double>(m_sampleRate);
        if (fs > 0.0) {
            double omega = 2.0 * 3.14159265358979323846 * fc / fs;
            double sn = std::sin(omega);
            double cs = std::cos(omega);
            double alpha = sn / (2.0 * 0.70710678118654752440);

            double a0 = 1.0 + alpha;
            m_bqHighB0 = alpha / a0;
            m_bqHighB1 = 0.0;
            m_bqHighB2 = -alpha / a0;
            m_bqHighA1 = (-2.0 * cs) / a0;
            m_bqHighA2 = (1.0 - alpha) / a0;
        }
    }

    REFERENCE_TIME hnsBuffer = 200000;
    hr = m_audioClient->Initialize(
        AUDCLNT_SHAREMODE_SHARED,
        AUDCLNT_STREAMFLAGS_LOOPBACK,
        hnsBuffer, 0, mixFormat, nullptr);

    CoTaskMemFree(mixFormat);

    if (FAILED(hr)) {
        fprintf(stderr, "[AudioCapture] Failed to initialize loopback: 0x%08lx\n", hr);
        return false;
    }

    hr = m_audioClient->GetService(
        __uuidof(IAudioCaptureClient),
        reinterpret_cast<void**>(&m_captureClient));
    if (FAILED(hr)) {
        fprintf(stderr, "[AudioCapture] Failed to get capture client: 0x%08lx\n", hr);
        return false;
    }

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

    double sumSq = 0.0;
    float peak = 0.0f;
    uint64_t sampleCount = 0;
    bool gotAny = false;

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

        m_stereoBuffer.resize(static_cast<size_t>(numFrames) * 2);

        convertPacketToStereoFloat(data, numFrames, flags, gain);
        accumulateMetering(numFrames, sumSq, peak);

        processMultiBandBeats(numFrames);

        sampleCount += numFrames;
        gotAny = true;

        projectm_pcm_add_float(pm, m_stereoBuffer.data(), numFrames, PROJECTM_STEREO);

        m_captureClient->ReleaseBuffer(numFrames);
    }

    if (gotAny && sampleCount > 0) {
        float rms = static_cast<float>(std::sqrt(sumSq / static_cast<double>(sampleCount)));
        float a = (rms > m_levelRms) ? 0.35f : 0.08f;
        m_levelRms = m_levelRms + (rms - m_levelRms) * a;
        m_levelPeak = (std::max)(m_levelPeak * 0.90f, peak);
    } else {
        decayLevels();
    }
}

void AudioCapture::feedAudio(IVisualizer* viz, float gain)
{
    if (!m_initialized || !viz)
        return;

    HRESULT hr;
    UINT32 packetSize;

    double sumSq = 0.0;
    float peak = 0.0f;
    uint64_t sampleCount = 0;
    bool gotAny = false;

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

        m_stereoBuffer.resize(static_cast<size_t>(numFrames) * 2);

        convertPacketToStereoFloat(data, numFrames, flags, gain);
        accumulateMetering(numFrames, sumSq, peak);

        processMultiBandBeats(numFrames);

        sampleCount += numFrames;
        gotAny = true;

        // Use the abstraction layer instead of direct projectM call
        viz->addAudioPCM(m_stereoBuffer.data(), numFrames);

        m_captureClient->ReleaseBuffer(numFrames);
    }

    if (gotAny && sampleCount > 0) {
        float rms = static_cast<float>(std::sqrt(sumSq / static_cast<double>(sampleCount)));
        float a = (rms > m_levelRms) ? 0.35f : 0.08f;
        m_levelRms = m_levelRms + (rms - m_levelRms) * a;
        m_levelPeak = (std::max)(m_levelPeak * 0.90f, peak);
    } else {
        decayLevels();
    }
}

void AudioCapture::convertPacketToStereoFloat(const BYTE* data, UINT32 numFrames, DWORD flags, float gain)
{
    const bool silent = (flags & AUDCLNT_BUFFERFLAGS_SILENT) || !data;

    if (silent) {
        std::fill(m_stereoBuffer.begin(), m_stereoBuffer.end(), 0.0f);
        return;
    }

    if (m_isFloat) {
        const float* src = reinterpret_cast<const float*>(data);
        const UINT32 ch = (m_channels > 0) ? m_channels : 2;
        for (UINT32 f = 0; f < numFrames; f++) {
            const UINT32 base = f * ch;
            float L = src[base + 0];
            float R = src[base + (ch > 1 ? 1 : 0)];
            m_stereoBuffer[f * 2 + 0] = L * gain;
            m_stereoBuffer[f * 2 + 1] = R * gain;
        }
    } else {
        const UINT32 ch = (m_channels > 0) ? m_channels : 2;
        const UINT16 bits = (m_bitsPerSample > 0) ? m_bitsPerSample : 16;
        const UINT16 validBits = (m_validBitsPerSample > 0) ? m_validBitsPerSample : bits;

        if (bits == 16) {
            const int16_t* src = reinterpret_cast<const int16_t*>(data);
            constexpr float denom = 32768.0f;
            for (UINT32 f = 0; f < numFrames; f++) {
                const UINT32 base = f * ch;
                float L = static_cast<float>(src[base + 0]) / denom;
                float R = static_cast<float>(src[base + (ch > 1 ? 1 : 0)]) / denom;
                m_stereoBuffer[f * 2 + 0] = L * gain;
                m_stereoBuffer[f * 2 + 1] = R * gain;
            }
        } else if (bits == 32) {
            const int32_t* src = reinterpret_cast<const int32_t*>(data);
            const int shift = (validBits < 32) ? (32 - validBits) : 0;
            const float denom = static_cast<float>(1u << (std::min<UINT16>(validBits, 32) - 1));
            for (UINT32 f = 0; f < numFrames; f++) {
                const UINT32 base = f * ch;
                int32_t rawL = src[base + 0];
                int32_t rawR = src[base + (ch > 1 ? 1 : 0)];
                if (shift > 0) { rawL >>= shift; rawR >>= shift; }
                float L = static_cast<float>(rawL) / denom;
                float R = static_cast<float>(rawR) / denom;
                m_stereoBuffer[f * 2 + 0] = L * gain;
                m_stereoBuffer[f * 2 + 1] = R * gain;
            }
        } else {
            std::fill(m_stereoBuffer.begin(), m_stereoBuffer.end(), 0.0f);
        }
    }
}

void AudioCapture::accumulateMetering(UINT32 numFrames, double& sumSq, float& peak) const
{
    for (UINT32 f = 0; f < numFrames; f++) {
        float L = m_stereoBuffer[f * 2 + 0];
        float R = m_stereoBuffer[f * 2 + 1];
        float mono = 0.5f * (L + R);
        sumSq += static_cast<double>(mono) * static_cast<double>(mono);
        peak = (std::max)(peak, (std::max)(std::fabs(L), std::fabs(R)));
    }
}

void AudioCapture::processBiquadBand(double mono,
                                     double& x1, double& x2, double& y1, double& y2,
                                     double b0, double b1, double b2, double a1, double a2,
                                     float& peak, float safetyCeil)
{
    double out = b0 * mono + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;

    if (!std::isfinite(out) || std::fabs(out) > 1000.0) {
        out = 0.0;
        x1 = x2 = y1 = y2 = 0.0;
    } else if (std::fabs(out) < 1e-15) {
        out = 0.0;
    }

    x2 = x1; x1 = mono;
    y2 = y1; y1 = out;

    float absOut = static_cast<float>(std::fabs(out));
    if (absOut > safetyCeil) absOut = 0.0f;
    if (absOut > peak) peak = absOut;
}

void AudioCapture::processMultiBandBeats(UINT32 numFrames)
{
    float bassAccum = 0.0f;
    float beatLowPeak = 0.0f;
    float beatMidPeak = 0.0f;
    float beatHighPeak = 0.0f;

    constexpr float bassAlpha = 0.028f;
    for (UINT32 i = 0; i < numFrames; i++) {
        float mono = 0.5f * (m_stereoBuffer[i * 2] + m_stereoBuffer[i * 2 + 1]);

        m_bassLpState += bassAlpha * (mono - m_bassLpState);
        bassAccum += std::fabs(m_bassLpState);

        double dMono = static_cast<double>(mono);

        processBiquadBand(dMono, m_bqLowX1, m_bqLowX2, m_bqLowY1, m_bqLowY2,
                          m_bqLowB0, m_bqLowB1, m_bqLowB2, m_bqLowA1, m_bqLowA2,
                          beatLowPeak, 10.0f);
        processBiquadBand(dMono, m_bqMidX1, m_bqMidX2, m_bqMidY1, m_bqMidY2,
                          m_bqMidB0, m_bqMidB1, m_bqMidB2, m_bqMidA1, m_bqMidA2,
                          beatMidPeak, 10.0f);
        processBiquadBand(dMono, m_bqHighX1, m_bqHighX2, m_bqHighY1, m_bqHighY2,
                          m_bqHighB0, m_bqHighB1, m_bqHighB2, m_bqHighA1, m_bqHighA2,
                          beatHighPeak, 10.0f);
    }

    if (numFrames > 0) {
        float bassLevel = bassAccum / static_cast<float>(numFrames);
        float ba = (bassLevel > m_levelBass) ? 0.4f : 0.1f;
        m_levelBass = m_levelBass + (bassLevel - m_levelBass) * ba;
    }

    updateBeatLevels(beatLowPeak, beatMidPeak, beatHighPeak);
    checkOnsets();
}

void AudioCapture::updateBeatLevels(float lowPeak, float midPeak, float highPeak)
{
    {
        constexpr float safetyCeil = 6.0f;
        float level = (lowPeak > safetyCeil) ? safetyCeil : lowPeak;
        float alpha = (level > m_levelBeatLow) ? 0.7f : 0.2f;
        m_levelBeatLow = m_levelBeatLow + (level - m_levelBeatLow) * alpha;
        m_levelBeatBass = m_levelBeatLow;
    }
    {
        constexpr float safetyCeil = 4.0f;
        float level = (midPeak > safetyCeil) ? safetyCeil : midPeak;
        float alpha = (level > m_levelBeatMid) ? 0.5f : 0.3f;
        m_levelBeatMid = m_levelBeatMid + (level - m_levelBeatMid) * alpha;
    }
    {
        constexpr float safetyCeil = 3.0f;
        float level = (highPeak > safetyCeil) ? safetyCeil : highPeak;
        float alpha = (level > m_levelBeatHigh) ? 0.8f : 0.4f;
        m_levelBeatHigh = m_levelBeatHigh + (level - m_levelBeatHigh) * alpha;
    }
}

void AudioCapture::checkOnsets()
{
    if (m_cooldownLow > 0) m_cooldownLow--;
    if (m_cooldownMid > 0) m_cooldownMid--;
    if (m_cooldownHigh > 0) m_cooldownHigh--;

    {
        float flux = (std::max)(0.0f, m_levelBeatLow - m_avgShortLow);
        m_fluxLow = m_fluxLow * 0.8f + flux * 0.2f;
        m_avgShortLow = m_avgShortLow * 0.92f + m_levelBeatLow * 0.08f;
        m_avgLongLow  = m_avgLongLow  * 0.98f + m_levelBeatLow * 0.02f;
        float threshold = (std::max)(m_avgLongLow * 1.5f + 0.015f, 0.02f);
        if (m_cooldownLow == 0 && m_fluxLow > threshold && m_levelBeatLow > 0.05f) {
            m_onsetLow = true;
            m_cooldownLow = 8;
        }
    }
    {
        float flux = (std::max)(0.0f, m_levelBeatMid - m_avgShortMid);
        m_fluxMid = m_fluxMid * 0.8f + flux * 0.2f;
        m_avgShortMid = m_avgShortMid * 0.92f + m_levelBeatMid * 0.08f;
        m_avgLongMid  = m_avgLongMid  * 0.98f + m_levelBeatMid * 0.02f;
        float threshold = (std::max)(m_avgLongMid * 1.6f + 0.012f, 0.015f);
        if (m_cooldownMid == 0 && m_fluxMid > threshold && m_levelBeatMid > 0.03f) {
            m_onsetMid = true;
            m_cooldownMid = 7;
        }
    }
    {
        float flux = (std::max)(0.0f, m_levelBeatHigh - m_avgShortHigh);
        m_fluxHigh = m_fluxHigh * 0.75f + flux * 0.25f;
        m_avgShortHigh = m_avgShortHigh * 0.90f + m_levelBeatHigh * 0.10f;
        m_avgLongHigh  = m_avgLongHigh  * 0.98f + m_levelBeatHigh * 0.02f;
        float threshold = (std::max)(m_avgLongHigh * 1.7f + 0.010f, 0.012f);
        if (m_cooldownHigh == 0 && m_fluxHigh > threshold && m_levelBeatHigh > 0.025f) {
            m_onsetHigh = true;
            m_cooldownHigh = 5;
        }
    }
}

void AudioCapture::decayLevels()
{
    m_levelRms *= 0.95f;
    m_levelPeak *= 0.90f;
    m_levelBass *= 0.92f;
    m_levelBeatBass *= 0.90f;
    m_levelBeatLow *= 0.90f;
    m_levelBeatMid *= 0.88f;
    m_levelBeatHigh *= 0.85f;
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
