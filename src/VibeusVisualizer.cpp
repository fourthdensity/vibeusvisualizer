#include "VibeusVisualizer.h"
#include <SDL_opengl.h>
#include <cstdio>
#include <cmath>
#include <algorithm>

VibeusVisualizer::~VibeusVisualizer() {
    shutdown();
}

bool VibeusVisualizer::initialize(int width, int height) {
    m_width = width;
    m_height = height;
    m_audioBuffer.reserve(4096);

    fprintf(stderr, "[VibeusVisualizer] Custom backend initialized (%dx%d)\n", width, height);
    return true;
}

void VibeusVisualizer::shutdown() {
    m_presets.clear();
    m_audioBuffer.clear();
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

    // Store audio buffer for visualization (simple implementation)
    m_audioBuffer.clear();
    for (uint32_t i = 0; i < numFrames * 2; i++) {
        m_audioBuffer.push_back(samples[i]);
    }
}

void VibeusVisualizer::renderFrame() {
    // Basic placeholder visualization:
    // - Audio-reactive gradient background
    // - Waveform overlay
    // This is a minimal demo - replace with custom effects later

    // Clear with audio-reactive color
    float intensity = std::min(1.0f, m_audioLevel * 2.0f * m_beatSensitivity);
    glClearColor(
        intensity * 0.2f,
        intensity * 0.4f,
        intensity * 0.6f,
        1.0f
    );
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw simple waveform if we have audio data
    if (!m_audioBuffer.empty()) {
        glColor4f(1.0f, 1.0f, 1.0f, 0.8f);
        glBegin(GL_LINE_STRIP);

        size_t skip = std::max<size_t>(1, m_audioBuffer.size() / 512);
        for (size_t i = 0; i < m_audioBuffer.size(); i += skip * 2) {
            float x = -1.0f + (2.0f * i) / m_audioBuffer.size();
            float y = m_audioBuffer[i] * 0.5f; // left channel
            glVertex2f(x, y);
        }

        glEnd();
    }

    // Draw preset name/info (would use text rendering in full implementation)
    // For now, just a colored rectangle indicating active preset
    if (!m_presets.empty()) {
        float hue = static_cast<float>(m_currentPreset) / std::max(1u, static_cast<uint32_t>(m_presets.size()));
        glColor4f(hue, 1.0f - hue, 0.5f, 0.3f);
        glBegin(GL_QUADS);
        glVertex2f(-0.9f, 0.8f);
        glVertex2f(-0.7f, 0.8f);
        glVertex2f(-0.7f, 0.9f);
        glVertex2f(-0.9f, 0.9f);
        glEnd();
    }
}

uint32_t VibeusVisualizer::loadPresetsFromDirectory(const std::string& path) {
    // Placeholder: In a real implementation, scan directory for preset files
    // For now, generate some dummy presets to demonstrate functionality

    uint32_t before = static_cast<uint32_t>(m_presets.size());

    // Add some placeholder presets
    for (int i = 0; i < 10; i++) {
        m_presets.push_back(path + "/VibeusPreset" + std::to_string(i + 1) + ".vbs");
    }

    uint32_t after = static_cast<uint32_t>(m_presets.size());
    fprintf(stderr, "[VibeusVisualizer] Loaded %u placeholder presets from %s\n",
            after - before, path.c_str());

    return after - before;
}

uint32_t VibeusVisualizer::getPresetCount() const {
    return static_cast<uint32_t>(m_presets.size());
}

void VibeusVisualizer::setPreset(uint32_t index, bool hardCut) {
    if (index < m_presets.size()) {
        m_currentPreset = index;
        if (m_logLevel >= 3) {
            fprintf(stderr, "[VibeusVisualizer] Switched to preset %u (%s)\n",
                    index, hardCut ? "hard cut" : "soft cut");
        }
    }
}

uint32_t VibeusVisualizer::getCurrentPresetIndex() const {
    return m_currentPreset;
}

std::string VibeusVisualizer::getPresetName(uint32_t index) const {
    if (index < m_presets.size()) {
        return m_presets[index];
    }
    return "";
}

void VibeusVisualizer::removePreset(uint32_t index) {
    if (index < m_presets.size()) {
        m_presets.erase(m_presets.begin() + index);
        if (m_currentPreset >= m_presets.size() && !m_presets.empty()) {
            m_currentPreset = static_cast<uint32_t>(m_presets.size() - 1);
        }
    }
}

void VibeusVisualizer::setBeatSensitivity(float sensitivity) {
    m_beatSensitivity = sensitivity;
}

float VibeusVisualizer::getBeatSensitivity() const {
    return m_beatSensitivity;
}

void VibeusVisualizer::setPresetDuration(double seconds) {
    m_presetDuration = seconds;
}

void VibeusVisualizer::setSoftCutDuration(double seconds) {
    m_softCutDuration = seconds;
}

double VibeusVisualizer::getSoftCutDuration() const {
    return m_softCutDuration;
}

void VibeusVisualizer::setHardCutEnabled(bool enabled) {
    m_hardCutEnabled = enabled;
}

void VibeusVisualizer::setHardCutSensitivity(float sensitivity) {
    m_hardCutSensitivity = sensitivity;
}

void VibeusVisualizer::setHardCutDuration(double seconds) {
    m_hardCutDuration = seconds;
}

void VibeusVisualizer::setPresetLocked(bool locked) {
    m_presetLocked = locked;
}

void VibeusVisualizer::setMeshSize(int width, int height) {
    m_meshWidth = width;
    m_meshHeight = height;
}

void VibeusVisualizer::setAspectCorrection(bool enabled) {
    m_aspectCorrection = enabled;
}

void VibeusVisualizer::setEasterEgg(float value) {
    m_easterEgg = value;
}

void VibeusVisualizer::setTextureSearchPaths(const char** paths, size_t count) {
    // Placeholder: store texture search paths
    if (m_logLevel >= 3) {
        fprintf(stderr, "[VibeusVisualizer] Set %zu texture search paths\n", count);
    }
}

void VibeusVisualizer::touch(float x, float y, int pressure, int type) {
    // Placeholder: interactive touch implementation
    if (m_logLevel >= 4) {
        fprintf(stderr, "[VibeusVisualizer] Touch at (%.2f, %.2f) pressure=%d type=%d\n",
                x, y, pressure, type);
    }
}

void VibeusVisualizer::touchDrag(float x, float y, int pressure) {
    // Placeholder: touch drag implementation
    if (m_logLevel >= 5) {
        fprintf(stderr, "[VibeusVisualizer] Touch drag to (%.2f, %.2f) pressure=%d\n",
                x, y, pressure);
    }
}

std::string VibeusVisualizer::getBackendInfo() const {
    return "Vibeus Custom Backend v0.1.0";
}

void VibeusVisualizer::setLogLevel(int level) {
    m_logLevel = level;
    if (level >= 3) {
        fprintf(stderr, "[VibeusVisualizer] Log level set to %d\n", level);
    }
}
