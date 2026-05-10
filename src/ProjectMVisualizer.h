#pragma once

#include "IVisualizer.h"
#include <projectM-4/projectM.h>
#include <projectM-4/parameters.h>
#include <projectM-4/playlist.h>

/// ProjectM-based visualizer backend.
/// Wraps the projectM library to implement the IVisualizer interface.
/// This allows the app to use projectM through the abstraction layer,
/// enabling future migration to a custom backend.
class ProjectMVisualizer : public IVisualizer {
public:
    ProjectMVisualizer() = default;
    ~ProjectMVisualizer() override;

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

    // Direct access to projectM handle (for legacy code during migration)
    projectm_handle getHandle() const { return m_pm; }
    projectm_playlist_handle getPlaylistHandle() const { return m_playlist; }

private:
    projectm_handle m_pm = nullptr;
    projectm_playlist_handle m_playlist = nullptr;
};
