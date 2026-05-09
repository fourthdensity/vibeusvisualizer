#include "preset_manager.h"

#include <projectM-4/playlist.h>
#include <cstdio>
#include <fstream>
#include <algorithm>
#include <filesystem>
#include <unordered_set>
#include <SDL2/SDL.h>
#include <ctime>

namespace fs = std::filesystem;

// External flags from main.cpp
extern bool g_lastPresetHadError;
extern bool g_captureProjectMErrors;
extern FILE* g_validationLogFile;

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

    return true;
}

void PresetManager::next(bool hardCut)
{
    if (!m_playlist)
        return;

    if (m_shuffle) {
        playRandom(hardCut);
        return;
    }

    uint32_t total = count();
    if (total == 0)
        return;

    uint32_t nextPos = (position() + 1) % total;
    playPosition(nextPos, hardCut, "Next");
}

void PresetManager::previous(bool hardCut)
{
    if (!m_playlist)
        return;

    uint32_t total = count();
    if (total == 0)
        return;

    uint32_t pos = position();
    uint32_t prevPos = (pos == 0) ? (total - 1) : (pos - 1);
    playPosition(prevPos, hardCut, "Previous");
}

void PresetManager::last(bool hardCut)
{
    if (m_playlist)
        projectm_playlist_play_last(m_playlist, hardCut);
}

bool PresetManager::playRandom(bool hardCut)
{
    if (!m_playlist)
        return false;

    uint32_t total = projectm_playlist_size(m_playlist);
    if (total == 0)
        return false;

    uint32_t pos = 0;
    if (!chooseRandomPosition(pos))
        return false;

    return playPosition(pos, hardCut, "Random");
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

std::string PresetManager::blacklistCurrent(const std::string& userBlacklistPath)
{
    if (!m_playlist) return "";

    uint32_t pos = projectm_playlist_get_position(m_playlist);
    char* name = projectm_playlist_item(m_playlist, pos);
    if (!name) return "";

    std::string presetPath(name);
    projectm_playlist_free_string(name);

    // Extract just the filename for display
    fs::path p(presetPath);
    std::string displayName = p.stem().string();

    // Write header if file doesn't exist yet
    bool isNew = !fs::exists(userBlacklistPath);

    // Append to user blacklist file
    std::ofstream file(userBlacklistPath, std::ios::app);
    if (file.is_open()) {
        if (isNew) {
            file << "# Vibeus User Blacklist\n";
            file << "# Presets you've chosen to skip.\n";
            file << "# Remove a line to restore that preset.\n";
            file << "#\n\n";
        }
        file << presetPath << "\n";
        file.close();
    }

    // Remove from live playlist and advance
    projectm_playlist_remove_preset(m_playlist, pos);
    if (count() > 0) {
        projectm_playlist_play_next(m_playlist, false);
    }

    fprintf(stderr, "[PresetManager] Blacklisted: %s (%u remaining)\n",
            displayName.c_str(), count());

    return displayName;
}

uint32_t PresetManager::applyBlacklist(const std::string& blacklistPath)
{
    if (!m_playlist) return 0;

    std::ifstream f(blacklistPath);
    if (!f.is_open()) return 0;

    std::unordered_set<std::string> blacklist;
    std::string line;
    while (std::getline(f, line)) {
        if (line.empty()) continue;
        if (line[0] == '#') continue;
        // Trim
        while (!line.empty() && (line.back() == '\r' || line.back() == ' ' || line.back() == '\t')) line.pop_back();
        size_t start = 0;
        while (start < line.size() && (line[start] == ' ' || line[start] == '\t')) start++;
        if (start > 0) line.erase(0, start);
        if (!line.empty()) blacklist.insert(line);
    }

    if (blacklist.empty()) return 0;

    uint32_t removed = 0;
    for (int64_t i = (int64_t)projectm_playlist_size(m_playlist) - 1; i >= 0; --i) {
        char* name = projectm_playlist_item(m_playlist, (uint32_t)i);
        if (!name) continue;
        std::string preset(name);
        projectm_playlist_free_string(name);

        if (blacklist.find(preset) != blacklist.end()) {
            projectm_playlist_remove_preset(m_playlist, (uint32_t)i);
            removed++;
        }
    }

    if (removed > 0)
        fprintf(stderr, "[PresetManager] Applied blacklist: removed %u presets\n", removed);

    if (removed > 0) {
        m_recentPositions.clear();
        m_recentPresetPaths.clear();
    }

    return removed;
}

bool PresetManager::playPosition(uint32_t pos, bool hardCut, const char* reason)
{
    if (!m_playlist)
        return false;

    uint32_t total = count();
    if (total == 0 || pos >= total)
        return false;

    projectm_playlist_set_position(m_playlist, pos, hardCut);

    std::string presetPath = presetPathAt(pos);
    rememberPosition(pos, presetPath);

    if (!presetPath.empty()) {
        fprintf(stderr, "[PresetManager] %s preset %u/%u: %s\n",
                reason ? reason : "Play", pos + 1, total, presetPath.c_str());
    } else {
        fprintf(stderr, "[PresetManager] %s preset %u/%u\n",
                reason ? reason : "Play", pos + 1, total);
    }

    return true;
}

bool PresetManager::chooseRandomPosition(uint32_t& pos)
{
    uint32_t total = count();
    if (total == 0)
        return false;

    if (total == 1) {
        pos = 0;
        return true;
    }

    uint32_t current = position();
    std::string currentPath = presetPathAt(current);
    size_t recentLimit = std::min<size_t>(m_recentPositions.size(), std::min<uint32_t>(total - 1, 48));

    for (int attempt = 0; attempt < 64; ++attempt) {
        std::uniform_int_distribution<uint32_t> dist(0, total - 1);
        uint32_t candidate = dist(m_rng);
        if (candidate == current)
            continue;

        std::string candidatePath = presetPathAt(candidate);
        if (!currentPath.empty() && candidatePath == currentPath)
            continue;

        bool recentlySeen = false;
        size_t checked = 0;
        for (auto it = m_recentPositions.rbegin(); it != m_recentPositions.rend() && checked < recentLimit; ++it, ++checked) {
            if (*it == candidate) {
                recentlySeen = true;
                break;
            }
        }

        if (!candidatePath.empty()) {
            checked = 0;
            for (auto it = m_recentPresetPaths.rbegin(); it != m_recentPresetPaths.rend() && checked < recentLimit; ++it, ++checked) {
                if (*it == candidatePath) {
                    recentlySeen = true;
                    break;
                }
            }
        }

        if (!recentlySeen) {
            pos = candidate;
            return true;
        }
    }

    // Dense recent history fallback: still guarantee not-current.
    std::uniform_int_distribution<uint32_t> dist(0, total - 2);
    uint32_t candidate = dist(m_rng);
    if (candidate >= current)
        candidate++;
    pos = candidate;
    return true;
}

void PresetManager::rememberPosition(uint32_t pos, const std::string& presetPath)
{
    m_recentPositions.push_back(pos);
    while (m_recentPositions.size() > 96)
        m_recentPositions.pop_front();

    if (!presetPath.empty()) {
        m_recentPresetPaths.push_back(presetPath);
        while (m_recentPresetPaths.size() > 96)
            m_recentPresetPaths.pop_front();
    }
}

std::string PresetManager::presetPathAt(uint32_t pos) const
{
    if (!m_playlist || pos >= projectm_playlist_size(m_playlist))
        return {};

    char* name = projectm_playlist_item(m_playlist, pos);
    if (!name)
        return {};

    std::string result(name);
    projectm_playlist_free_string(name);
    return result;
}

uint32_t PresetManager::validateAndFilter(const std::string& userDataDir,
                                          const std::string& blacklistPath,
                                          const std::string& validationLogPath,
                                          bool* wasSkipped)
{
    if (!m_playlist) return 0;
    if (wasSkipped) *wasSkipped = false;

    FILE* validationLog = nullptr;
    errno_t openErr = fopen_s(&validationLog, validationLogPath.c_str(), "a");
    if (openErr != 0 || !validationLog) {
        fprintf(stderr, "[PresetManager] WARNING: Could not open validation log: %s\n", validationLogPath.c_str());
    } else {
        std::time_t t = std::time(nullptr);
        const char* ts = std::ctime(&t);
        fprintf(validationLog, "\n=== Preset Validation Run: %s===\n", ts ? ts : "");
        fflush(validationLog);
    }

    FILE* previousValidationLog = g_validationLogFile;
    g_validationLogFile = validationLog;
    
    std::vector<uint32_t> brokenIndices;
    uint32_t total = count();
    uint32_t currentPos = position();
    
    fprintf(stderr, "[PresetManager] Validating %u presets...\n", total);
    
    // Create quarantine directory in a per-user writable location.
    // Installed builds typically live under Program Files (not writable), so we never rely on that.
    fs::path quarantineDir = fs::path(userDataDir) / "broken_presets_quarantine";
    bool canQuarantine = true;
    try {
        if (!fs::exists(quarantineDir)) {
            fs::create_directories(quarantineDir);
            fprintf(stderr, "[PresetManager] Quarantine dir: %s\n", quarantineDir.string().c_str());
        }
    } catch (const std::exception& e) {
        canQuarantine = false;
        fprintf(stderr, "[PresetManager] WARNING: Could not create quarantine dir (%s).\n", e.what());
        fprintf(stderr, "[PresetManager] Broken presets will be blacklisted (not moved).\n");
    }
    
    // Temporarily disable shuffle for sequential testing
    bool wasShuffled = m_shuffle;
    projectm_playlist_set_shuffle(m_playlist, false);
    
    for (uint32_t i = 0; i < total; i++) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT ||
                (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)) {
                fprintf(stderr, "[PresetManager] Validation skipped by user.\n");
                if (validationLog) {
                    fprintf(validationLog, "[PresetManager] Validation skipped by user at preset %u/%u\n", i, total);
                    fflush(validationLog);
                }
                if (wasSkipped) *wasSkipped = true;
                g_captureProjectMErrors = false;
                projectm_playlist_set_shuffle(m_playlist, wasShuffled);
                if (currentPos < total) {
                    projectm_playlist_set_position(m_playlist, currentPos, true);
                }
                if (!brokenIndices.empty()) {
                    removePresets(brokenIndices);
                    saveBrokenPresetLog(blacklistPath, quarantineDir.string());
                }
                g_validationLogFile = previousValidationLog;
                if (validationLog) fclose(validationLog);
                return (uint32_t)brokenIndices.size();
            }
        }

        // Show progress every 100 presets
        if (i % 100 == 0) {
            fprintf(stderr, "[PresetManager] Testing preset %u/%u...\n", i, total);
            if (validationLog) {
                fprintf(validationLog, "[PresetManager] Testing preset %u/%u\n", i, total);
            }
        }
        
        // Load preset at index i
        g_lastPresetHadError = false;
        g_captureProjectMErrors = true;
        projectm_playlist_set_position(m_playlist, i, true);

        // Wait a bit for projectM to compile preset code (GL + scripts)
        SDL_Delay(60);
        g_captureProjectMErrors = false;

        // Keep the OS message pump alive during long scans
        SDL_PumpEvents();
        
        // Check if error occurred
        if (g_lastPresetHadError) {
            // Get preset path
            char* name = projectm_playlist_item(m_playlist, i);
            if (name) {
                fs::path presetPath(name);
                m_brokenPresets.push_back(std::string(name));
                
                // Best-effort move to quarantine (may fail under Program Files permissions)
                if (canQuarantine) {
                    try {
                        if (fs::exists(presetPath)) {
                            fs::path quarantinePath = quarantineDir / presetPath.filename();
                            // If already exists in quarantine, add number suffix
                            int suffix = 1;
                            while (fs::exists(quarantinePath)) {
                                std::string newName = presetPath.stem().string() + "_" + std::to_string(suffix) + presetPath.extension().string();
                                quarantinePath = quarantineDir / newName;
                                suffix++;
                            }
                            fs::rename(presetPath, quarantinePath);
                            fprintf(stderr, "[PresetManager] QUARANTINED: %s\n", presetPath.filename().string().c_str());
                            if (validationLog) {
                                fprintf(validationLog, "[PresetManager] QUARANTINED: %s\n", presetPath.string().c_str());
                            }
                        }
                    } catch (const std::exception& e) {
                        fprintf(stderr, "[PresetManager] WARNING: Could not move preset (will remain on disk): %s\n", e.what());
                        if (validationLog) {
                            fprintf(validationLog, "[PresetManager] WARNING: Could not move preset: %s\n", e.what());
                        }
                    }
                }
                
                projectm_playlist_free_string(name);
            }
            brokenIndices.push_back(i);
        }
    }
    
    fprintf(stderr, "[PresetManager] Validation complete: %u broken presets found\n", 
            (uint32_t)brokenIndices.size());
    
    // Restore shuffle state
    projectm_playlist_set_shuffle(m_playlist, wasShuffled);
    
    // Restore original position (or close to it after removal)
    if (currentPos < total) {
        projectm_playlist_set_position(m_playlist, currentPos, true);
    }
    
    // Remove broken presets from playlist and write/update blacklist
    if (!brokenIndices.empty()) {
        removePresets(brokenIndices);
        saveBrokenPresetLog(blacklistPath, quarantineDir.string());
    }

    if (validationLog) {
        fprintf(validationLog, "[PresetManager] Validation complete: %zu broken presets found\n", brokenIndices.size());
        fflush(validationLog);
    }
    g_validationLogFile = previousValidationLog;
    if (validationLog) fclose(validationLog);

    return (uint32_t)brokenIndices.size();
}

void PresetManager::removePresets(const std::vector<uint32_t>& indices)
{
    if (!m_playlist || indices.empty()) return;
    
    // Sort indices in descending order to preserve positions during removal
    std::vector<uint32_t> sorted = indices;
    std::sort(sorted.begin(), sorted.end(), std::greater<uint32_t>());
    
    fprintf(stderr, "[PresetManager] Removing %u broken presets...\n", (uint32_t)sorted.size());
    
    for (uint32_t idx : sorted) {
        projectm_playlist_remove_preset(m_playlist, idx);
    }
    
    fprintf(stderr, "[PresetManager] Removal complete. New count: %u\n", count());
}

void PresetManager::saveBrokenPresetLog(const std::string& blacklistPath,
                                         const std::string& quarantineDirForNotes)
{
    if (m_brokenPresets.empty()) return;
    
    std::ofstream file(blacklistPath);
    if (!file.is_open()) {
        fprintf(stderr, "[PresetManager] WARNING: Failed to write broken preset blacklist: %s\n", blacklistPath.c_str());
        return;
    }
    
    file << "# Broken Presets Detected by Vibeus\n";
    file << "# These presets had shader compilation errors and were removed from the runtime playlist.\n";
    file << "# They are persisted here so they stay removed on subsequent launches.\n";
    file << "# Total: " << m_brokenPresets.size() << " presets\n";
    file << "#\n";
    file << "# Quarantine folder (best-effort): " << quarantineDirForNotes << "\n";
    file << "# NOTE: On installed builds, presets may not be movable due to permissions.\n";
    file << "# In that case, this file still blacklists them so they won't appear next run.\n";
    file << "#\n";
    file << "# To restore a preset:\n";
    file << "# 1. Move the .milk file back to the presets folder\n";
    file << "# 2. Restart Vibeus\n";
    file << "#\n";
    file << "# To review:\n";
    file << "# 1. Open in text editor to inspect shader code\n";
    file << "# 2. Try loading in projectM standalone\n";
    file << "# 3. Report to preset pack maintainer for fixes\n";
    file << "#\n\n";
    
    for (const auto& preset : m_brokenPresets) {
        file << preset << "\n";
    }
    
    file.close();
    fprintf(stderr, "[PresetManager] Saved broken preset blacklist to: %s\n", blacklistPath.c_str());
}
