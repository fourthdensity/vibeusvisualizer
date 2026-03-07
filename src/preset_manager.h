#pragma once

#include <projectM-4/types.h>
#include <projectM-4/playlist_types.h>
#include <string>
#include <cstdint>

/// Wraps the projectM playlist library for easy preset management
class PresetManager {
public:
    PresetManager() = default;
    ~PresetManager();

    /// Create the playlist and attach to a projectM instance.
    /// Scans presetPath for .milk files (recursively).
    bool init(projectm_handle pm, const std::string& presetPath);

    /// Play next preset (respects shuffle)
    void next(bool hardCut = false);

    /// Play previous preset
    void previous(bool hardCut = false);

    /// Go back in history
    void last(bool hardCut = false);

    /// Toggle shuffle mode
    void toggleShuffle();
    bool isShuffled() const;

    /// Get total preset count
    uint32_t count() const;

    /// Get current position in playlist
    uint32_t position() const;

    /// Get filename of current preset
    std::string currentPresetName() const;

    projectm_playlist_handle handle() const { return m_playlist; }

private:
    projectm_playlist_handle m_playlist = nullptr;
    projectm_handle          m_pm       = nullptr;
    bool                     m_shuffle  = true;
};
