#include "preset_database.h"

#include <algorithm>
#include <fstream>
#include <cctype>
#include <chrono>

PresetDatabase::PresetDatabase()
    : m_rng(static_cast<unsigned>(
          std::chrono::steady_clock::now().time_since_epoch().count()))
{
}

uint32_t PresetDatabase::loadFromDirectory(const std::string& rootPath)
{
    m_rootPath = rootPath;
    m_presets.clear();
    m_categories.clear();
    m_authorIndex.clear();

    if (!fs::exists(rootPath) || !fs::is_directory(rootPath)) {
        return 0;
    }

    // First pass: scan all subfolders for .milk files
    for (const auto& entry : fs::directory_iterator(rootPath)) {
        if (!entry.is_directory()) continue;
        
        std::string folderName = entry.path().filename().string();
        PresetCategory cat = categoryFromFolder(folderName);
        
        // Skip unknown folders
        if (cat == PresetCategory::All) continue;
        
        // Recursively find all .milk files in this category folder
        for (const auto& preset : fs::recursive_directory_iterator(entry.path())) {
            if (!preset.is_regular_file()) continue;
            
            std::string ext = preset.path().extension().string();
            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
            
            if (ext == ".milk") {
                PresetEntry pe = parsePreset(preset.path(), cat);
                pe.globalIndex = static_cast<uint32_t>(m_presets.size());
                m_presets.push_back(std::move(pe));
            }
        }
    }

    // Build category and author indices
    buildIndices();

    return static_cast<uint32_t>(m_presets.size());
}

PresetEntry PresetDatabase::parsePreset(const fs::path& filepath, PresetCategory category)
{
    PresetEntry entry;
    entry.fullPath = filepath.string();
    entry.category = category;
    entry.tags = TAG_NONE;
    
    // Get filename without extension
    std::string name = filepath.stem().string();
    entry.displayName = name;
    
    // Parse author: text before first " - "
    size_t dashPos = name.find(" - ");
    if (dashPos != std::string::npos && dashPos > 0) {
        entry.author = name.substr(0, dashPos);
        // Keep display name as full name for now (could strip author if desired)
    } else {
        entry.author = "";
    }
    
    // Detect tags from filename
    if (name.find(" + ") != std::string::npos || 
        name.find(", ") != std::string::npos) {
        entry.tags |= TAG_COLLAB;
    }
    if (containsIgnoreCase(name, "mashup")) {
        entry.tags |= TAG_MASHUP;
    }
    if (containsIgnoreCase(name, "remix") || 
        containsIgnoreCase(name, " mix") ||
        containsIgnoreCase(name, "edit")) {
        entry.tags |= TAG_REMIX;
    }
    if (name.find("nz+") != std::string::npos || 
        name.find(" nz") != std::string::npos ||
        name.find("nz ") != std::string::npos) {
        entry.tags |= TAG_NOISE_VAR;
    }
    if (containsIgnoreCase(name, "roam")) {
        entry.tags |= TAG_ROAMING;
    }
    if (containsIgnoreCase(name, "jelly")) {
        entry.tags |= TAG_JELLY;
    }
    if (containsIgnoreCase(name, "isosceles")) {
        entry.tags |= TAG_ISOSCELES;
    }
    
    return entry;
}

void PresetDatabase::buildIndices()
{
    m_categories.clear();
    m_authorIndex.clear();
    
    // Initialize category stacks for each category type
    for (size_t i = 0; i < CATEGORY_COUNT; ++i) {
        const auto& info = CATEGORY_INFO[i];
        CategoryStack stack;
        stack.category = info.category;
        stack.displayName = info.displayName;
        stack.folderName = info.folderName;
        stack.icon = info.icon;
        m_categories.push_back(std::move(stack));
    }
    
    // Populate category indices and author index
    for (uint32_t i = 0; i < m_presets.size(); ++i) {
        const auto& preset = m_presets[i];
        
        // Add to specific category
        for (auto& cat : m_categories) {
            if (cat.category == preset.category) {
                cat.indices.push_back(i);
                break;
            }
        }
        
        // Add to "All" virtual category
        for (auto& cat : m_categories) {
            if (cat.category == PresetCategory::All) {
                cat.indices.push_back(i);
                break;
            }
        }
        
        // Add to author index
        if (!preset.author.empty()) {
            // Normalize author name for indexing (lowercase)
            std::string authorKey = preset.author;
            std::transform(authorKey.begin(), authorKey.end(), authorKey.begin(), ::tolower);
            m_authorIndex[authorKey].push_back(i);
        }
    }
    
    // Sort presets within each category by display name
    for (auto& cat : m_categories) {
        std::sort(cat.indices.begin(), cat.indices.end(),
            [this](uint32_t a, uint32_t b) {
                return m_presets[a].displayName < m_presets[b].displayName;
            });
    }
}

const CategoryStack* PresetDatabase::getCategory(PresetCategory cat) const
{
    for (const auto& stack : m_categories) {
        if (stack.category == cat) {
            return &stack;
        }
    }
    return nullptr;
}

const CategoryStack* PresetDatabase::getCategoryByIndex(size_t index) const
{
    if (index < m_categories.size()) {
        return &m_categories[index];
    }
    return nullptr;
}

const PresetEntry* PresetDatabase::getPreset(uint32_t index) const
{
    if (index < m_presets.size()) {
        return &m_presets[index];
    }
    return nullptr;
}

std::vector<uint32_t> PresetDatabase::getPresetsInCategory(PresetCategory cat) const
{
    // Handle favorites specially
    if (cat == PresetCategory::Favorites) {
        return std::vector<uint32_t>(m_favorites.begin(), m_favorites.end());
    }
    
    const CategoryStack* stack = getCategory(cat);
    if (stack) {
        return stack->indices;
    }
    return {};
}

std::vector<uint32_t> PresetDatabase::searchPresets(const std::string& query) const
{
    std::vector<uint32_t> results;
    if (query.empty()) {
        // Return all if no query
        results.reserve(m_presets.size());
        for (uint32_t i = 0; i < m_presets.size(); ++i) {
            results.push_back(i);
        }
        return results;
    }
    
    for (uint32_t i = 0; i < m_presets.size(); ++i) {
        if (containsIgnoreCase(m_presets[i].displayName, query)) {
            results.push_back(i);
        }
    }
    return results;
}

std::vector<uint32_t> PresetDatabase::searchInCategory(PresetCategory cat, 
                                                        const std::string& query) const
{
    std::vector<uint32_t> results;
    std::vector<uint32_t> indices = getPresetsInCategory(cat);
    
    if (query.empty()) {
        return indices;
    }
    
    for (uint32_t idx : indices) {
        if (containsIgnoreCase(m_presets[idx].displayName, query)) {
            results.push_back(idx);
        }
    }
    return results;
}

std::vector<uint32_t> PresetDatabase::getByAuthor(const std::string& author) const
{
    std::string authorKey = author;
    std::transform(authorKey.begin(), authorKey.end(), authorKey.begin(), ::tolower);
    
    auto it = m_authorIndex.find(authorKey);
    if (it != m_authorIndex.end()) {
        return it->second;
    }
    return {};
}

std::vector<uint32_t> PresetDatabase::getByTags(uint32_t tagMask) const
{
    std::vector<uint32_t> results;
    for (uint32_t i = 0; i < m_presets.size(); ++i) {
        if (m_presets[i].tags & tagMask) {
            results.push_back(i);
        }
    }
    return results;
}

std::vector<std::string> PresetDatabase::getAuthors() const
{
    std::vector<std::string> authors;
    authors.reserve(m_authorIndex.size());
    for (const auto& [author, indices] : m_authorIndex) {
        authors.push_back(author);
    }
    std::sort(authors.begin(), authors.end());
    return authors;
}

uint32_t PresetDatabase::randomPreset()
{
    if (m_presets.empty()) return 0;
    std::uniform_int_distribution<uint32_t> dist(0, 
        static_cast<uint32_t>(m_presets.size() - 1));
    return dist(m_rng);
}

uint32_t PresetDatabase::randomFromCategory(PresetCategory cat)
{
    std::vector<uint32_t> indices = getPresetsInCategory(cat);
    if (indices.empty()) return randomPreset();
    
    std::uniform_int_distribution<size_t> dist(0, indices.size() - 1);
    return indices[dist(m_rng)];
}

uint32_t PresetDatabase::randomFavorite()
{
    if (m_favorites.empty()) return randomPreset();
    
    std::vector<uint32_t> favVec(m_favorites.begin(), m_favorites.end());
    std::uniform_int_distribution<size_t> dist(0, favVec.size() - 1);
    return favVec[dist(m_rng)];
}

bool PresetDatabase::isFavorite(uint32_t index) const
{
    return m_favorites.count(index) > 0;
}

void PresetDatabase::toggleFavorite(uint32_t index)
{
    if (m_favorites.count(index)) {
        m_favorites.erase(index);
    } else {
        m_favorites.insert(index);
    }
    
    // Update favorites category count
    for (auto& cat : m_categories) {
        if (cat.category == PresetCategory::Favorites) {
            cat.indices.clear();
            cat.indices.insert(cat.indices.end(), 
                               m_favorites.begin(), m_favorites.end());
            break;
        }
    }
}

void PresetDatabase::addFavorite(uint32_t index)
{
    if (index < m_presets.size()) {
        m_favorites.insert(index);
    }
}

void PresetDatabase::removeFavorite(uint32_t index)
{
    m_favorites.erase(index);
}

void PresetDatabase::saveFavorites(const std::string& path) const
{
    std::ofstream file(path);
    if (!file.is_open()) return;
    
    for (uint32_t idx : m_favorites) {
        if (idx < m_presets.size()) {
            file << m_presets[idx].fullPath << "\n";
        }
    }
}

void PresetDatabase::loadFavorites(const std::string& path)
{
    m_favorites.clear();
    
    std::ifstream file(path);
    if (!file.is_open()) return;
    
    // Build path-to-index map for lookup
    std::unordered_map<std::string, uint32_t> pathIndex;
    for (uint32_t i = 0; i < m_presets.size(); ++i) {
        pathIndex[m_presets[i].fullPath] = i;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        
        auto it = pathIndex.find(line);
        if (it != pathIndex.end()) {
            m_favorites.insert(it->second);
        }
    }
    
    // Update favorites category
    for (auto& cat : m_categories) {
        if (cat.category == PresetCategory::Favorites) {
            cat.indices.clear();
            cat.indices.insert(cat.indices.end(), 
                               m_favorites.begin(), m_favorites.end());
            break;
        }
    }
}

const char* PresetDatabase::categoryName(PresetCategory cat)
{
    for (const auto& info : CATEGORY_INFO) {
        if (info.category == cat) {
            return info.displayName;
        }
    }
    return "Unknown";
}

PresetCategory PresetDatabase::categoryFromFolder(const std::string& folderName)
{
    for (const auto& info : CATEGORY_INFO) {
        if (info.folderName[0] != '\0' && folderName == info.folderName) {
            return info.category;
        }
    }
    return PresetCategory::All; // Unknown folders go to All
}

bool PresetDatabase::containsIgnoreCase(const std::string& haystack, 
                                         const std::string& needle)
{
    if (needle.empty()) return true;
    if (haystack.empty()) return false;
    
    auto it = std::search(
        haystack.begin(), haystack.end(),
        needle.begin(), needle.end(),
        [](char a, char b) {
            return std::tolower(static_cast<unsigned char>(a)) == 
                   std::tolower(static_cast<unsigned char>(b));
        });
    
    return it != haystack.end();
}
