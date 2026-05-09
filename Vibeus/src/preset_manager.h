#pragma once

#include <projectM-4/types.h>
#include <projectM-4/playlist_types.h>
#include <string>
#include <vector>
#include <cstdint>
#include <deque>
#include <random>

/// Wraps the projectM playlist library for easy preset management
class PresetManager {
public:
    PresetManager() = default;
    ~PresetManager();

    /// Create the playlist and attach to a projectM instance.
    /// Scans presetPath for .milk files (recursively).
    bool init(projectm_handle pm, const std::string& presetPath);

    /// Remove presets listed in a saved blacklist file.
    /// Returns number of presets removed from the playlist.
    uint32_t applyBlacklist(const std::string& blacklistPath);

    /// Validate all presets and remove broken ones.
    /// Writes/updates the blacklist file and appends validation diagnostics to validationLogPath.
    /// If validation is skipped by user input, wasSkipped is set to true.
    /// Returns number of presets removed.
    uint32_t validateAndFilter(const std::string& userDataDir,
                               const std::string& blacklistPath,
                               const std::string& validationLogPath,
                               bool* wasSkipped = nullptr);

    /// Blacklist the currently playing preset (user curated).
    /// Removes it from the live playlist and appends to the user blacklist file.
    /// Returns the name of the removed preset (empty if none).
    std::string blacklistCurrent(const std::string& userBlacklistPath);

    /// Play next preset (respects shuffle)
    void next(bool hardCut = false);

    /// Play previous preset
    void previous(bool hardCut = false);

    /// Go back in history
    void last(bool hardCut = false);

    /// Jump to a random available preset in the current live playlist.
    bool playRandom(bool hardCut = true);

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
    bool playPosition(uint32_t pos, bool hardCut, const char* reason);
    bool chooseRandomPosition(uint32_t& pos);
    void rememberPosition(uint32_t pos, const std::string& presetPath);
    std::string presetPathAt(uint32_t pos) const;

    /// Remove presets by index (sorted descending)
    void removePresets(const std::vector<uint32_t>& indices);
    
    /// Save broken preset list to file
    void saveBrokenPresetLog(const std::string& blacklistPath,
                             const std::string& quarantineDirForNotes);

    projectm_playlist_handle m_playlist = nullptr;
    projectm_handle          m_pm       = nullptr;
    bool                     m_shuffle  = true;
    std::vector<std::string> m_brokenPresets; // Track broken preset paths
    std::deque<uint32_t>     m_recentPositions;
    std::deque<std::string>  m_recentPresetPaths;
    std::mt19937            m_rng{ std::random_device{}() };
};
