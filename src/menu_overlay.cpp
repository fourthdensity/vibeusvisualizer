#include "menu_overlay.h"

#include <imgui.h>
#include "imgui_impl_sdl2.h"
#include <imgui_impl_opengl3.h>

#include <algorithm>
#include <fstream>
#include <cstring>
#include <cmath>

// ─── Init / Shutdown ───────────────────────────────────────────────

bool MenuOverlay::init(SDL_Window* window, SDL_GLContext glContext)
{
    m_window = window;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.IniFilename = nullptr;

    applyStyle();

    ImGui_ImplSDL2_InitForOpenGL(window, glContext);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    return true;
}

void MenuOverlay::shutdown()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}

// ─── Screen Management ─────────────────────────────────────────────

void MenuOverlay::showScreen(UIScreen screen)
{
    m_screen = screen;
    if (screen == UIScreen::Splash)
        m_splashStart = SDL_GetTicks();
}

void MenuOverlay::hideAll()
{
    m_screen = UIScreen::None;
}

void MenuOverlay::processEvent(const SDL_Event& event)
{
    ImGui_ImplSDL2_ProcessEvent(&event);
}

// ─── Render Dispatch ────────────────────────────────────────────────

MenuAction MenuOverlay::render()
{
    if (m_screen == UIScreen::None)
        return MenuAction::None;

    beginFrame();

    MenuAction action = MenuAction::None;
    switch (m_screen) {
    case UIScreen::Splash:        action = renderSplash(); break;
    case UIScreen::MainMenu:      action = renderMainMenu(); break;
    case UIScreen::PauseMenu:     action = renderPauseMenu(); break;
    case UIScreen::PresetBrowser: action = renderPresetBrowser(); break;
    default: break;
    }

    endFrame();
    return action;
}

// ─── Frame Helpers ──────────────────────────────────────────────────

void MenuOverlay::beginFrame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
}

void MenuOverlay::endFrame()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void MenuOverlay::drawBackdrop(float alpha)
{
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 ds = io.DisplaySize;

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ds);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("##backdrop", nullptr,
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoInputs);

    auto* dl = ImGui::GetWindowDrawList();
    int a = static_cast<int>(alpha * 255.0f);
    dl->AddRectFilled(ImVec2(0, 0), ds, IM_COL32(4, 4, 16, a));
    // Vignette
    dl->AddRectFilledMultiColor(ImVec2(0, 0), ds,
        IM_COL32(0, 0, 0, a / 2), IM_COL32(0, 0, 0, a / 2),
        IM_COL32(0, 0, 0, a / 5), IM_COL32(0, 0, 0, a / 5));

    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
}

void MenuOverlay::drawCenteredTitle(const char* title, const char* subtitle)
{
    ImVec2 winSize = ImGui::GetWindowSize();

    if (title) {
        ImVec2 ts = ImGui::CalcTextSize(title);
        ImGui::SetCursorPosX((winSize.x - ts.x) * 0.5f);
        ImGui::SetCursorPosY(28);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.65f, 0.75f, 1.0f, 0.95f));
        ImGui::TextUnformatted(title);
        ImGui::PopStyleColor();
    }

    if (subtitle) {
        ImVec2 ss = ImGui::CalcTextSize(subtitle);
        ImGui::SetCursorPosX((winSize.x - ss.x) * 0.5f);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.45f, 0.45f, 0.55f, 0.75f));
        ImGui::TextUnformatted(subtitle);
        ImGui::PopStyleColor();
    }
}

// ─── Splash Screen (Epilepsy Warning) ──────────────────────────────

MenuAction MenuOverlay::renderSplash()
{
    drawBackdrop(0.95f);

    ImGuiIO& io = ImGui::GetIO();
    ImVec2 ds = io.DisplaySize;
    ImVec2 panelSize(520, 400);
    ImVec2 panelPos((ds.x - panelSize.x) * 0.5f, (ds.y - panelSize.y) * 0.5f);

    ImGui::SetNextWindowPos(panelPos);
    ImGui::SetNextWindowSize(panelSize);
    ImGui::Begin("##splash", nullptr,
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoResize);

    float w = panelSize.x;
    float pad = ImGui::GetStyle().WindowPadding.x;
    float contentW = w - pad * 2.0f;

    // Title
    {
        const char* t = "V I B E U S";
        ImVec2 ts = ImGui::CalcTextSize(t);
        ImGui::SetCursorPosX((w - ts.x) * 0.5f);
        ImGui::SetCursorPosY(24);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.65f, 0.75f, 1.0f, 0.95f));
        ImGui::TextUnformatted(t);
        ImGui::PopStyleColor();
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::Spacing();

    // Warning icon + text
    {
        // Pulsing warning color
        float elapsed = (SDL_GetTicks() - m_splashStart) / 1000.0f;
        float pulse = 0.7f + 0.3f * sinf(elapsed * 2.5f);

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.85f, 0.3f, pulse));
        const char* warn = "   PHOTOSENSITIVITY WARNING";
        ImVec2 ws = ImGui::CalcTextSize(warn);
        ImGui::SetCursorPosX((w - ws.x) * 0.5f);
        ImGui::TextUnformatted(warn);
        ImGui::PopStyleColor();
    }

    ImGui::Spacing();
    ImGui::Spacing();

    // Warning body text
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.78f, 0.78f, 0.85f, 0.9f));
    ImGui::PushTextWrapPos(pad + contentW);
    ImGui::SetCursorPosX(pad);
    ImGui::TextWrapped(
        "This application displays rapidly flashing lights, colors, "
        "and patterns that may trigger seizures in people with "
        "photosensitive epilepsy.\n\n"
        "If you or anyone in your household has a history of "
        "epilepsy or seizures, please consult a physician before "
        "using this software.\n\n"
        "If you experience dizziness, altered vision, eye or muscle "
        "twitching, disorientation, or any involuntary movement, "
        "immediately stop and consult a doctor.");
    ImGui::PopTextWrapPos();
    ImGui::PopStyleColor();

    ImGui::Spacing();
    ImGui::Spacing();

    // "I Understand" button
    float btnH = 48.0f;
    ImGui::SetCursorPosX(pad);
    if (ImGui::Button("  I Understand - Continue", ImVec2(contentW, btnH))) {
        return MenuAction::BackToMenu;
    }

    // Also allow Enter/Space to dismiss
    if (ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_Space)) {
        return MenuAction::BackToMenu;
    }

    ImGui::End();
    return MenuAction::None;
}

// ─── Main Menu ──────────────────────────────────────────────────────

MenuAction MenuOverlay::renderMainMenu()
{
    drawBackdrop(0.92f);

    ImGuiIO& io = ImGui::GetIO();
    ImVec2 ds = io.DisplaySize;
    ImVec2 menuSize(380, 480);
    ImVec2 menuPos((ds.x - menuSize.x) * 0.5f, (ds.y - menuSize.y) * 0.5f);

    ImGui::SetNextWindowPos(menuPos);
    ImGui::SetNextWindowSize(menuSize);
    ImGui::Begin("##mainmenu", nullptr,
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoResize);

    float w = menuSize.x;
    float pad = ImGui::GetStyle().WindowPadding.x;
    float contentW = w - pad * 2.0f;
    float btnH = 52.0f;

    drawCenteredTitle("V I B E U S", nullptr);

    // Version subtitle
    {
        const char* ver = "v 0 . 2 . 0";
        ImVec2 vs = ImGui::CalcTextSize(ver);
        ImGui::SetCursorPosX((w - vs.x) * 0.5f);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.4f, 0.52f, 0.6f));
        ImGui::TextUnformatted(ver);
        ImGui::PopStyleColor();
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::Spacing();

    MenuAction action = MenuAction::None;

    // Start Visualizer
    ImGui::SetCursorPosX(pad);
    if (ImGui::Button("  Start Visualizer", ImVec2(contentW, btnH)))
        action = MenuAction::StartVisualizer;

    ImGui::Spacing();

    // Browse Presets
    ImGui::SetCursorPosX(pad);
    if (ImGui::Button("  Browse Presets", ImVec2(contentW, btnH))) {
        m_browserReturnTo = UIScreen::MainMenu;
        action = MenuAction::BrowsePresets;
    }

    ImGui::Spacing();

    // Settings
    ImGui::SetCursorPosX(pad);
    if (ImGui::Button("  Settings", ImVec2(contentW, btnH)))
        action = MenuAction::Settings;

    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Exit
    ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.45f, 0.08f, 0.08f, 0.65f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered,  ImVec4(0.65f, 0.12f, 0.12f, 0.80f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,   ImVec4(0.75f, 0.18f, 0.18f, 0.95f));
    ImGui::SetCursorPosX(pad);
    if (ImGui::Button("  Exit to Desktop", ImVec2(contentW, btnH)))
        action = MenuAction::ExitToDesktop;
    ImGui::PopStyleColor(3);

    ImGui::End();
    return action;
}

// ─── Pause Menu ─────────────────────────────────────────────────────

MenuAction MenuOverlay::renderPauseMenu()
{
    drawBackdrop(0.6f);

    ImGuiIO& io = ImGui::GetIO();
    ImVec2 ds = io.DisplaySize;
    ImVec2 menuSize(380, 480);
    ImVec2 menuPos((ds.x - menuSize.x) * 0.5f, (ds.y - menuSize.y) * 0.5f);

    ImGui::SetNextWindowPos(menuPos);
    ImGui::SetNextWindowSize(menuSize);
    ImGui::Begin("##pausemenu", nullptr,
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoResize);

    float w = menuSize.x;
    float pad = ImGui::GetStyle().WindowPadding.x;
    float contentW = w - pad * 2.0f;
    float btnH = 52.0f;

    drawCenteredTitle("V I B E U S", "P A U S E D");

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::Spacing();

    MenuAction action = MenuAction::None;

    // Resume
    ImGui::SetCursorPosX(pad);
    if (ImGui::Button("  Resume", ImVec2(contentW, btnH)))
        action = MenuAction::Resume;

    ImGui::Spacing();

    // Browse Presets
    ImGui::SetCursorPosX(pad);
    if (ImGui::Button("  Browse Presets", ImVec2(contentW, btnH))) {
        m_browserReturnTo = UIScreen::PauseMenu;
        action = MenuAction::BrowsePresets;
    }

    ImGui::Spacing();

    // Settings
    ImGui::SetCursorPosX(pad);
    if (ImGui::Button("  Settings", ImVec2(contentW, btnH)))
        action = MenuAction::Settings;

    ImGui::Spacing();

    // Record
    ImGui::SetCursorPosX(pad);
    if (ImGui::Button("  Record", ImVec2(contentW, btnH)))
        action = MenuAction::Record;

    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Exit
    ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.45f, 0.08f, 0.08f, 0.65f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered,  ImVec4(0.65f, 0.12f, 0.12f, 0.80f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,   ImVec4(0.75f, 0.18f, 0.18f, 0.95f));
    ImGui::SetCursorPosX(pad);
    if (ImGui::Button("  Exit to Desktop", ImVec2(contentW, btnH)))
        action = MenuAction::ExitToDesktop;
    ImGui::PopStyleColor(3);

    ImGui::End();
    return action;
}

// ─── Preset Browser ─────────────────────────────────────────────────

void MenuOverlay::loadPresetList(projectm_playlist_handle playlist)
{
    m_presetList.clear();
    if (!playlist) return;

    uint32_t count = projectm_playlist_size(playlist);
    m_presetList.reserve(count);

    // Fetch all preset paths
    char** items = projectm_playlist_items(playlist, 0, count);
    if (!items) return;

    for (uint32_t i = 0; items[i] != nullptr; i++) {
        PresetEntry entry;
        entry.filename = items[i];

        // Extract display name (filename without path and extension)
        std::string name = entry.filename;
        auto slash = name.find_last_of("/\\");
        if (slash != std::string::npos)
            name = name.substr(slash + 1);
        auto dot = name.rfind(".milk");
        if (dot != std::string::npos)
            name = name.substr(0, dot);
        entry.displayName = name;

        m_presetList.push_back(std::move(entry));
    }

    projectm_playlist_free_string_array(items);
}

MenuAction MenuOverlay::renderPresetBrowser()
{
    drawBackdrop(0.92f);

    ImGuiIO& io = ImGui::GetIO();
    ImVec2 ds = io.DisplaySize;

    // Browser takes up most of the screen
    float margin = 60.0f;
    ImVec2 panelSize(ds.x - margin * 2.0f, ds.y - margin * 2.0f);
    if (panelSize.x < 400) panelSize.x = 400;
    if (panelSize.y < 300) panelSize.y = 300;
    ImVec2 panelPos((ds.x - panelSize.x) * 0.5f, (ds.y - panelSize.y) * 0.5f);

    ImGui::SetNextWindowPos(panelPos);
    ImGui::SetNextWindowSize(panelSize);
    ImGui::Begin("##browser", nullptr,
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoResize);

    float pad = ImGui::GetStyle().WindowPadding.x;
    float contentW = panelSize.x - pad * 2.0f;

    MenuAction action = MenuAction::None;

    // Title row with back button
    if (ImGui::Button("< Back", ImVec2(80, 36))) {
        if (m_browserReturnTo == UIScreen::PauseMenu)
            action = MenuAction::BackToPause;
        else
            action = MenuAction::BackToMenu;
    }
    ImGui::SameLine();
    {
        const char* t = "P R E S E T   B R O W S E R";
        ImVec2 ts = ImGui::CalcTextSize(t);
        ImGui::SetCursorPosX((panelSize.x - ts.x) * 0.5f);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.65f, 0.75f, 1.0f, 0.95f));
        ImGui::TextUnformatted(t);
        ImGui::PopStyleColor();
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Search bar + favorites filter
    ImGui::SetCursorPosX(pad);
    ImGui::SetNextItemWidth(contentW - 180);
    ImGui::InputTextWithHint("##search", "Search presets...", m_searchBuf, sizeof(m_searchBuf));
    ImGui::SameLine();
    ImGui::Checkbox("Favorites Only", &m_showFavoritesOnly);

    ImGui::Spacing();

    // Stats
    {
        uint32_t totalCount = static_cast<uint32_t>(m_presetList.size());
        uint32_t favCount = static_cast<uint32_t>(m_favorites.size());
        char stats[128];
        snprintf(stats, sizeof(stats), "%u presets | %u favorites", totalCount, favCount);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.45f, 0.45f, 0.55f, 0.75f));
        ImGui::TextUnformatted(stats);
        ImGui::PopStyleColor();
    }

    ImGui::Spacing();

    // Scrollable preset list
    float listH = ImGui::GetContentRegionAvail().y - 10.0f;
    ImGui::BeginChild("##presetList", ImVec2(contentW, listH), true);

    std::string searchLower;
    if (m_searchBuf[0]) {
        searchLower = m_searchBuf;
        std::transform(searchLower.begin(), searchLower.end(), searchLower.begin(), ::tolower);
    }

    for (uint32_t i = 0; i < static_cast<uint32_t>(m_presetList.size()); i++) {
        const auto& entry = m_presetList[i];

        // Filter: favorites only
        if (m_showFavoritesOnly && !isFavorite(i))
            continue;

        // Filter: search
        if (!searchLower.empty()) {
            std::string nameLower = entry.displayName;
            std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);
            if (nameLower.find(searchLower) == std::string::npos)
                continue;
        }

        ImGui::PushID(static_cast<int>(i));

        // Star button for favorites
        bool fav = isFavorite(i);
        if (fav)
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.85f, 0.2f, 1.0f));
        else
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.35f, 0.35f, 0.45f, 0.6f));

        if (ImGui::SmallButton(fav ? "*" : " ")) {
            toggleFavorite(i);
        }
        ImGui::PopStyleColor();

        ImGui::SameLine();

        // Preset name — clickable
        bool selected = (m_selectedPreset == i);
        if (ImGui::Selectable(entry.displayName.c_str(), selected, ImGuiSelectableFlags_AllowDoubleClick)) {
            m_selectedPreset = i;
            if (ImGui::IsMouseDoubleClicked(0)) {
                action = MenuAction::PlayPreset;
            }
        }

        ImGui::PopID();
    }

    ImGui::EndChild();
    ImGui::End();

    return action;
}

// ─── Favorites ──────────────────────────────────────────────────────

void MenuOverlay::toggleFavorite(uint32_t idx)
{
    if (m_favorites.count(idx))
        m_favorites.erase(idx);
    else
        m_favorites.insert(idx);
}

void MenuOverlay::saveFavorites(const std::string& path)
{
    std::ofstream f(path);
    if (!f.is_open()) return;

    for (uint32_t idx : m_favorites) {
        if (idx < m_presetList.size())
            f << m_presetList[idx].filename << "\n";
    }
}

void MenuOverlay::loadFavorites(const std::string& path)
{
    m_favorites.clear();
    std::ifstream f(path);
    if (!f.is_open()) return;

    std::string line;
    while (std::getline(f, line)) {
        if (line.empty()) continue;
        for (uint32_t i = 0; i < static_cast<uint32_t>(m_presetList.size()); i++) {
            if (m_presetList[i].filename == line) {
                m_favorites.insert(i);
                break;
            }
        }
    }
}

// ─── Style ──────────────────────────────────────────────────────────

void MenuOverlay::applyStyle()
{
    ImGuiStyle& style = ImGui::GetStyle();
    ImGuiIO& io = ImGui::GetIO();

    io.FontGlobalScale = 1.35f;

    style.WindowRounding    = 16.0f;
    style.FrameRounding     = 10.0f;
    style.PopupRounding     = 10.0f;
    style.ScrollbarRounding = 10.0f;
    style.GrabRounding      = 8.0f;

    style.WindowPadding = ImVec2(24, 24);
    style.FramePadding  = ImVec2(18, 12);
    style.ItemSpacing   = ImVec2(10, 6);

    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize  = 0.0f;

    ImVec4* c = style.Colors;

    c[ImGuiCol_WindowBg] = ImVec4(0.05f, 0.05f, 0.11f, 0.92f);
    c[ImGuiCol_ChildBg]  = ImVec4(0.04f, 0.04f, 0.09f, 0.60f);
    c[ImGuiCol_Border]   = ImVec4(0.45f, 0.55f, 1.0f, 0.14f);

    c[ImGuiCol_Button]        = ImVec4(0.12f, 0.13f, 0.24f, 0.60f);
    c[ImGuiCol_ButtonHovered] = ImVec4(0.20f, 0.22f, 0.42f, 0.78f);
    c[ImGuiCol_ButtonActive]  = ImVec4(0.28f, 0.30f, 0.52f, 0.92f);

    c[ImGuiCol_Header]        = ImVec4(0.14f, 0.15f, 0.28f, 0.55f);
    c[ImGuiCol_HeaderHovered] = ImVec4(0.22f, 0.24f, 0.44f, 0.70f);
    c[ImGuiCol_HeaderActive]  = ImVec4(0.28f, 0.30f, 0.52f, 0.85f);

    c[ImGuiCol_FrameBg]        = ImVec4(0.08f, 0.08f, 0.16f, 0.70f);
    c[ImGuiCol_FrameBgHovered] = ImVec4(0.12f, 0.13f, 0.24f, 0.80f);
    c[ImGuiCol_FrameBgActive]  = ImVec4(0.16f, 0.17f, 0.30f, 0.90f);

    c[ImGuiCol_Text]         = ImVec4(0.92f, 0.93f, 0.98f, 1.0f);
    c[ImGuiCol_TextDisabled] = ImVec4(0.38f, 0.38f, 0.48f, 1.0f);

    c[ImGuiCol_Separator] = ImVec4(0.45f, 0.55f, 1.0f, 0.10f);

    c[ImGuiCol_ScrollbarBg]          = ImVec4(0.04f, 0.04f, 0.08f, 0.5f);
    c[ImGuiCol_ScrollbarGrab]        = ImVec4(0.18f, 0.18f, 0.28f, 0.7f);
    c[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.25f, 0.25f, 0.40f, 0.8f);
    c[ImGuiCol_ScrollbarGrabActive]  = ImVec4(0.32f, 0.32f, 0.50f, 0.9f);

    c[ImGuiCol_NavHighlight] = ImVec4(0.35f, 0.45f, 0.85f, 0.8f);

    c[ImGuiCol_CheckMark] = ImVec4(0.55f, 0.65f, 1.0f, 0.9f);
}
