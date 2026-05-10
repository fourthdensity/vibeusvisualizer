#include "ProjectMVisualizer.h"
#include <cstdio>

ProjectMVisualizer::~ProjectMVisualizer() {
    shutdown();
}

bool ProjectMVisualizer::initialize(int width, int height) {
    m_pm = projectm_create();
    if (!m_pm) {
        fprintf(stderr, "[ProjectMVisualizer] projectm_create() failed\n");
        return false;
    }

    // Print version
    int major, minor, patch;
    projectm_get_version_components(&major, &minor, &patch);
    fprintf(stderr, "[ProjectMVisualizer] projectM %d.%d.%d initialized\n", major, minor, patch);

    // Set initial window size
    projectm_set_window_size(m_pm, width, height);

    // Create playlist
    m_playlist = projectm_playlist_create(m_pm);
    if (!m_playlist) {
        fprintf(stderr, "[ProjectMVisualizer] Failed to create playlist\n");
        projectm_destroy(m_pm);
        m_pm = nullptr;
        return false;
    }

    return true;
}

void ProjectMVisualizer::shutdown() {
    if (m_playlist) {
        projectm_playlist_destroy(m_playlist);
        m_playlist = nullptr;
    }
    if (m_pm) {
        projectm_destroy(m_pm);
        m_pm = nullptr;
    }
}

void ProjectMVisualizer::setWindowSize(int width, int height) {
    if (m_pm) {
        projectm_set_window_size(m_pm, width, height);
    }
}

void ProjectMVisualizer::setFrameTime(double seconds) {
    if (m_pm) {
        projectm_set_frame_time(m_pm, seconds);
    }
}

void ProjectMVisualizer::addAudioPCM(const float* samples, uint32_t numFrames) {
    if (m_pm && samples && numFrames > 0) {
        projectm_pcm_add_float(m_pm, samples, numFrames, PROJECTM_STEREO);
    }
}

void ProjectMVisualizer::renderFrame() {
    if (m_pm) {
        projectm_opengl_render_frame(m_pm);
    }
}

uint32_t ProjectMVisualizer::loadPresetsFromDirectory(const std::string& path) {
    if (!m_playlist) return 0;

    uint32_t before = projectm_playlist_size(m_playlist);
    projectm_playlist_add_path(m_playlist, path.c_str(), true, true);
    uint32_t after = projectm_playlist_size(m_playlist);

    return after - before;
}

uint32_t ProjectMVisualizer::getPresetCount() const {
    return m_playlist ? projectm_playlist_size(m_playlist) : 0;
}

void ProjectMVisualizer::setPreset(uint32_t index, bool hardCut) {
    if (m_playlist) {
        projectm_playlist_set_position(m_playlist, index, hardCut);
    }
}

uint32_t ProjectMVisualizer::getCurrentPresetIndex() const {
    return m_playlist ? projectm_playlist_position(m_playlist) : 0;
}

std::string ProjectMVisualizer::getPresetName(uint32_t index) const {
    if (!m_playlist) return "";

    char* name = projectm_playlist_item(m_playlist, index);
    if (!name) return "";

    std::string result(name);
    projectm_playlist_free_string(name);
    return result;
}

void ProjectMVisualizer::removePreset(uint32_t index) {
    if (m_playlist) {
        projectm_playlist_remove_index(m_playlist, index);
    }
}

void ProjectMVisualizer::setBeatSensitivity(float sensitivity) {
    if (m_pm) {
        projectm_set_beat_sensitivity(m_pm, sensitivity);
    }
}

float ProjectMVisualizer::getBeatSensitivity() const {
    return m_pm ? projectm_get_beat_sensitivity(m_pm) : 0.0f;
}

void ProjectMVisualizer::setPresetDuration(double seconds) {
    if (m_pm) {
        projectm_set_preset_duration(m_pm, seconds);
    }
}

void ProjectMVisualizer::setSoftCutDuration(double seconds) {
    if (m_pm) {
        projectm_set_soft_cut_duration(m_pm, seconds);
    }
}

double ProjectMVisualizer::getSoftCutDuration() const {
    return m_pm ? projectm_get_soft_cut_duration(m_pm) : 0.0;
}

void ProjectMVisualizer::setHardCutEnabled(bool enabled) {
    if (m_pm) {
        projectm_set_hard_cut_enabled(m_pm, enabled);
    }
}

void ProjectMVisualizer::setHardCutSensitivity(float sensitivity) {
    if (m_pm) {
        projectm_set_hard_cut_sensitivity(m_pm, sensitivity);
    }
}

void ProjectMVisualizer::setHardCutDuration(double seconds) {
    if (m_pm) {
        projectm_set_hard_cut_duration(m_pm, seconds);
    }
}

void ProjectMVisualizer::setPresetLocked(bool locked) {
    if (m_pm) {
        projectm_set_preset_locked(m_pm, locked);
    }
}

void ProjectMVisualizer::setMeshSize(int width, int height) {
    if (m_pm) {
        projectm_set_mesh_size(m_pm, width, height);
    }
}

void ProjectMVisualizer::setAspectCorrection(bool enabled) {
    if (m_pm) {
        projectm_set_aspect_correction(m_pm, enabled);
    }
}

void ProjectMVisualizer::setEasterEgg(float value) {
    if (m_pm) {
        projectm_set_easter_egg(m_pm, value);
    }
}

void ProjectMVisualizer::setTextureSearchPaths(const char** paths, size_t count) {
    if (m_pm) {
        projectm_set_texture_search_paths(m_pm, paths, count);
    }
}

void ProjectMVisualizer::touch(float x, float y, int pressure, int type) {
    if (m_pm) {
        projectm_touch(m_pm, x, y, pressure, static_cast<projectm_touch_type>(type));
    }
}

void ProjectMVisualizer::touchDrag(float x, float y, int pressure) {
    if (m_pm) {
        projectm_touch_drag(m_pm, x, y, pressure);
    }
}

std::string ProjectMVisualizer::getBackendInfo() const {
    if (!m_pm) return "ProjectM (uninitialized)";

    int major, minor, patch;
    projectm_get_version_components(&major, &minor, &patch);

    char buffer[128];
    snprintf(buffer, sizeof(buffer), "ProjectM %d.%d.%d", major, minor, patch);
    return buffer;
}

void ProjectMVisualizer::setLogLevel(int level) {
    projectm_log_level pmLevel;
    switch (level) {
        case 0: return; // off - don't set
        case 1: pmLevel = PROJECTM_LOG_LEVEL_ERROR; break;
        case 2: pmLevel = PROJECTM_LOG_LEVEL_WARN; break;
        case 3: pmLevel = PROJECTM_LOG_LEVEL_INFO; break;
        case 4: pmLevel = PROJECTM_LOG_LEVEL_DEBUG; break;
        case 5: pmLevel = PROJECTM_LOG_LEVEL_TRACE; break;
        default: pmLevel = PROJECTM_LOG_LEVEL_INFO; break;
    }
    projectm_set_log_level(pmLevel, false);
}
