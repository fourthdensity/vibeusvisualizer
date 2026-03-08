#pragma once

#include <SDL.h>
#include <projectM-4/playlist.h>

#include "config.h"

#include <string>
#include <vector>
#include <set>
#include <cstdint>
#include <deque>

// Which screen the UI is showing
enum class UIScreen {
    None,           // UI hidden (visualizer running)
    Splash,         // Epilepsy warning on first launch
    MainMenu,       // Main menu (Start, Browse, Settings, Exit)
    PauseMenu,      // Pause overlay during visualization
    PresetBrowser,  // Browse / search / favorite presets
    Settings,       // Settings panel (Basic / Advanced tabs)
};

// Actions the UI can request from the app
enum class MenuAction {
    None,
    StartVisualizer,
    Resume,
    Settings,
    Record,
    ExitToDesktop,
    // Preset browser actions
    BrowsePresets,
    PlayPreset,       // selected preset index in selectedPresetIndex()
    BackToMenu,
    BackToPause,
    ApplySettings,
    BackFromSettings,
};

class MenuOverlay {
public:
    bool init(SDL_Window* window, SDL_GLContext glContext);
    void shutdown();

    // Screen management
    void showScreen(UIScreen screen);
    void hideAll();
    UIScreen currentScreen() const { return m_screen; }
    bool isVisible() const { return m_screen != UIScreen::None; }

    // Forward SDL events to ImGui. Call only when UI is visible.
    void processEvent(const SDL_Event& event);

    // Render the current screen. Returns any action selected this frame.
    MenuAction render();

    // Preset browser: load preset list from playlist manager
    void loadPresetList(projectm_playlist_handle playlist);

    // Preset browser: get the selected preset index (valid after PlayPreset action)
    uint32_t selectedPresetIndex() const { return m_selectedPreset; }

    // Favorites management
    const std::set<uint32_t>& favorites() const { return m_favorites; }
    bool isFavorite(uint32_t idx) const { return m_favorites.count(idx) > 0; }
    void toggleFavorite(uint32_t idx);

    // Save/load favorites to disk
    void saveFavorites(const std::string& path);
    void loadFavorites(const std::string& path);

    // Settings: point at the app's config struct (non-owning)
    void setConfigPtr(VibeusConfig* cfg) { m_config = cfg; }
    void setSettingsReturnScreen(UIScreen screen) { m_settingsReturnTo = screen; }
    UIScreen settingsReturnScreen() const { return m_settingsReturnTo; }

    // Toast notifications — brief confirmations shown over the visualizer
    void showToast(const char* message, float durationSec = 2.0f);
    void renderToasts(); // call every frame (even when UI is hidden)

private:
    UIScreen m_screen = UIScreen::None;
    SDL_Window* m_window = nullptr;
    float m_splashTimer = 0.0f;
    Uint32 m_splashStart = 0;

    // Preset browser state
    struct PresetEntry {
        std::string filename;   // full path
        std::string displayName; // just the name
    };
    std::vector<PresetEntry> m_presetList;
    char m_searchBuf[256] = {};
    uint32_t m_selectedPreset = 0;
    std::set<uint32_t> m_favorites;
    bool m_showFavoritesOnly = false;
    UIScreen m_browserReturnTo = UIScreen::MainMenu; // where to go back to

    // Settings state
    VibeusConfig* m_config = nullptr;
    int m_settingsTab = 0;                           // 0=Basic, 1=Advanced
    UIScreen m_settingsReturnTo = UIScreen::MainMenu;

    // Toast state
    struct Toast {
        std::string message;
        Uint32 startMs;
        float durationSec;
    };
    std::deque<Toast> m_toasts;

    void applyStyle();

    // Screen renderers
    MenuAction renderSplash();
    MenuAction renderMainMenu();
    MenuAction renderPauseMenu();
    MenuAction renderPresetBrowser();
    MenuAction renderSettings();

    // Shared helpers
    void beginFrame();
    void endFrame();
    void drawBackdrop(float alpha = 0.9f);
    void drawCenteredTitle(const char* title, const char* subtitle = nullptr);
};
