#include "VibeusVisualizer.h"
#include <SDL_opengl.h>
#include <cstdio>
#include <cmath>
#include <algorithm>
#include <vector>
#include <cstdlib>

// Simple FFT for spectrum (radix-2, power of 2 size)
static void simpleFFT(const float* input, float* output, int n) {
    std::vector<float> real(n), imag(n);
    for (int i = 0; i < n; ++i) {
        real[i] = input[i * 2];
        imag[i] = 0.0f;
    }
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
    m_particles.reserve(200);
    fprintf(stderr, "[VibeusVisualizer] Custom backend initialized (%dx%d) v0.3.0\n", width, height);
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

    float sumSq = 0.0f;
    for (uint32_t i = 0; i < numFrames * 2; i++) {
        float s = samples[i];
        sumSq += s * s;
    }
    m_audioLevel = sqrtf(sumSq / (numFrames * 2.0f));

    m_audioBuffer.assign(samples, samples + numFrames * 2);

    int fftSize = 512;
    if (m_audioBuffer.size() >= fftSize * 2) {
        std::vector<float> fftOut(fftSize / 2);
        simpleFFT(m_audioBuffer.data(), fftOut.data(), fftSize);
        for (int b = 0; b < 64; ++b) {
            float sum = 0.0f;
            int start = b * (fftSize / 2 / 64);
            int end = (b + 1) * (fftSize / 2 / 64);
            for (int k = start; k < end; ++k) sum += fftOut[k];
            m_spectrum[b] = sum / (end - start) * 5.0f;
            m_spectrum[b] = std::min(1.0f, m_spectrum[b]);
        }
    }

    static float lastLevel = 0.0f;
    if (m_audioLevel > lastLevel * 1.4f && m_audioLevel > 0.25f) {
        int spawnCount = 5 + static_cast<int>(m_audioLevel * 15);
        for (int i = 0; i < spawnCount && m_particles.size() < 180; ++i) {
            Particle p;
            p.x = (rand() % 2000 - 1000) / 1000.0f;
            p.y = (rand() % 2000 - 1000) / 1000.0f;
            p.vx = (rand() % 1000 - 500) / 800.0f;
            p.vy = (rand() % 1000 - 500) / 800.0f;
            p.life = 0.8f + (rand() % 100) / 200.0f;
            p.size = 0.015f + m_audioLevel * 0.04f;
            p.hue = static_cast<float>(rand() % 360) / 360.0f;
            m_particles.push_back(p);
        }
    }
    lastLevel = m_audioLevel;
}

void VibeusVisualizer::renderFrame() {
    float intensity = std::min(1.0f, m_audioLevel * 2.5f * m_beatSensitivity);
    
    // Dynamic background - subtle pulse
    glClearColor(0.03f + intensity * 0.04f, 0.05f + intensity * 0.06f, 0.12f + intensity * 0.08f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Central pulsing core (bass reactive orb)
    float coreSize = 0.08f + intensity * 0.12f;
    float coreAlpha = 0.6f + intensity * 0.4f;
    glColor4f(0.9f, 0.4f + intensity * 0.3f, 1.0f - intensity * 0.2f, coreAlpha);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(0.0f, 0.0f);
    for (int i = 0; i <= 32; ++i) {
        float angle = (i / 32.0f) * 2.0f * 3.14159265f;
        glVertex2f(cosf(angle) * coreSize, sinf(angle) * coreSize);
    }
    glEnd();

    // Inner bright core
    glColor4f(1.0f, 0.9f, 0.7f, 0.9f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(0.0f, 0.0f);
    for (int i = 0; i <= 32; ++i) {
        float angle = (i / 32.0f) * 2.0f * 3.14159265f;
        glVertex2f(cosf(angle) * coreSize * 0.4f, sinf(angle) * coreSize * 0.4f);
    }
    glEnd();

    // Spectrum bars with frequency-based colors (red=low, green=mid, blue=high)
    float barWidth = 1.85f / 64.0f;
    glBegin(GL_QUADS);
    for (int b = 0; b < 64; ++b) {
        float h = m_spectrum[b] * 0.85f;
        float x = -0.92f + b * barWidth;
        
        // Color gradient: low freq red, mid green, high blue/purple
        float hue = (b / 64.0f) * 0.85f; // 0=red to ~0.85=purple
        float r = std::max(0.2f, sinf(hue * 3.14159f * 2.0f) * 0.8f + 0.3f);
        float g = std::max(0.2f, sinf((hue + 0.33f) * 3.14159f * 2.0f) * 0.8f + 0.3f);
        float bl = std::max(0.2f, sinf((hue + 0.66f) * 3.14159f * 2.0f) * 0.8f + 0.3f);
        
        glColor4f(r, g, bl, 0.85f);
        glVertex2f(x, -0.92f);
        glVertex2f(x + barWidth * 0.85f, -0.92f);
        glVertex2f(x + barWidth * 0.85f, -0.92f + h);
        glVertex2f(x, -0.92f + h);
    }
    glEnd();

    // Neon waveform with thickness simulation
    if (!m_audioBuffer.empty()) {
        glLineWidth(2.5f);
        glColor4f(0.3f, 1.0f, 0.6f, 0.75f);
        glBegin(GL_LINE_STRIP);
        size_t skip = std::max<size_t>(1, m_audioBuffer.size() / 300);
        for (size_t i = 0; i < m_audioBuffer.size(); i += skip * 2) {
            float x = -0.96f + (1.92f * i) / m_audioBuffer.size();
            float y = m_audioBuffer[i] * 0.55f;
            glVertex2f(x, y);
        }
        glEnd();
        glLineWidth(1.0f);
    }

    // Update & draw particles with network connections
    std::vector<Particle> aliveParticles;
    glBegin(GL_LINES);
    glColor4f(0.6f, 0.8f, 1.0f, 0.25f);
    for (size_t i = 0; i < m_particles.size(); ++i) {
        for (size_t j = i + 1; j < m_particles.size(); ++j) {
            float dx = m_particles[i].x - m_particles[j].x;
            float dy = m_particles[i].y - m_particles[j].y;
            float dist = sqrtf(dx*dx + dy*dy);
            if (dist < 0.35f && dist > 0.01f) {
                glVertex2f(m_particles[i].x, m_particles[i].y);
                glVertex2f(m_particles[j].x, m_particles[j].y);
            }
        }
    }
    glEnd();

    glPointSize(3.5f);
    glBegin(GL_POINTS);
    for (auto& p : m_particles) {
        p.x += p.vx * 0.018f;
        p.y += p.vy * 0.018f;
        p.vx *= 0.985f;
        p.vy *= 0.985f;
        p.life -= 0.028f;
        
        // Gentle attraction to center
        float distToCenter = sqrtf(p.x*p.x + p.y*p.y);
        if (distToCenter > 0.05f) {
            p.vx -= p.x * 0.008f;
            p.vy -= p.y * 0.008f;
        }
        
        if (p.life > 0.0f) {
            float alpha = p.life * 0.85f;
            float r = 0.7f + sinf(p.hue * 6.28f) * 0.3f;
            float g = 0.6f + cosf(p.hue * 6.28f) * 0.4f;
            float b = 0.9f - sinf(p.hue * 6.28f) * 0.2f;
            glColor4f(r, g, b, alpha);
            glVertex2f(p.x, p.y);
            aliveParticles.push_back(p);
        }
    }
    glEnd();
    m_particles = std::move(aliveParticles);

    // Preset indicator - glowing pill
    if (!m_presets.empty()) {
        float hue = static_cast<float>(m_currentPreset) / std::max(1u, static_cast<uint32_t>(m_presets.size()));
        float pr = 0.6f + sinf(hue * 6.28f) * 0.4f;
        float pg = 0.5f + cosf(hue * 6.28f) * 0.5f;
        float pb = 0.8f;
        glColor4f(pr, pg, pb, 0.55f);
        glBegin(GL_QUADS);
        glVertex2f(-0.96f, 0.86f);
        glVertex2f(-0.68f, 0.86f);
        glVertex2f(-0.68f, 0.96f);
        glVertex2f(-0.96f, 0.96f);
        glEnd();
        
        // Inner highlight
        glColor4f(1.0f, 1.0f, 1.0f, 0.35f);
        glBegin(GL_QUADS);
        glVertex2f(-0.94f, 0.88f);
        glVertex2f(-0.70f, 0.88f);
        glVertex2f(-0.70f, 0.94f);
        glVertex2f(-0.94f, 0.94f);
        glEnd();
    }
}

uint32_t VibeusVisualizer::loadPresetsFromDirectory(const std::string& path) {
    uint32_t before = static_cast<uint32_t>(m_presets.size());
    for (int i = 0; i < 25; i++) {
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
std::string VibeusVisualizer::getBackendInfo() const { return "Vibeus Custom Backend v0.3.0 (FFT + Particles + Network + Core)"; }
void VibeusVisualizer::setLogLevel(int level) { m_logLevel = level; }
