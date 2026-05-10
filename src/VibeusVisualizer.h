#pragma once

#include "IVisualizer.h"
#include <vector>
#include <string>

/// Vibeus custom visualizer backend.
/// A clean-slate implementation that provides full creative control
/// without any LGPL dependencies.
///
/// This is a basic placeholder implementation that demonstrates the abstraction.
/// Future enhancements can include:
/// - Custom shader system (GLSL-based effects)
/// - Modern GPU compute for audio analysis
/// - Custom preset format (JSON/YAML-based)
/// - Advanced beat detection and state machines
/// - ML-based audio feature extraction
class VibeusVisualizer : public IVisualizer {
public:
    VibeusVisualizer() = default;
    ~VibeusVisualizer() override;

    // IVisualizer implementation
    bool initialize(int width, int height) override;
    void shutdown() override;

    void setWindowSize(int width, int height) override;
    void setFrameTime(double seconds) override;

    void addAudioPCM(const float* samples, uint32_t numFrames) override;
    void renderFrame() override;

    uint32_t loadPresetsFromDirectory(const std::string& path) override;
    uint32_t getPresetCount() const override;
    void setPreset(uint32_t index, bool hardCut) override;
    uint32_t getCurrentPresetIndex() const override;
    std::string getPresetName(uint32_t index) const override;
    void removePreset(uint32_t index) override;

    void setBeatSensitivity(float sensitivity) override;
    float getBeatSensitivity() const override;
    void setPresetDuration(double seconds) override;
    void setSoftCutDuration(double seconds) override;
    double getSoftCutDuration() const override;
    void setHardCutEnabled(bool enabled) override;
    void setHardCutSensitivity(float sensitivity) override;
    void setHardCutDuration(double seconds) override;
    void setPresetLocked(bool locked) override;
    void setMeshSize(int width, int height) override;
    void setAspectCorrection(bool enabled) override;
    void setEasterEgg(float value) override;
    void setTextureSearchPaths(const char** paths, size_t count) override;

    void touch(float x, float y, int pressure, int type) override;
    void touchDrag(float x, float y, int pressure) override;

    std::string getBackendInfo() const override;
    void setLogLevel(int level) override;

private:
    // Window state
    int m_width = 0;
    int m_height = 0;
    double m_frameTime = 0.0;

    // Audio state
    std::vector<float> m_audioBuffer;
    float m_audioLevel = 0.0f;
    float m_beatSensitivity = 1.0f;

    // Preset state
    std::vector<std::string> m_presets;
    uint32_t m_currentPreset = 0;
    double m_presetDuration = 30.0;
    double m_softCutDuration = 3.0;
    double m_hardCutDuration = 15.0;
    bool m_hardCutEnabled = true;
    float m_hardCutSensitivity = 1.0f;
    bool m_presetLocked = false;

    // Rendering state
    int m_meshWidth = 64;
    int m_meshHeight = 48;
    bool m_aspectCorrection = true;
    float m_easterEgg = 0.0f;

    // Debug
    int m_logLevel = 2; // 2=warn
};
