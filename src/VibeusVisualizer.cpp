#include "VibeusVisualizer.h"
#include <SDL_opengl.h>
#include <cstdio>
#include <cmath>
#include <algorithm>
#include <vector>

// Simple FFT for spectrum (radix-2, power of 2 size)
static void simpleFFT(const float* input, float* output, int n) {
    // Bit-reversal and butterfly for demo (assumes n power of 2, e.g. 512)
    std::vector<float> real(n), imag(n);
    for (int i = 0; i < n; ++i) {
        real[i] = input[i * 2]; // left channel for simplicity
        imag[i] = 0.0f;
    }
    // Bit reversal
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        for (; j >= bit; bit >>= 1) j -= bit;
        j += bit;
        if (i < j) {
            std::swap(real[i], real[j]);
            std::swap(imag[i], imag[j]);
        }
    }
    for (int len = 2; len <= n; len <<= 1) {
        float ang = 2 * 3.14159265f / len;
        float wreal = cosf(ang), wimag = -sinf(ang);
        for (int i = 0; i < n; i += len) {
            float wr = 1.0f, wi = 0.0f;
            for (int j = 0; j < len / 2; ++j) {
                int u = i + j, v = i + j + len / 2;
                float tr = wr * real[v] - wi * imag[v];
                float ti = wr * imag[v] + wi * real[v];
                real[v] = real[u] - tr;
                imag[v] = imag[u] - ti;
                real[u] += tr;
                imag[u] += ti;
                float temp = wr;
                wr = wr * wreal - wi * wimag;
                wi = temp * wimag + wi * wreal;
            }
        }
    }
    for (int i = 0; i < n / 2; ++i) {
        output[i] = sqrtf(real[i] * real[i] + imag[i] * imag[i]) / n;
    }
}

VibeusVisualizer::~VibeusVisualizer() {
    shutdown();
}

bool VibeusVisualizer::initialize(int width, int height) {
    m_width = width;
    m_height = height;
    m_audioBuffer.reserve(4096);
    m_spectrum.resize(64);
    fprintf(stderr, "[VibeusVisualizer] Custom backend initialized (%dx%d) with FFT + particles\n", width, height);
    return true;
}

void VibeusVisualizer::shutdown() {
    m_presets.clear();
    m_audioBuffer.clear();
    m_spectrum.clear();
    m_particles.clear();
}

void VibeusVisualizer::setWindowSize(int width, int height) {
    m_width = width;
    m_height = height;
}

void VibeusVisualizer::setFrameTime(double seconds) {
    m_frameTime = seconds;
}

void VibeusVisualizer::addAudioPCM(const float* samples, uint32_t numFrames) {
    if (!samples || numFrames == 0) return;

    // Calculate audio level (RMS)
    float sumSq = 0.0f;
    for (uint32_t i = 0; i < numFrames * 2; i++) {
        float s = samples[i];
        sumSq += s * s;
    }
    m_audioLevel = sqrtf(sumSq / (numFrames * 2.0f));

    // Store audio buffer
    m_audioBuffer.assign(samples, samples + numFrames * 2);

    // Compute spectrum (64 bands)
    int fftSize = 512;
    if (m_audioBuffer.size() >= fftSize * 2) {
        std::vector<float> fftOut(fftSize / 2);
        simpleFFT(m_audioBuffer.data(), fftOut.data(), fftSize);
        // Downsample to 64 bands with log scaling
        for (int b = 0; b < 64; ++b) {
            float sum = 0.0f;
            int start = b * (fftSize / 2 / 64);
            int end = (b + 1) * (fftSize / 2 / 64);
            for (int k = start; k < end; ++k) sum += fftOut[k];
            m_spectrum[b] = sum / (end - start) * 4.0f; // boost
            m_spectrum[b] = std::min(1.0f, m_spectrum[b]);
        }
    }

    // Spawn particles on beat
    static float lastLevel = 0.0f;
    if (m_audioLevel > lastLevel * 1.3f && m_audioLevel > 0.3f) {
        for (int i = 0; i < 8; ++i) {
            Particle p;
            p.x = (rand() % 1000) / 1000.0f * 2.0f - 1.0f;
            p.y = (rand() % 1000) / 1000.0f * 2.0f - 1.0f;
            p.vx = (rand() % 1000 - 500) / 2000.0f;
            p.vy = (rand() % 1000 - 500) / 2000.0f;
            p.life = 1.0f;
            p.size = 0.02f + m_audioLevel * 0.03f;
            m_particles.push_back(p);
        }
    }
    lastLevel = m_audioLevel;
}

void VibeusVisualizer::renderFrame() {
    // Modern-ish look with immediate mode for compatibility
    float intensity = std::min(1.0f, m_audioLevel * 2.0f * m_beatSensitivity);
    glClearColor(0.05f * intensity, 0.1f * intensity, 0.2f + 0.1f * intensity, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Spectrum bars (64 bars)
    glColor4f(0.2f, 0.8f, 1.0f, 0.9f);
    glBegin(GL_QUADS);
    float barWidth = 1.8f / 64.0f;
    for (int b = 0; b < 64; ++b) {
        float h = m_spectrum[b] * 0.8f;
        float x = -0.9f + b * barWidth;
        glVertex2f(x, -0.9f);
        glVertex2f(x + barWidth * 0.8f, -0.9f);
        glVertex2f(x + barWidth * 0.8f, -0.9f + h);
        glVertex2f(x, -0.9f + h);
    }
    glEnd();

    // Waveform
    if (!m_audioBuffer.empty()) {
        glColor4f(1.0f, 1.0f, 0.5f, 0.7f);
        glBegin(GL_LINE_STRIP);
        size_t skip = std::max<size_t>(1, m_audioBuffer.size() / 256);
        for (size_t i = 0; i < m_audioBuffer.size(); i += skip * 2) {
            float x = -0.95f + (1.9f * i) / m_audioBuffer.size();
            float y = m_audioBuffer[i] * 0.6f;
            glVertex2f(x, y);
        }
        glEnd();
    }

    // Update and draw particles
    glColor4f(1.0f, 0.9f, 0.3f, 0.8f);
    glBegin(GL_POINTS);
    for (auto it = m_particles.begin(); it != m_particles.end(); ) {
        it->x += it->vx * 0.02f;
        it->y += it->vy * 0.02f;
        it->life -= 0.03f;
        it->vx *= 0.98f;
        it->vy *= 0.98f;
        if (it->life <= 0.0f) {
            it = m_particles.erase(it);
            continue;
        }
        glVertex2f(it->x, it->y);
        ++it;
    }
    glEnd();

    // Preset indicator
    if (!m_presets.empty()) {
        float hue = static_cast<float>(m_currentPreset) / std::max(1u, static_cast<uint32_t>(m_presets.size()));
        glColor4f(hue, 1.0f - hue, 0.5f, 0.6f);
        glBegin(GL_QUADS);
        glVertex2f(-0.95f, 0.85f);
        glVertex2f(-0.7f, 0.85f);
        glVertex2f(-0.7f, 0.95f);
        glVertex2f(-0.95f, 0.95f);
        glEnd();
    }
}

uint32_t VibeusVisualizer::loadPresetsFromDirectory(const std::string& path) {
    uint32_t before = static_cast<uint32_t>(m_presets.size());
    for (int i = 0; i < 20; i++) {
        m_presets.push_back(path + "/VibeusPreset" + std::to_string(i + 1) + ".vbs");
    }
    uint32_t after = static_cast<uint32_t>(m_presets.size());
    fprintf(stderr, "[VibeusVisualizer] Loaded %u placeholder presets from %s\n", after - before, path.c_str());
    return after - before;
}

uint32_t VibeusVisualizer::getPresetCount() const {
    return static_cast<uint32_t>(m_presets.size());
}

void VibeusVisualizer::setPreset(uint32_t index, bool hardCut) {
    if (index < m_presets.size()) {
        m_currentPreset = index;
        if (m_logLevel >= 3) {
            fprintf(stderr, "[VibeusVisualizer] Switched to preset %u (%s)\n", index, hardCut ? "hard cut" : "soft cut");
        }
    }
}

uint32_t VibeusVisualizer::getCurrentPresetIndex() const {
    return m_currentPreset;
}

std::string VibeusVisualizer::getPresetName(uint32_t index) const {
    if (index < m_presets.size()) return m_presets[index];
    return "";
}

void VibeusVisualizer::removePreset(uint32_t index) {
    if (index < m_presets.size()) {
        m_presets.erase(m_presets.begin() + index);
        if (m_currentPreset >= m_presets.size() && !m_presets.empty())
            m_currentPreset = static_cast<uint32_t>(m_presets.size() - 1);
    }
}

void VibeusVisualizer::setBeatSensitivity(float sensitivity) { m_beatSensitivity = sensitivity; }
float VibeusVisualizer::getBeatSensitivity() const { return m_beatSensitivity; }
void VibeusVisualizer::setPresetDuration(double seconds) { m_presetDuration = seconds; }
void VibeusVisualizer::setSoftCutDuration(double seconds) { m_softCutDuration = seconds; }
double VibeusVisualizer::getSoftCutDuration() const { return m_softCutDuration; }
void VibeusVisualizer::setHardCutEnabled(bool enabled) { m_hardCutEnabled = enabled; }
void VibeusVisualizer::setHardCutSensitivity(float sensitivity) { m_hardCutSensitivity = sensitivity; }
void VibeusVisualizer::setHardCutDuration(double seconds) { m_hardCutDuration = seconds; }
void VibeusVisualizer::setPresetLocked(bool locked) { m_presetLocked = locked; }
void VibeusVisualizer::setMeshSize(int width, int height) { m_meshWidth = width; m_meshHeight = height; }
void VibeusVisualizer::setAspectCorrection(bool enabled) { m_aspectCorrection = enabled; }
void VibeusVisualizer::setEasterEgg(float value) { m_easterEgg = value; }
void VibeusVisualizer::setTextureSearchPaths(const char** paths, size_t count) {
    if (m_logLevel >= 3) fprintf(stderr, "[VibeusVisualizer] Set %zu texture paths\n", count);
}
void VibeusVisualizer::touch(float x, float y, int pressure, int type) {
    if (m_logLevel >= 4) fprintf(stderr, "[VibeusVisualizer] Touch (%.2f, %.2f) pressure=%d\n", x, y, pressure);
}
void VibeusVisualizer::touchDrag(float x, float y, int pressure) {
    if (m_logLevel >= 5) fprintf(stderr, "[VibeusVisualizer] TouchDrag (%.2f, %.2f)\n", x, y);
}
std::string VibeusVisualizer::getBackendInfo() const { return "Vibeus Custom Backend v0.2.0 (FFT + Particles)"; }
void VibeusVisualizer::setLogLevel(int level) { m_logLevel = level; }
