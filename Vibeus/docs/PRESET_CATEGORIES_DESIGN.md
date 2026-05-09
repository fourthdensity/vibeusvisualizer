# Preset Categorization System Design

## Executive Summary

This document describes a comprehensive categorization system for Vibeus's ~9,800 Milkdrop presets. The system leverages the existing folder structure while providing intuitive stack-based navigation with filtering capabilities.

---

## 1. Analysis of Existing Structure

### 1.1 Current Directory Organization

The `presets-cream-of-the-crop` collection is **already pre-categorized** into 11 visual categories:

| Category     | Count  | Description                                    |
|--------------|--------|------------------------------------------------|
| Dancer       | 1,351  | Organic, flowing, anthropomorphic shapes       |
| Drawing      | 1,143  | Line art, sketchy, hand-drawn aesthetics       |
| Fractal      | 1,354  | Mathematical fractals, recursive patterns      |
| Geometric    | 1,027  | Cubes, shapes, architectural structures        |
| Hypnotic     | 280    | Optical illusions, Escher-style patterns       |
| Particles    | 389    | Particle systems, emitters, sparks             |
| Reaction     | 1,791  | Highly music-reactive, bass-driven visuals     |
| Sparkle      | 797    | Stars, glitter, luminous effects               |
| Supernova    | 380    | Cosmic explosions, space-themed                |
| Waveform     | 1,279  | Audio waveform visualizations                  |
| ! Transition | 4      | Special transition presets                     |
| **Total**    | **9,795** |                                             |

### 1.2 Filename Naming Patterns

Analysis reveals consistent patterns:

**Author Prefixes:**
- `author - preset name.milk` (most common)
- `author + author2 - name.milk` (collaborations)
- `$$$ Royal - Mashup (N).milk` (mashup series)

**Top Authors (by preset count):**
| Author        | Count | Author        | Count |
|---------------|-------|---------------|-------|
| suksma        | 1,972 | martin        | 244   |
| flexi         | 362   | amandio c     | 218   |
| eos           | 350   | geiss         | 188   |
| shifter       | 290   | orb           | 174   |
| $$$ royal     | 271   | tonymilkdrop  | 152   |
| luxxx         | 247   | tripgnosis    | 132   |

**Collaboration presets:** 2,087 (21% of collection)

**Modifier Suffixes (auto-detected tags):**
| Tag Pattern     | Example            | Count  |
|-----------------|--------------------| -------|
| Collab (+ or ,) | `shifter + flexi`  | 2,087  |
| Remix/Mix/Edit  | `EoS remix`        | 1,489  |
| Noise (nz/nz+)  | `preset nz+`       | 1,085  |
| Isosceles edit  | `--- Isosceles`    | 569    |
| Mashup          | `Royal - Mashup`   | 485    |
| Roaming         | `preset roams`     | 364    |
| Jelly           | `Jelly V3`         | 190    |

---

## 2. Proposed Category System

### 2.1 Primary Categories (Folder-Based)

Since presets are already organized in folders, use these as **first-class categories**:

```cpp
enum class PresetCategory {
    All,            // Virtual: all presets combined
    Favorites,      // Virtual: user favorites
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
};
```

### 2.2 Secondary Tags (Filename-Derived)

Auto-extract tags for additional filtering:

| Tag Type     | Detection Pattern                        | Example             |
|--------------|------------------------------------------|---------------------|
| Author       | First segment before ` - `               | `martin`, `flexi`   |
| Collab       | Contains ` + ` or `, ` in author         | `shifter + Flexi`   |
| Mashup       | Contains `Mashup`                        | `$$$ Royal - Mashup`|
| Remix        | Contains `remix`, `mix`, `edit`          | `EoS remix`         |
| NoiseVar     | Contains `nz+` or `nz`                   | `preset nz+`        |
| Roaming      | Contains `roam`                          | `preset roams`      |
| Jelly        | Contains `Jelly`                         | `Jelly V3`          |

### 2.3 Mood-Based Smart Stacks

Virtual categories that span folders based on visual characteristics:

| Stack Name     | Algorithm                                            |
|----------------|------------------------------------------------------|
| **Chill**      | Waveform + Hypnotic + slow/ambient keywords          |
| **Intense**    | Reaction + Supernova + "bass", "beat" keywords       |
| **Cosmic**     | Supernova + Particles + "space", "nebula", "star"    |
| **Trippy**     | Fractal + Hypnotic + "psychedelic" keywords          |
| **Organic**    | Dancer + Drawing + "flow", "liquid" keywords         |

---

## 3. Data Structures

### 3.1 PresetEntry (Enhanced)

```cpp
struct PresetEntry {
    std::string fullPath;           // Full filesystem path
    std::string displayName;        // Extracted from filename
    std::string author;             // Parsed author name
    PresetCategory category;        // Folder-based category
    uint32_t tags;                  // Bitmask of PresetTag flags
    bool isFavorite;                // User favorite status
};
```

### 3.2 PresetTag Flags

```cpp
enum PresetTag : uint32_t {
    TAG_NONE        = 0,
    TAG_COLLAB      = 1 << 0,   // Multiple authors
    TAG_MASHUP      = 1 << 1,   // Mashup preset
    TAG_REMIX       = 1 << 2,   // Remix/edit
    TAG_NOISE_VAR   = 1 << 3,   // nz+ variant
    TAG_ROAMING     = 1 << 4,   // Has camera roaming
    TAG_JELLY       = 1 << 5,   // Jelly effect variant
    TAG_ISOSCELES   = 1 << 6,   // Isosceles edit
};
```

### 3.3 CategoryStack

```cpp
struct CategoryStack {
    PresetCategory category;
    std::string displayName;
    std::string icon;               // Emoji or icon code
    uint32_t presetCount;           // Cached count
    std::vector<uint32_t> indices;  // Indices into main preset list
};
```

### 3.4 Preset Database

```cpp
class PresetDatabase {
public:
    // Initialization
    bool loadFromDirectory(const std::string& rootPath);
    
    // Category access
    const std::vector<CategoryStack>& categories() const;
    const CategoryStack& getCategory(PresetCategory cat) const;
    
    // Preset access
    const PresetEntry& getPreset(uint32_t index) const;
    std::vector<uint32_t> searchPresets(const std::string& query) const;
    std::vector<uint32_t> getByAuthor(const std::string& author) const;
    std::vector<uint32_t> getByTags(uint32_t tagMask) const;
    
    // Random selection
    uint32_t randomFromCategory(PresetCategory cat) const;
    uint32_t randomFromAll() const;
    
    // Favorites
    void toggleFavorite(uint32_t index);
    const std::vector<uint32_t>& favorites() const;
    
private:
    std::vector<PresetEntry> m_presets;
    std::vector<CategoryStack> m_categories;
    std::vector<uint32_t> m_favorites;
    std::unordered_map<std::string, std::vector<uint32_t>> m_authorIndex;
};
```

---

## 4. UI/UX Design

### 4.1 Navigation Flow

```
┌─────────────────────────────────────────────────────┐
│  MAIN MENU                                          │
│  ├── Start Visualizer                               │
│  ├── Browse Presets ──────────────────────┐         │
│  │                                        │         │
│  │   ┌────────────────────────────────────▼───────┐ │
│  │   │  PRESET BROWSER                            │ │
│  │   │  ┌─────────────────────────────────────┐   │ │
│  │   │  │ [←] Categories    [🔍] [★] [🎲]     │   │ │
│  │   │  ├─────────────────────────────────────┤   │ │
│  │   │  │  ★ Favorites           (47)         │   │ │
│  │   │  │  ∿ Waveform          (1,279)        │   │ │
│  │   │  │  ◎ Reaction          (1,791)        │   │ │
│  │   │  │  △ Geometric         (1,027)        │   │ │
│  │   │  │  ✦ Fractal           (1,354)        │   │ │
│  │   │  │  💃 Dancer            (1,351)        │   │ │
│  │   │  │  ✎ Drawing           (1,143)        │   │ │
│  │   │  │  ○ Hypnotic            (280)        │   │ │
│  │   │  │  ⋮ Particles           (389)        │   │ │
│  │   │  │  ✨ Sparkle             (797)        │   │ │
│  │   │  │  ☀ Supernova           (380)        │   │ │
│  │   │  │  ── All Presets      (9,795)        │   │ │
│  │   │  └─────────────────────────────────────┘   │ │
│  │   └────────────────────────────────────────────┘ │
│  ├── Settings                                       │
│  └── Exit                                           │
└─────────────────────────────────────────────────────┘
```

### 4.2 Category View (After Selection)

```
┌─────────────────────────────────────────────────────┐
│  ◎ REACTION                          1,791 presets  │
├─────────────────────────────────────────────────────┤
│  [← Back]    [🔍 Search...]    [★ Fav] [🎲 Random]  │
├─────────────────────────────────────────────────────┤
│  ★  Flexi - collapse theory                         │
│     cope - drove through ghosts to get here         │
│  ★  ORB - Space Plasma                              │
│     martin - foggy notion                           │
│     suksma - passive probabilistic roaming...       │
│     Rozzor & Rovastar - Dynamic Swirls 3...         │
│     amandio c - the green machine 2 skin...         │
│     flexi - what is the matrix                      │
│     ...                                             │
│                                                     │
│  ──────────────────────────────────────────────────│
│  │◀  47/1791  ▶│  Use ↑↓ to browse, Enter to play  │
└─────────────────────────────────────────────────────┘
```

### 4.3 Header Bar Controls

| Control      | Icon | Action                                |
|--------------|------|---------------------------------------|
| Back         | `←`  | Return to category list               |
| Search       | `🔍` | Toggle search bar                     |
| Favorites    | `★`  | Toggle "favorites only" filter        |
| Random       | `🎲` | Play random preset from category      |
| Shuffle      | `⟳`  | Toggle shuffle within category        |

### 4.4 Keyboard/Gamepad Navigation

| Input            | Action                                   |
|------------------|------------------------------------------|
| Up/Down          | Navigate preset/category list            |
| Enter/A          | Select category or play preset           |
| Escape/B         | Go back one level                        |
| Tab              | Cycle between categories/presets panel   |
| Left/Right       | Page up/down in long lists               |
| `/` or `Ctrl+F`  | Focus search bar                         |
| `F`              | Toggle favorite on selected preset       |
| `R`              | Random preset in current view            |
| `Space`          | Preview preset (soft cut)                |

---

## 5. Implementation Strategy

### 5.1 Loading Strategy: Hybrid Approach

**Recommendation: Scan on Startup + Cache**

```
Startup Flow:
1. Check for category_cache.json
2. If cache exists and is recent (< 24h):
   - Load from cache (fast)
   - Background: verify folder mtimes match
3. If cache missing or stale:
   - Scan directories (1-2 seconds for 10K files)
   - Build indices
   - Save cache
```

**Cache Format (category_cache.json):**
```json
{
  "version": 2,
  "generated": "2025-01-15T12:00:00Z",
  "rootPath": "F:/chilltittiesvisualizer/presets-cream-of-the-crop",
  "categories": [
    {
      "id": "reaction",
      "name": "Reaction",
      "folderName": "Reaction",
      "count": 1791,
      "mtime": 1705312800
    }
  ],
  "presets": [
    {
      "path": "Reaction/flexi - collapse theory.milk",
      "name": "flexi - collapse theory",
      "author": "flexi",
      "category": "reaction",
      "tags": 0
    }
  ],
  "favorites": [0, 47, 123, 456]
}
```

### 5.2 Parsing Algorithm

```cpp
PresetEntry parsePresetFilename(const fs::path& filepath) {
    PresetEntry entry;
    entry.fullPath = filepath.string();
    
    // Extract category from parent folder
    entry.category = categoryFromFolder(filepath.parent_path().filename().string());
    
    // Get filename without extension
    std::string name = filepath.stem().string();
    entry.displayName = name;
    
    // Parse author (text before first " - ")
    size_t dashPos = name.find(" - ");
    if (dashPos != std::string::npos) {
        entry.author = name.substr(0, dashPos);
        entry.displayName = name.substr(dashPos + 3);
    }
    
    // Detect tags
    entry.tags = 0;
    if (name.find(" + ") != std::string::npos || 
        name.find(", ") != std::string::npos) {
        entry.tags |= TAG_COLLAB;
    }
    if (containsIgnoreCase(name, "mashup")) entry.tags |= TAG_MASHUP;
    if (containsIgnoreCase(name, "remix") || 
        containsIgnoreCase(name, "mix") ||
        containsIgnoreCase(name, "edit")) entry.tags |= TAG_REMIX;
    if (name.find("nz+") != std::string::npos || 
        name.find(" nz") != std::string::npos) entry.tags |= TAG_NOISE_VAR;
    if (containsIgnoreCase(name, "roam")) entry.tags |= TAG_ROAMING;
    if (containsIgnoreCase(name, "jelly")) entry.tags |= TAG_JELLY;
    if (containsIgnoreCase(name, "isosceles")) entry.tags |= TAG_ISOSCELES;
    
    return entry;
}
```

### 5.3 File Structure

```
src/
├── preset_database.h       // PresetDatabase, PresetEntry, CategoryStack
├── preset_database.cpp     // Loading, parsing, indexing logic
├── preset_manager.h        // (existing) projectM playlist wrapper
├── preset_manager.cpp      // (existing)
├── menu_overlay.h          // (modify) Add category browser state
├── menu_overlay.cpp        // (modify) Add renderCategoryBrowser()
├── config.h                // (modify) Add lastCategory, categoryCache
└── config.cpp              // (modify) Persist category preferences
```

### 5.4 Integration with Existing Code

**menu_overlay.h additions:**
```cpp
enum class UIScreen {
    // ... existing ...
    CategoryBrowser,    // NEW: Category selection screen
    PresetBrowser,      // Modify: now shows presets within a category
};

class MenuOverlay {
    // ... existing ...
    
    // Category browser state
    PresetDatabase m_presetDb;
    PresetCategory m_currentCategory = PresetCategory::All;
    int m_selectedCategoryIndex = 0;
    
    // Screen renderers
    MenuAction renderCategoryBrowser();    // NEW
    MenuAction renderPresetBrowser();      // MODIFY: filter by category
};
```

**config.h additions:**
```cpp
struct VibeusConfig {
    // ... existing ...
    
    // Category preferences
    std::string lastCategory = "All";
    bool rememberCategory = true;
    bool showPresetCount = true;
    bool compactCategoryView = false;
};
```

---

## 6. User Preferences & Persistence

### 6.1 Category-Related Settings

| Setting             | Type    | Default | Description                       |
|---------------------|---------|---------|-----------------------------------|
| lastCategory        | string  | "All"   | Last selected category            |
| rememberCategory    | bool    | true    | Restore last category on launch   |
| favoriteCategories  | array   | []      | Pinned categories at top          |
| hiddenCategories    | array   | []      | Categories to hide from list      |
| showPresetCount     | bool    | true    | Show "(N)" count beside category  |
| compactCategoryView | bool    | false   | Single-line category entries      |

### 6.2 Per-Category State

```json
// vibeus_config.json additions
{
  "categories": {
    "reaction": {
      "shuffle": true,
      "lastPresetIndex": 456,
      "sortOrder": "name"
    },
    "fractal": {
      "shuffle": false,
      "lastPresetIndex": 12,
      "sortOrder": "recent"
    }
  }
}
```

---

## 7. Advanced Features (Future)

### 7.1 Smart Playlists

User-definable filters:
- "Night Mode" = Supernova + Sparkle + Hypnotic
- "Bass Drops" = Reaction where tags include NOISE_VAR
- "Martin's Best" = author == "martin" && isFavorite

### 7.2 Auto-Categorization Improvements

Machine learning potential:
- Analyze preset shader code for visual characteristics
- Cluster by color palette, motion type, complexity
- Community tagging integration

### 7.3 Category Statistics

Track per-category usage:
- Play count
- Average duration before skip
- Favorite ratio
- Most active time of day

---

## 8. Implementation Phases

### Phase 1: Core Category System (MVP)
- [ ] Create `PresetDatabase` class
- [ ] Scan directories into category stacks
- [ ] Add `CategoryBrowser` screen to menu
- [ ] Modify `PresetBrowser` to filter by category
- [ ] Persist last category in config

### Phase 2: Enhanced UX
- [ ] Add category icons/emoji
- [ ] Random-from-category feature
- [ ] Keyboard shortcuts for quick category jump
- [ ] "Play all in category" mode

### Phase 3: Author & Tag System
- [ ] Parse author names from filenames
- [ ] Implement tag detection
- [ ] Add author filter dropdown
- [ ] Tag-based search (e.g., `tag:mashup`)

### Phase 4: Smart Features
- [ ] Favorite categories
- [ ] Hidden categories
- [ ] Category-specific shuffle/loop settings
- [ ] Usage statistics

---

## 9. Summary

The categorization system leverages the existing folder structure as primary categories, enhancing it with:

1. **Natural Navigation**: Stack-based flow (categories → presets)
2. **Zero Configuration**: Auto-detects categories from folders
3. **Rich Metadata**: Author parsing and tag detection
4. **Familiar UX**: Maintains existing favorites and search
5. **Future-Proof**: Extensible for smart playlists and ML features

This design minimizes implementation effort while significantly improving preset discovery for the 9,800+ preset collection.
