#pragma once

#include <string>
#include <vector>
#include <set>
#include <unordered_map>
#include <cstdint>
#include <random>
#include <filesystem>

namespace fs = std::filesystem;

// Primary preset categories based on folder structure
enum class PresetCategory {
    All,            // Virtual: all presets combined
    Favorites,      // Virtual: user favorites only
    Transition,     // ! Transition folder
    Dancer,
    Drawing,
    Fractal,
    Geometric,
    Hypnotic,
    Particles,
    Reaction,
    Sparkle,
    Supernova,
    Waveform,
    COUNT
};

// Preset tag flags (auto-detected from filename)
enum PresetTag : uint32_t {
    TAG_NONE        = 0,
    TAG_COLLAB      = 1 << 0,   // Multiple authors (contains " + " or ", ")
    TAG_MASHUP      = 1 << 1,   // Mashup preset
    TAG_REMIX       = 1 << 2,   // Remix/edit/mix variant
    TAG_NOISE_VAR   = 1 << 3,   // nz/nz+ noise variation
    TAG_ROAMING     = 1 << 4,   // Has camera roaming
    TAG_JELLY       = 1 << 5,   // Jelly effect variant
    TAG_ISOSCELES   = 1 << 6,   // Isosceles edit
};

// Single preset entry with metadata
struct PresetEntry {
    std::string fullPath;           // Full filesystem path to .milk file
    std::string displayName;        // Display name (preset title portion)
    std::string author;             // Parsed author name(s)
    PresetCategory category;        // Folder-based category
    uint32_t tags;                  // Bitmask of PresetTag flags
    uint32_t globalIndex;           // Index in the master list
};

// Category stack for navigation
struct CategoryStack {
    PresetCategory category;
    std::string displayName;        // Human-readable name
    std::string folderName;         // Actual folder name on disk
    std::string icon;               // Emoji or icon for UI
    std::vector<uint32_t> indices;  // Indices into master preset list
    
    uint32_t count() const { return static_cast<uint32_t>(indices.size()); }
};

// Category display info for UI
struct CategoryInfo {
    PresetCategory category;
    const char* displayName;
    const char* folderName;
    const char* icon;
};

// Static category metadata
inline const CategoryInfo CATEGORY_INFO[] = {
    { PresetCategory::All,        "All Presets",  "",            "≡"  },
    { PresetCategory::Favorites,  "Favorites",    "",            "★"  },
    { PresetCategory::Transition, "Transitions",  "! Transition", "↔"  },
    { PresetCategory::Dancer,     "Dancer",       "Dancer",       "💃" },
    { PresetCategory::Drawing,    "Drawing",      "Drawing",      "✎"  },
    { PresetCategory::Fractal,    "Fractal",      "Fractal",      "✦"  },
    { PresetCategory::Geometric,  "Geometric",    "Geometric",    "△"  },
    { PresetCategory::Hypnotic,   "Hypnotic",     "Hypnotic",     "○"  },
    { PresetCategory::Particles,  "Particles",    "Particles",    "⋮"  },
    { PresetCategory::Reaction,   "Reaction",     "Reaction",     "◎"  },
    { PresetCategory::Sparkle,    "Sparkle",      "Sparkle",      "✨" },
    { PresetCategory::Supernova,  "Supernova",    "Supernova",    "☀"  },
    { PresetCategory::Waveform,   "Waveform",     "Waveform",     "∿"  },
};

constexpr size_t CATEGORY_COUNT = sizeof(CATEGORY_INFO) / sizeof(CATEGORY_INFO[0]);

/// Database of presets with category/tag indexing
class PresetDatabase {
public:
    PresetDatabase();
    ~PresetDatabase() = default;

    /// Load presets from root directory (scans subfolders as categories)
    /// Returns number of presets loaded
    uint32_t loadFromDirectory(const std::string& rootPath);
    
    /// Check if database is loaded and valid
    bool isLoaded() const { return !m_presets.empty(); }

    
    /// Get all category stacks (for rendering category browser)
    const std::vector<CategoryStack>& categories() const { return m_categories; }
    
    /// Get specific category stack
    const CategoryStack* getCategory(PresetCategory cat) const;
    
    /// Get category by index (for UI iteration)
    const CategoryStack* getCategoryByIndex(size_t index) const;
    
    /// Number of categories (including virtual ones)
    size_t categoryCount() const { return m_categories.size(); }
    
    // ─── Preset Access ─────────────────────────────────────────────────────
    
    /// Get all presets
    const std::vector<PresetEntry>& presets() const { return m_presets; }
    
    /// Get preset by global index
    const PresetEntry* getPreset(uint32_t index) const;
    
    /// Total preset count
    uint32_t presetCount() const { return static_cast<uint32_t>(m_presets.size()); }
    
    /// Get presets in a category (returns indices)
    std::vector<uint32_t> getPresetsInCategory(PresetCategory cat) const;
    
    // ─── Search & Filter ───────────────────────────────────────────────────
    
    /// Search presets by name (case-insensitive substring match)
    std::vector<uint32_t> searchPresets(const std::string& query) const;
    
    /// Search within a specific category
    std::vector<uint32_t> searchInCategory(PresetCategory cat, const std::string& query) const;
    
    /// Get presets by author
    std::vector<uint32_t> getByAuthor(const std::string& author) const;
    
    /// Get presets matching tag mask (any match)
    std::vector<uint32_t> getByTags(uint32_t tagMask) const;
    
    /// Get list of known authors
    std::vector<std::string> getAuthors() const;
    
    // ─── Random Selection ──────────────────────────────────────────────────
    
    /// Random preset from entire collection
    uint32_t randomPreset();
    
    /// Random preset from specific category
    uint32_t randomFromCategory(PresetCategory cat);
    
    /// Random preset from favorites
    uint32_t randomFavorite();
    
    // ─── Favorites ─────────────────────────────────────────────────────────
    
    /// Check if preset is favorited
    bool isFavorite(uint32_t index) const;
    
    /// Toggle favorite status
    void toggleFavorite(uint32_t index);
    
    /// Add to favorites
    void addFavorite(uint32_t index);
    
    /// Remove from favorites
    void removeFavorite(uint32_t index);
    
    /// Get all favorites
    const std::set<uint32_t>& favorites() const { return m_favorites; }
    
    /// Number of favorites
    uint32_t favoriteCount() const { return static_cast<uint32_t>(m_favorites.size()); }
    
    /// Save favorites to file
    void saveFavorites(const std::string& path) const;
    
    /// Load favorites from file
    void loadFavorites(const std::string& path);
    
    // ─── Utility ───────────────────────────────────────────────────────────
    
    /// Get root path
    const std::string& rootPath() const { return m_rootPath; }
    
    /// Convert category enum to string
    static const char* categoryName(PresetCategory cat);
    
    /// Convert folder name to category enum
    static PresetCategory categoryFromFolder(const std::string& folderName);
    
private:
    // Parse a single preset filename into entry
    PresetEntry parsePreset(const fs::path& filepath, PresetCategory category);
    
    // Build category indices after loading
    void buildIndices();
    
    // Case-insensitive contains check
    static bool containsIgnoreCase(const std::string& haystack, const std::string& needle);
    
    std::string m_rootPath;
    std::vector<PresetEntry> m_presets;
    std::vector<CategoryStack> m_categories;
    std::set<uint32_t> m_favorites;
    std::unordered_map<std::string, std::vector<uint32_t>> m_authorIndex;
    
    std::mt19937 m_rng;
};
