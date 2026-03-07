#include "preset_manager.h"

#include <projectM-4/playlist.h>
#include <cstdio>

PresetManager::~PresetManager()
{
    if (m_playlist) {
        projectm_playlist_destroy(m_playlist);
        m_playlist = nullptr;
    }
}

bool PresetManager::init(projectm_handle pm, const std::string& presetPath)
{
    m_pm = pm;

    m_playlist = projectm_playlist_create(pm);
    if (!m_playlist) {
        fprintf(stderr, "[PresetManager] Failed to create playlist\n");
        return false;
    }

    uint32_t added = projectm_playlist_add_path(m_playlist, presetPath.c_str(), true, false);
    fprintf(stderr, "[PresetManager] Loaded %u presets from: %s\n", added, presetPath.c_str());

    if (added == 0) {
        fprintf(stderr, "[PresetManager] WARNING: No presets found!\n");
        return true; // still valid, just empty
    }

    projectm_playlist_set_shuffle(m_playlist, m_shuffle);
    projectm_playlist_play_next(m_playlist, true); // start first preset

    return true;
}

void PresetManager::next(bool hardCut)
{
    if (m_playlist)
        projectm_playlist_play_next(m_playlist, hardCut);
}

void PresetManager::previous(bool hardCut)
{
    if (m_playlist)
        projectm_playlist_play_previous(m_playlist, hardCut);
}

void PresetManager::last(bool hardCut)
{
    if (m_playlist)
        projectm_playlist_play_last(m_playlist, hardCut);
}

void PresetManager::toggleShuffle()
{
    m_shuffle = !m_shuffle;
    if (m_playlist)
        projectm_playlist_set_shuffle(m_playlist, m_shuffle);
    fprintf(stderr, "[PresetManager] Shuffle: %s\n", m_shuffle ? "ON" : "OFF");
}

bool PresetManager::isShuffled() const
{
    return m_shuffle;
}

uint32_t PresetManager::count() const
{
    return m_playlist ? projectm_playlist_size(m_playlist) : 0;
}

uint32_t PresetManager::position() const
{
    return m_playlist ? projectm_playlist_get_position(m_playlist) : 0;
}

std::string PresetManager::currentPresetName() const
{
    if (!m_playlist) return "";
    char* name = projectm_playlist_item(m_playlist, projectm_playlist_get_position(m_playlist));
    if (!name) return "";
    std::string result(name);
    projectm_playlist_free_string(name);
    return result;
}
