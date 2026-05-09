#include "menu_overlay.h"

#include <imgui.h>
#include "imgui_impl_sdl2.h"
#include <imgui_impl_opengl3.h>

#include <algorithm>
#include <fstream>
#include <cstring>
#include <cmath>

namespace {

constexpr const char* kTransitionStyles[] = {
    "Cycling (Auto)",
    "Flash Cut",
    "Slow Morph",
    "Quick Blend",
    "Glitch Cut",
    "Zoom Burst",
    "Energy Flash",
    "Snap Fade",
    "Long Dissolve",
    "Pulse Blend",
    "Bass Slam",
    "Breathing Fade",
    "Clean Slate",
    "Liquid Drift",
    "Ambient Wash",
    "Spark Jump",
    "Deep Bloom",
    "Afterimage",
    "Drop Smash"
};

constexpr int kTransitionStyleCount = sizeof(kTransitionStyles) / sizeof(kTransitionStyles[0]);

void applyTransitionStyleDefaults(VibeusConfig& cfg)
{
    switch (cfg.storyTransitionStyle) {
    case 0: // Cycling (Auto)
        cfg.hardCutEnabled = true;
        cfg.transitionTime = 2.8f;
        cfg.hardCutSensitivity = 2.0f;
        break;
    case 1: // Flash Cut
    case 7: // Snap Fade
    case 18: // Drop Smash
        cfg.hardCutEnabled = true;
        cfg.transitionTime = 0.3f;
        cfg.hardCutSensitivity = 1.5f;
        break;
    case 2: // Slow Morph
    case 8: // Long Dissolve
    case 11: // Breathing Fade
    case 14: // Ambient Wash
        cfg.hardCutEnabled = false;
        cfg.transitionTime = 6.0f;
        cfg.hardCutSensitivity = 2.8f;
        break;
    case 3: // Quick Blend
    case 9: // Pulse Blend
    case 15: // Spark Jump
        cfg.hardCutEnabled = true;
        cfg.transitionTime = 1.2f;
        cfg.hardCutSensitivity = 1.9f;
        break;
    case 4: // Glitch Cut
    case 10: // Bass Slam
    case 12: // Clean Slate
        cfg.hardCutEnabled = true;
        cfg.transitionTime = 0.6f;
        cfg.hardCutSensitivity = 1.4f;
        break;
    case 5: // Zoom Burst
    case 13: // Liquid Drift
    case 16: // Deep Bloom
    case 17: // Afterimage
        cfg.hardCutEnabled = false;
        cfg.transitionTime = 3.8f;
        cfg.hardCutSensitivity = 2.2f;
        break;
    case 6: // Energy Flash
        cfg.hardCutEnabled = true;
        cfg.transitionTime = 0.8f;
        cfg.hardCutSensitivity = 1.7f;
        break;
    default:
        break;
    }
}

} // namespace

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

void MenuOverlay::setLiveStatus(const std::string& presetName, const std::string& storyState,
                                float beatSens, bool stasisActive)
{
    m_livePresetName = presetName.empty() ? "—" : presetName;
    m_liveStoryState = storyState.empty() ? "CHILL" : storyState;
    m_liveBeatSens   = beatSens;
    m_liveStasis     = stasisActive;
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
    case UIScreen::Settings:      action = renderSettings(); break;
    default: break;
    }

    // Don't end frame yet — toasts may still draw into this frame
    m_frameActive = true;
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
    m_frameActive = false;
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

// ─── Toast Notifications ────────────────────────────────────────────

void MenuOverlay::showToast(const char* message, float durationSec)
{
    m_toasts.push_back({ message, SDL_GetTicks(), durationSec });
    // Keep at most 4 toasts visible
    while (m_toasts.size() > 4)
        m_toasts.pop_front();
}

void MenuOverlay::setRemappingControl(const char* controlName, int* bindingPtr, bool isGamepad)
{
    m_remappingActive = true;
    m_remappingControl = controlName;
    m_remappingBindingPtr = bindingPtr;
    m_remappingIsGamepad = isGamepad;
}

void MenuOverlay::renderToasts()
{
    if (m_toasts.empty()) {
        // If render() left a frame open with no toasts to draw, close it now
        if (m_frameActive)
            endFrame();
        return;
    }

    // Start an ImGui frame only if one isn't already active
    bool needFrame = !m_frameActive;
    if (needFrame) {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
    }

    ImGuiIO& io = ImGui::GetIO();
    float yOffset = io.DisplaySize.y - 60.0f;
    Uint32 now = SDL_GetTicks();

    for (auto it = m_toasts.begin(); it != m_toasts.end(); ) {
        float elapsed = (now - it->startMs) / 1000.0f;
        if (elapsed >= it->durationSec) {
            it = m_toasts.erase(it);
            continue;
        }

        // Fade in (first 0.2s) and out (last 0.4s)
        float alpha = 1.0f;
        if (elapsed < 0.2f)
            alpha = elapsed / 0.2f;
        else if (elapsed > it->durationSec - 0.4f)
            alpha = (it->durationSec - elapsed) / 0.4f;

        ImVec2 textSize = ImGui::CalcTextSize(it->message.c_str());
        float padX = 20.0f, padY = 10.0f;
        float boxW = textSize.x + padX * 2;
        float boxH = textSize.y + padY * 2;
        float boxX = (io.DisplaySize.x - boxW) * 0.5f;

        ImGui::SetNextWindowPos(ImVec2(boxX, yOffset - boxH));
        ImGui::SetNextWindowSize(ImVec2(boxW, boxH));
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.08f, 0.08f, 0.18f, 0.85f * alpha));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.45f, 0.55f, 1.0f, 0.3f * alpha));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 12.0f);

        char winId[64];
        snprintf(winId, sizeof(winId), "##toast_%u", it->startMs);
        ImGui::Begin(winId, nullptr,
            ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoInputs |
            ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus);

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.75f, 0.85f, 1.0f, alpha));
        ImGui::SetCursorPos(ImVec2(padX, padY));
        ImGui::TextUnformatted(it->message.c_str());
        ImGui::PopStyleColor();

        ImGui::End();
        ImGui::PopStyleVar();
        ImGui::PopStyleColor(2);

        yOffset -= boxH + 8.0f;
        ++it;
    }

    if (needFrame) {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    } else if (m_frameActive) {
        endFrame();
    }
}

void MenuOverlay::renderBeatIndicator(float alpha)
{
    if (alpha <= 0.0f) return;

    bool needFrame = !m_frameActive;
    if (needFrame) {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
    }

    ImGui::GetBackgroundDrawList()->AddCircleFilled(
        ImVec2(50, 50),
        20,
        ImColor(1.0f, 0.0f, 0.0f, alpha)
    );

    if (needFrame) {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
}

void MenuOverlay::renderStoryOverlay(const char* stateLabel, float energyRatio, float flux)
{
    if (!stateLabel)
        return;

    bool needFrame = !m_frameActive;
    if (needFrame) {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
    }

    ImGuiIO& io = ImGui::GetIO();
    const ImVec2 pos = ImVec2(14.0f, 14.0f);

    char buf[160];
    snprintf(buf, sizeof(buf), "Story: %s  |  ratio %.2f  |  flux %.3f", stateLabel, energyRatio, flux);

    ImVec2 textSize = ImGui::CalcTextSize(buf);
    ImVec2 pad = ImVec2(10.0f, 6.0f);
    ImVec2 p0 = ImVec2(pos.x - pad.x, pos.y - pad.y);
    ImVec2 p1 = ImVec2(pos.x + textSize.x + pad.x, pos.y + textSize.y + pad.y);

    auto* dl = ImGui::GetBackgroundDrawList();
    dl->AddRectFilled(p0, p1, IM_COL32(12, 16, 28, 190), 6.0f);
    dl->AddRect(p0, p1, IM_COL32(80, 140, 255, 200), 6.0f, 0, 1.5f);
    dl->AddText(pos, IM_COL32(200, 215, 255, 230), buf);

    if (needFrame) {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
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
    bool dismiss = ImGui::Button("  I Understand - Continue", ImVec2(contentW, btnH));

    // Also allow Enter/Space to dismiss
    dismiss = dismiss || ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_Space);

    ImGui::End();

    if (dismiss) {
        return MenuAction::BackToMenu;
    }
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
        const char* ver = "v 0 . 3 . 0  d e v";
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

    // Controls (quick access to help)
    ImGui::SetCursorPosX(pad);
    if (ImGui::Button("  ? Controls", ImVec2(contentW, btnH - 14.0f)))
        action = MenuAction::ShowControls;
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("View all keyboard and gamepad controls");

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

    // Controls (quick access to help)
    ImGui::SetCursorPosX(pad);
    if (ImGui::Button("  ? Controls", ImVec2(contentW, btnH - 14.0f)))
        action = MenuAction::ShowControls;
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("View all keyboard and gamepad controls");

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

    // Bulk favorites toggle (very useful for building playlists)
    ImGui::SameLine(0, 12);
    if (ImGui::Button("★ Add All", ImVec2(85, 0))) {
        if (m_presetDb && m_presetDb->isLoaded() && m_currentCategory != PresetCategory::All) {
            auto vis = m_presetDb->getPresetsInCategory(m_currentCategory);
            addAllVisibleToFavorites(vis);
        } else {
            // Fallback: add everything
            std::vector<uint32_t> all;
            for (uint32_t i = 0; i < static_cast<uint32_t>(m_presetList.size()); ++i) all.push_back(i);
            addAllVisibleToFavorites(all);
        }
    }
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Add all presets in the current category to Favorites");

    ImGui::SameLine(0, 4);
    if (ImGui::Button("☆ Clear All", ImVec2(85, 0))) {
        if (m_presetDb && m_presetDb->isLoaded() && m_currentCategory != PresetCategory::All) {
            auto vis = m_presetDb->getPresetsInCategory(m_currentCategory);
            removeAllVisibleFromFavorites(vis);
        } else {
            std::vector<uint32_t> all;
            for (uint32_t i = 0; i < static_cast<uint32_t>(m_presetList.size()); ++i) all.push_back(i);
            removeAllVisibleFromFavorites(all);
        }
    }
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Remove all presets in the current category from Favorites");

    ImGui::Spacing();

    // ── Category Bar (uses PresetDatabase when available) ─────────────────
    if (m_presetDb && m_presetDb->isLoaded()) {
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(6, 6));
        // Show a compact row of category buttons
        for (size_t ci = 0; ci < m_presetDb->categoryCount(); ++ci) {
            const CategoryStack* cat = m_presetDb->getCategoryByIndex(ci);
            if (!cat) continue;

            bool isActive = (m_currentCategory == cat->category);
            if (isActive) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.5f, 0.9f, 0.9f));
            }

            std::string label = std::string(cat->icon) + " " + cat->displayName + " (" + std::to_string(cat->count()) + ")";
            if (ImGui::Button(label.c_str())) {
                m_currentCategory = cat->category;
                m_searchBuf[0] = '\0'; // clear search when changing category
            }

            if (isActive) ImGui::PopStyleColor();
            ImGui::SameLine();
        }
        ImGui::PopStyleVar();
        ImGui::Spacing();

        // ── Transition Mode (live control for vibe/flow) ───────────────────
        if (m_config) {
            int current = m_config->storyTransitionStyle;
            if (current < 0 || current >= kTransitionStyleCount)
                current = 0;

            ImGui::SetNextItemWidth(190.0f);
            if (ImGui::Combo("##transMode", &current, kTransitionStyles, kTransitionStyleCount)) {
                m_config->storyTransitionStyle = current;
                applyTransitionStyleDefaults(*m_config);
                // Note: main loop will pick up the changes via g_config
            }
            ImGui::SameLine();
            ImGui::TextDisabled("(affects next switch)");

            // Power-user sliders (live edit of raw values)
            ImGui::SameLine(0, 20);
            ImGui::PushItemWidth(90);
            ImGui::SliderFloat("Blend", &m_config->transitionTime, 0.5f, 12.0f, "%.1fs");
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::PushItemWidth(90);
            ImGui::SliderFloat("HardCut", &m_config->hardCutSensitivity, 0.5f, 5.0f, "%.1f");
            ImGui::PopItemWidth();

            // Immediate apply button
            ImGui::SameLine(0, 12);
            if (ImGui::Button("Apply Now", ImVec2(85, 0))) {
                action = MenuAction::ApplyTransitionSettings;
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Push transition settings to projectM + storyteller immediately");
            }
        }
    }

    // Stats
    {
        uint32_t totalCount = static_cast<uint32_t>(m_presetList.size());
        uint32_t favCount = static_cast<uint32_t>(m_favorites.size());
        char stats[128];
        if (m_presetDb && m_presetDb->isLoaded() && m_currentCategory != PresetCategory::All) {
            const CategoryStack* cat = m_presetDb->getCategory(m_currentCategory);
            uint32_t catCount = cat ? cat->count() : 0;
            snprintf(stats, sizeof(stats), "%s: %u presets | %u favorites",
                     cat ? cat->displayName.c_str() : "Category", catCount, favCount);
        } else {
            snprintf(stats, sizeof(stats), "%u presets | %u favorites", totalCount, favCount);
        }
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.45f, 0.45f, 0.55f, 0.75f));
        ImGui::TextUnformatted(stats);
        ImGui::PopStyleColor();
    }

    ImGui::Spacing();

    // Scrollable preset list (filtered by current category + search + favorites)
    float listH = ImGui::GetContentRegionAvail().y - 10.0f;
    ImGui::BeginChild("##presetList", ImVec2(contentW, listH), true);

    std::string searchLower;
    if (m_searchBuf[0]) {
        searchLower = m_searchBuf;
        std::transform(searchLower.begin(), searchLower.end(), searchLower.begin(), ::tolower);
    }

    // Determine which indices to show
    std::vector<uint32_t> indicesToShow;
    if (m_presetDb && m_presetDb->isLoaded() && m_currentCategory != PresetCategory::All) {
        indicesToShow = m_presetDb->getPresetsInCategory(m_currentCategory);
    } else {
        // Fallback: show everything from the flat list
        for (uint32_t i = 0; i < static_cast<uint32_t>(m_presetList.size()); ++i)
            indicesToShow.push_back(i);
    }

    // --- Grid layout (2 columns) for better scannability ---
    ImGui::Columns(2, nullptr, false);
    float colWidth = (contentW - ImGui::GetStyle().ItemSpacing.x) * 0.5f;
    ImGui::SetColumnWidth(0, colWidth);
    ImGui::SetColumnWidth(1, colWidth);

    int colIndex = 0;
    for (uint32_t idx : indicesToShow) {
        // Get display name and path (prefer database entry if available for this list source)
        std::string displayName;
        std::string selectedPath;
        if (m_presetDb && m_presetDb->isLoaded() && m_currentCategory != PresetCategory::All) {
            const ::PresetEntry* pe = m_presetDb->getPreset(idx);
            if (pe) {
                displayName = pe->displayName;
                selectedPath = pe->fullPath;
            } else if (idx < m_presetList.size()) {
                displayName = m_presetList[idx].displayName;
                selectedPath = m_presetList[idx].filename;
            } else {
                continue;
            }
        } else {
            displayName = m_presetList[idx].displayName;
            selectedPath = m_presetList[idx].filename;
        }

        // Filter: favorites only
        if (m_showFavoritesOnly && !isFavorite(idx))
            continue;

        // Filter: search
        if (!searchLower.empty()) {
            std::string nameLower = displayName;
            std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);
            if (nameLower.find(searchLower) == std::string::npos)
                continue;
        }

        ImGui::PushID(static_cast<int>(idx));

        // Star button for favorites
        bool fav = isFavorite(idx);
        if (fav)
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.85f, 0.2f, 1.0f));
        else
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.35f, 0.35f, 0.45f, 0.6f));

        if (ImGui::SmallButton(fav ? "*" : " ")) {
            toggleFavorite(idx);
        }
        ImGui::PopStyleColor();

        ImGui::SameLine();

        // Preset name — clickable (grid friendly)
        bool selected = (m_selectedPreset == idx);
        if (ImGui::Selectable(displayName.c_str(), selected, ImGuiSelectableFlags_AllowDoubleClick)) {
            m_selectedPreset = idx;
            m_selectedPresetPath = selectedPath;
            if (ImGui::IsMouseDoubleClicked(0)) {
                action = MenuAction::PlayPreset;
            }
        }

        ImGui::PopID();

        // Move to next column
        ImGui::NextColumn();
        colIndex++;
        if (colIndex % 2 == 0) {
            ImGui::Spacing(); // small gap between rows
        }
    }

    ImGui::Columns(1); // reset
    ImGui::EndChild();
    ImGui::End();

    return action;
}

// ─── Settings ───────────────────────────────────────────────────────

MenuAction MenuOverlay::renderSettings()
{
    // Use configurable glass opacity so the live visualizer shows through
    float opacity = m_config ? m_config->overlayOpacity : 0.65f;
    drawBackdrop(opacity * 0.5f); // light tint behind the panel

    ImGuiIO& io = ImGui::GetIO();
    ImVec2 ds = io.DisplaySize;

    float margin = 60.0f;
    ImVec2 panelSize(std::min(ds.x - margin * 2, 620.0f), ds.y - margin * 2);
    if (panelSize.y < 400) panelSize.y = 400;
    ImVec2 panelPos((ds.x - panelSize.x) * 0.5f, (ds.y - panelSize.y) * 0.5f);

    // Push glass-panel window background using the overlay opacity
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.05f, 0.05f, 0.11f, opacity));
    ImGui::PushStyleColor(ImGuiCol_ChildBg,  ImVec4(0.04f, 0.04f, 0.09f, opacity * 0.6f));

    ImGui::SetNextWindowPos(panelPos);
    ImGui::SetNextWindowSize(panelSize);
    ImGui::Begin("##settings", nullptr,
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoResize);

    float pad = ImGui::GetStyle().WindowPadding.x;
    float contentW = panelSize.x - pad * 2.0f;

    MenuAction action = MenuAction::None;
    bool changed = false;

    // ── Title row with back button ──
    if (ImGui::Button("< Back", ImVec2(80, 36))) {
        action = MenuAction::BackFromSettings;
    }
    ImGui::SameLine();
    {
        const char* t = "S E T T I N G S";
        ImVec2 ts = ImGui::CalcTextSize(t);
        ImGui::SetCursorPosX((panelSize.x - ts.x) * 0.5f);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.65f, 0.75f, 1.0f, 0.95f));
        ImGui::TextUnformatted(t);
        ImGui::PopStyleColor();
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    if (!m_config) {
        ImGui::TextDisabled("No configuration loaded.");
        ImGui::End();
        return action;
    }

    // Global reset (always visible, not buried at bottom of Advanced)
    {
        ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.45f, 0.08f, 0.08f, 0.50f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.65f, 0.12f, 0.12f, 0.70f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.75f, 0.18f, 0.18f, 0.90f));
        if (ImGui::Button("Reset to Defaults", ImVec2(contentW, 34))) {
            *m_config = VibeusConfig{};
            changed = true;
            showToast("Defaults restored");
        }
        ImGui::PopStyleColor(3);
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Restore all settings to their original values.");

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
    }

    // ── Tab bar ──
    if (ImGui::BeginTabBar("##settingsTabs")) {

        // Pending tab jump (from ShowControls / jumpToSettingsTab)
        const int tabTarget = m_settingsTabTarget;
        m_settingsTabTarget = -1; // consume it

        ImGuiTabItemFlags tabFlagsAudioBeat  = (tabTarget == 0) ? ImGuiTabItemFlags_SetSelected : 0;
        ImGuiTabItemFlags tabFlagsPresets    = (tabTarget == 1) ? ImGuiTabItemFlags_SetSelected : 0;
        ImGuiTabItemFlags tabFlagsVisuals    = (tabTarget == 2) ? ImGuiTabItemFlags_SetSelected : 0;
        ImGuiTabItemFlags tabFlagsAdvanced   = (tabTarget == 3) ? ImGuiTabItemFlags_SetSelected : 0;
        ImGuiTabItemFlags tabFlagsControls   = (tabTarget == 4) ? ImGuiTabItemFlags_SetSelected : 0;

        // ═══════ VIBES TAB ═══════
        if (ImGui::BeginTabItem("  Audio & Beat  ", nullptr, tabFlagsAudioBeat)) {
            m_settingsTab = 0;

            ImGui::BeginChild("##vibesScroll", ImVec2(0, 0), false);

            // ── Mood Preset ──
            ImGui::SeparatorText("Mood Preset");
            {
                const char* moods[] = { "Chill", "Party", "Focus", "Psychedelic", "Custom" };
                int moodIdx = static_cast<int>(m_config->mood);
                ImGui::SetNextItemWidth(contentW * 0.5f);
                if (ImGui::Combo("##mood", &moodIdx, moods, 5)) {
                    m_config->mood = static_cast<MoodPreset>(moodIdx);
                    if (m_config->mood != MoodPreset::Custom) {
                        applyMoodPreset(*m_config, m_config->mood);
                    }
                    changed = true;
                }
                ImGui::SameLine();
                ImGui::TextDisabled("(?)");
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Mood presets adjust audio, speed, transitions\nand sensitivity as a bundle. Choose Custom\nto tune everything manually.");
                }
            }

            ImGui::Spacing();

            // ── Live Status (always visible feedback) ──
            ImGui::SeparatorText("Live Status");
            {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.85f, 1.0f, 1.0f));
                ImGui::Text("Preset: %s", m_livePresetName.c_str());
                ImGui::PopStyleColor();

                const char* state = m_liveStoryState.c_str();
                ImVec4 stateColor = ImVec4(0.4f, 1.0f, 0.5f, 1.0f); // green for Chill
                if (strcmp(state, "BUILD") == 0) stateColor = ImVec4(1.0f, 0.9f, 0.3f, 1.0f);
                else if (strcmp(state, "DROP") == 0) stateColor = ImVec4(1.0f, 0.4f, 0.4f, 1.0f);
                else if (strcmp(state, "SUSTAIN") == 0) stateColor = ImVec4(0.5f, 0.7f, 1.0f, 1.0f);

                ImGui::PushStyleColor(ImGuiCol_Text, stateColor);
                ImGui::Text("Storyteller: %s  |  Beat Sens: %.2f", state, m_liveBeatSens);
                ImGui::PopStyleColor();

                if (m_liveStasis) {
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.6f, 0.3f, 1.0f));
                    ImGui::Text("STASIS — Visuals frozen (no audio)");
                    ImGui::PopStyleColor();
                }
            }

            ImGui::Spacing();

            // ── Visual Quality ──
            ImGui::SeparatorText("Visual Quality");
            {
                // Perf Mode
                const char* perfLabels[] = { "Battery Saver (32×24)", "Balanced (64×48)", "Quality (128×96)" };
                int perfIdx = static_cast<int>(m_config->perfMode);
                ImGui::SetNextItemWidth(contentW * 0.65f);
                if (ImGui::Combo("Performance Mode", &perfIdx, perfLabels, 3)) {
                    m_config->perfMode = static_cast<PerfMode>(perfIdx);
                    changed = true;
                }
                ImGui::SameLine(); ImGui::TextDisabled("(?)");
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Battery Saver = low mesh, no VSync\nBalanced = good quality + VSync\nQuality = high detail + VSync");

                // Mesh Detail
                int mesh = static_cast<int>(m_config->meshDetail);
                ImGui::SetNextItemWidth(contentW * 0.65f);
                if (ImGui::SliderInt("Mesh Detail", &mesh, 32, 256, "%d")) {
                    m_config->meshDetail = static_cast<float>(mesh);
                    changed = true;
                }
                ImGui::SameLine(); ImGui::TextDisabled("(?)");
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Higher = smoother curves, more GPU load.\nLower = faster, more angular look.");

                // Aspect Correction
                if (ImGui::Checkbox("Aspect Ratio Correction", &m_config->aspectCorrection)) {
                    changed = true;
                }
                ImGui::SameLine(); ImGui::TextDisabled("(?)");
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Prevents stretching on ultrawide / non-4:3 displays.");

                // Easter Egg (preset variety)
                ImGui::SetNextItemWidth(contentW * 0.65f);
                if (ImGui::SliderFloat("Preset Variety (Easter Egg)", &m_config->easterEgg, 0.0f, 1.0f, "%.2f")) {
                    changed = true;
                }
                ImGui::SameLine(); ImGui::TextDisabled("(?)");
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Higher = more random preset selection.\nLower = more predictable cycling.");
            }

            ImGui::Spacing();

            // ── Audio ──
            ImGui::SeparatorText("Audio");
            {
                int gainPct = static_cast<int>(m_config->audioGain * 100.0f);
                ImGui::SetNextItemWidth(contentW * 0.60f);
                if (ImGui::SliderInt("Audio Gain", &gainPct, 0, 300, "%d%%")) {
                    m_config->audioGain = gainPct / 100.0f;
                    m_config->mood = MoodPreset::Custom;
                    changed = true;
                }
                ImGui::SameLine(); ImGui::TextDisabled("(?)");
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("How loud the audio signal is for the visualizer.\nHigher = more reactive visuals. 100%% is default.");

                ImGui::SetNextItemWidth(contentW * 0.60f);
                if (ImGui::SliderFloat("Beat Sensitivity", &m_config->beatSensitivity, 0.0f, 5.0f, "%.1f")) {
                    m_config->mood = MoodPreset::Custom;
                    changed = true;
                }
                ImGui::SameLine(); ImGui::TextDisabled("(?)");
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Base beat reactivity (used when Adaptive Beat is off).\nLow = mellow, High = intense.");

                // New: Beat Reactivity multiplier + Adaptive toggle
                ImGui::SetNextItemWidth(contentW * 0.60f);
                if (ImGui::SliderFloat("Beat Reactivity", &m_config->beatReactivity, 0.5f, 2.0f, "%.1fx")) {
                    m_config->mood = MoodPreset::Custom;
                    changed = true;
                }
                ImGui::SameLine(); ImGui::TextDisabled("(?)");
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Multiplier for dynamic beat sensitivity during storytelling.\n1.0x = normal, 1.5x = more explosive, 0.7x = calmer.");

                if (ImGui::Checkbox("Adaptive Beat Sensitivity", &m_config->adaptiveBeat)) {
                    changed = true;
                }
                ImGui::SameLine(); ImGui::TextDisabled("(?)");
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("When ON, the storyteller can raise/lower beat sensitivity\nbased on energy (buildups & drops).\nTurn OFF for consistent fixed sensitivity.");

                if (ImGui::Checkbox("Freeze on Silence##AudioStasis", &m_config->stasisEnabled)) {
                    changed = true;
                }
                ImGui::SameLine(); ImGui::TextDisabled("(?)");
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Stasis freezes visual time when the audio signal stays quiet.\nThe Live Status header shows when it is active.");

                if (m_config->stasisEnabled) {
                    ImGui::SetNextItemWidth(contentW * 0.60f);
                    if (ImGui::SliderFloat("Silence Threshold##AudioStasis", &m_config->stasisThreshold, 0.001f, 0.050f, "%.3f"))
                        changed = true;
                    ImGui::SameLine(); ImGui::TextDisabled("(?)");
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("RMS audio level below this value counts as silence.\nRaise it if quiet music should freeze sooner; lower it\nif Vibeus enters stasis too aggressively.");

                    ImGui::SetNextItemWidth(contentW * 0.60f);
                    if (ImGui::SliderFloat("Stasis Delay##AudioStasis", &m_config->stasisFadeTime, 0.10f, 5.00f, "%.2f sec"))
                        changed = true;
                    ImGui::SameLine(); ImGui::TextDisabled("(?)");
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("How long audio must stay below the threshold\nbefore Vibeus enters stasis.");
                }

                // Beat Hold Time (hysteresis / smoothing)
                ImGui::SetNextItemWidth(contentW * 0.60f);
                int holdMs = static_cast<int>(m_config->beatHoldTime * 1000.0f);
                if (ImGui::SliderInt("Beat Hold Time", &holdMs, 0, 500, "%d ms")) {
                    m_config->beatHoldTime = holdMs / 1000.0f;
                    m_config->mood = MoodPreset::Custom;
                    changed = true;
                }
                ImGui::SameLine(); ImGui::TextDisabled("(?)");
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Minimum time between beat triggers.\nHigher = smoother, less 'nervous' response.\n0 ms = raw projectM beat detection.");
            }

            ImGui::Spacing();

            // ── Storytelling ──
            ImGui::SeparatorText("Storytelling");
            {
                if (ImGui::Checkbox("Intelligent Storytelling", &m_config->storytellingEnabled))
                    changed = true;
                ImGui::SameLine(); ImGui::TextDisabled("(?)");
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Audio-reactive storytelling that detects buildups,\ndrops, and beats to dynamically control preset\ntransitions and visual intensity.\nDisable to use standard projectM auto-advance.");

                // Show tuning controls only when storytelling is enabled
                if (m_config->storytellingEnabled) {
                    ImGui::Spacing();

                    ImGui::SetNextItemWidth(contentW * 0.60f);
                    if (ImGui::SliderFloat("Drop Sensitivity", &m_config->storyDropSensitivity, 1.0f, 4.0f, "%.1f")) {
                        m_config->mood = MoodPreset::Custom;
                        changed = true;
                    }
                    ImGui::SameLine(); ImGui::TextDisabled("(?)");
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("How easily drops are triggered.\nLow = drops fire on smaller energy spikes.\nHigh = only massive peaks cause drops.");

                    ImGui::SetNextItemWidth(contentW * 0.60f);
                    if (ImGui::SliderFloat("Bass Reactivity", &m_config->storyBassReactivity, 0.2f, 3.0f, "%.1fx")) {
                        m_config->mood = MoodPreset::Custom;
                        changed = true;
                    }
                    ImGui::SameLine(); ImGui::TextDisabled("(?)");
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("How much kicks and bass influence detection.\n1x = balanced. Higher = bass dominates.\nLower = snare/hi-hat matters more.");

                    ImGui::SetNextItemWidth(contentW * 0.60f);
                    if (ImGui::SliderFloat("Sustain Hold", &m_config->storySustainMax, 4.0f, 30.0f, "%.0f sec")) {
                        m_config->mood = MoodPreset::Custom;
                        changed = true;
                    }
                    ImGui::SameLine(); ImGui::TextDisabled("(?)");
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("Max time to stay in the Sustain state\n(holding intensity after a drop) before\nreturning to Chill. Longer = more epic plateaus.");

                    // Energy Gates (Advanced) - collapsed by default to keep the UI clean
                    if (ImGui::CollapsingHeader("Energy Gates (Advanced)", ImGuiTreeNodeFlags_None)) {
                        ImGui::TextDisabled("Fine-tune how the storyteller detects buildups and drops.");
                        ImGui::Spacing();

                        ImGui::SetNextItemWidth(contentW * 0.55f);
                        if (ImGui::SliderFloat("Chill Gate", &m_config->storyChillGateSec, 0.5f, 5.0f, "%.1f sec")) {
                            m_config->mood = MoodPreset::Custom; changed = true;
                        }
                        ImGui::SameLine(); ImGui::TextDisabled("(?)");
                        if (ImGui::IsItemHovered())
                            ImGui::SetTooltip("Seconds to stay in Chill before allowing buildup.\nHigher = more relaxed, less jumpy.");

                        ImGui::SetNextItemWidth(contentW * 0.55f);
                        if (ImGui::SliderFloat("Buildup Min", &m_config->storyBuildupMinSec, 0.3f, 3.0f, "%.1f sec")) {
                            m_config->mood = MoodPreset::Custom; changed = true;
                        }
                        ImGui::SameLine(); ImGui::TextDisabled("(?)");
                        if (ImGui::IsItemHovered())
                            ImGui::SetTooltip("Minimum time in Buildup before a Drop can trigger.");

                        ImGui::SetNextItemWidth(contentW * 0.55f);
                        if (ImGui::SliderFloat("Buildup Ratio", &m_config->storyBuildupRatio, 1.0f, 1.15f, "%.3f")) {
                            m_config->mood = MoodPreset::Custom; changed = true;
                        }
                        ImGui::SameLine(); ImGui::TextDisabled("(?)");
                        if (ImGui::IsItemHovered())
                            ImGui::SetTooltip("Short-term vs long-term energy ratio needed to enter Buildup.");

                        ImGui::SetNextItemWidth(contentW * 0.55f);
                        if (ImGui::SliderFloat("Buildup Flux", &m_config->storyBuildupFlux, 0.01f, 0.20f, "%.3f")) {
                            m_config->mood = MoodPreset::Custom; changed = true;
                        }
                        ImGui::SameLine(); ImGui::TextDisabled("(?)");
                        if (ImGui::IsItemHovered())
                            ImGui::SetTooltip("Spectral flux required to help start a buildup.");

                        ImGui::SetNextItemWidth(contentW * 0.55f);
                        if (ImGui::SliderFloat("Drop Ratio", &m_config->storyDropRatio, 1.0f, 1.20f, "%.3f")) {
                            m_config->mood = MoodPreset::Custom; changed = true;
                        }
                        ImGui::SameLine(); ImGui::TextDisabled("(?)");
                        if (ImGui::IsItemHovered())
                            ImGui::SetTooltip("Energy ratio required to trigger a Drop.");

                        ImGui::SetNextItemWidth(contentW * 0.55f);
                        if (ImGui::SliderFloat("Drop Bass", &m_config->storyDropBass, 1.0f, 1.50f, "%.2f")) {
                            m_config->mood = MoodPreset::Custom; changed = true;
                        }
                        ImGui::SameLine(); ImGui::TextDisabled("(?)");
                        if (ImGui::IsItemHovered())
                            ImGui::SetTooltip("Bass energy ratio required to confirm a Drop.");

                        ImGui::SetNextItemWidth(contentW * 0.55f);
                        if (ImGui::SliderFloat("Drop Flux", &m_config->storyDropFlux, 0.01f, 0.25f, "%.3f")) {
                            m_config->mood = MoodPreset::Custom; changed = true;
                        }
                        ImGui::SameLine(); ImGui::TextDisabled("(?)");
                        if (ImGui::IsItemHovered())
                            ImGui::SetTooltip("Spectral flux required to fire a Drop.");
                    }

                    ImGui::SetNextItemWidth(contentW * 0.55f);
                    if (m_config->storyTransitionStyle < 0 || m_config->storyTransitionStyle >= kTransitionStyleCount)
                        m_config->storyTransitionStyle = 0;
                    if (ImGui::Combo("Transition Style", &m_config->storyTransitionStyle, kTransitionStyles, kTransitionStyleCount)) {
                        applyTransitionStyleDefaults(*m_config);
                        m_config->mood = MoodPreset::Custom;
                        changed = true;
                    }
                    ImGui::SameLine(); ImGui::TextDisabled("(?)");
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("How presets change during drops.\nCycling rotates through every effect.\nFast styles use hard or near-hard cuts; slow styles\nuse longer projectM-compatible soft blends.");

                    if (ImGui::Checkbox("Vibeus Post FX Transitions", &m_config->transitionCompositorEnabled))
                        changed = true;
                    ImGui::SameLine(); ImGui::TextDisabled("(?)");
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("Enables the Vibeus-owned post-processing compositor.\nThis adds shader pulses after preset switches without\nmodifying projectM internals.");

                    ImGui::SetNextItemWidth(contentW * 0.60f);
                    if (ImGui::SliderFloat("Post FX Crossfade", &m_config->transitionCrossfadeDuration, 0.50f, 1.20f, "%.2f s"))
                        changed = true;
                    ImGui::SameLine(); ImGui::TextDisabled("(?)");
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("Default compositor opacity blend duration.\n750ms is the professional baseline; avoid going below 500ms\nfor major visual-world changes.");

                    // Prominent Transition Controls (raw sliders + Apply Now)
                    ImGui::Spacing();
                    ImGui::SeparatorText("Transition Controls");
                    {
                        ImGui::SetNextItemWidth(contentW * 0.60f);
                        ImGui::SliderFloat("Blend Time", &m_config->transitionTime, 0.5f, 12.0f, "%.1f s");
                        ImGui::SameLine(); ImGui::TextDisabled("(?)");
                        if (ImGui::IsItemHovered())
                            ImGui::SetTooltip("Crossfade duration when switching presets.");

                        ImGui::SetNextItemWidth(contentW * 0.60f);
                        ImGui::SliderFloat("Hard Cut Sens", &m_config->hardCutSensitivity, 0.5f, 5.0f, "%.1f");
                        ImGui::SameLine(); ImGui::TextDisabled("(?)");
                        if (ImGui::IsItemHovered())
                            ImGui::SetTooltip("Beat volume threshold for instant cuts.");

                        ImGui::Spacing();
                        ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.20f, 0.55f, 0.95f, 0.85f));
                        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.30f, 0.65f, 1.00f, 0.95f));
                        ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.40f, 0.75f, 1.00f, 1.00f));
                        if (ImGui::Button("Apply Now", ImVec2(contentW * 0.55f, 30))) {
                            action = MenuAction::ApplyTransitionSettings;
                        }
                        ImGui::PopStyleColor(3);
                        if (ImGui::IsItemHovered())
                            ImGui::SetTooltip("Immediately push Blend Time + Hard Cut Sensitivity\nto projectM and the storyteller.");
                    }

                    if (ImGui::Checkbox("Lock Preset During Buildup", &m_config->storyBuildupLock))
                        changed = true;
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("Locks the current preset during buildups\nto prevent jarring switches right before\nthe drop. Recommended ON.");

                    if (ImGui::Checkbox("Storyteller Debug Logs (Console)", &m_config->storyDebug))
                        changed = true;
                    ImGui::SameLine(); ImGui::TextDisabled("(?)");
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("Enables real-time storyteller telemetry in the console.\nHelpful for tuning sensitivity and bass reactivity.\nHotkey: L");
                }
            }

            ImGui::Spacing();

            // ── Motion ──
            ImGui::SeparatorText("Motion");
            {
                ImGui::SetNextItemWidth(contentW * 0.60f);
                if (ImGui::SliderFloat("Animation Speed", &m_config->speedMultiplier, 0.05f, 4.0f, "%.2fx")) {
                    m_config->mood = MoodPreset::Custom;
                    changed = true;
                }
                ImGui::SameLine(); ImGui::TextDisabled("(?)");
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Multiplier for animation time.\n1x = normal, <1 = slow-mo, >1 = fast-forward.\nAffects how quickly visuals evolve.");
            }

            // Quick Tips at bottom of Audio & Beat tab
            ImGui::Spacing();
            if (ImGui::CollapsingHeader("Quick Tips", ImGuiTreeNodeFlags_None)) {
                ImGui::BulletText("F key — toggle favorite on current preset");
                ImGui::BulletText("L key — toggle storyteller debug logs in console");
                ImGui::BulletText("Adaptive Beat + Storytelling = most musical feel");
                ImGui::BulletText("Use 'Apply Now' after changing Blend/Hard Cut");
                ImGui::BulletText("Stasis freezes visuals when no audio is detected");
            }

            ImGui::EndChild();
            ImGui::EndTabItem();
        }

        // ═══════ PRESETS TAB ═══════
        if (ImGui::BeginTabItem("  Presets  ", nullptr, tabFlagsPresets)) {
            m_settingsTab = 1;

            ImGui::BeginChild("##presetsScroll", ImVec2(0, 0), false);

            // ── Lock Current Preset ──
            ImGui::SeparatorText("Current Preset");
            {
                bool locked = m_config->vibeLock;
                if (locked) {
                    ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.15f, 0.45f, 0.25f, 0.80f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered,  ImVec4(0.20f, 0.55f, 0.30f, 0.90f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive,   ImVec4(0.25f, 0.65f, 0.35f, 1.00f));
                } else {
                    ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.20f, 0.20f, 0.28f, 0.60f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered,  ImVec4(0.25f, 0.25f, 0.35f, 0.70f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive,   ImVec4(0.30f, 0.30f, 0.40f, 0.80f));
                }

                const char* label = locked
                    ? ">> Preset Locked — Click to Unlock <<"
                    : "Lock Current Preset";
                if (ImGui::Button(label, ImVec2(contentW, 40))) {
                    m_config->vibeLock = !locked;
                    showToast(m_config->vibeLock ? "Preset Locked" : "Preset Unlocked");
                    changed = true;
                }
                ImGui::PopStyleColor(3);
            }
            ImGui::SameLine();
            ImGui::TextDisabled("(?)");
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Locks the current preset so it keeps playing\nindefinitely. Auto-advance and manual skip are\ndisabled while locked. Click again to unlock.");
            }

            ImGui::Spacing();

            ImGui::SeparatorText("Playback");
            {
                if (ImGui::Checkbox("Auto-Advance", &m_config->autoAdvance))
                    changed = true;
                ImGui::SameLine(); ImGui::TextDisabled("(?)");
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Automatically switch to the next preset\nafter the duration expires.");

                if (m_config->autoAdvance) {
                    ImGui::SetNextItemWidth(contentW * 0.65f);
                    if (ImGui::SliderFloat("Preset Duration", &m_config->presetDuration, 10.0f, 120.0f, "%.0f sec")) {
                        m_config->mood = MoodPreset::Custom;
                        changed = true;
                    }
                    ImGui::SameLine(); ImGui::TextDisabled("(?)");
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("How long each preset plays before\nswitching to the next one.");
                }

                if (ImGui::Checkbox("Shuffle", &m_config->shuffle))
                    changed = true;
                ImGui::SameLine(); ImGui::TextDisabled("(?)");
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Randomize preset order instead of\nplaying them sequentially.");

                ImGui::SetNextItemWidth(contentW * 0.65f);
                if (ImGui::SliderFloat("Transition Time", &m_config->transitionTime, 0.0f, 10.0f, "%.1f sec")) {
                    m_config->mood = MoodPreset::Custom;
                    changed = true;
                }
                ImGui::SameLine(); ImGui::TextDisabled("(?)");
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("How long the crossfade lasts when\nswitching between presets.");
            }

            ImGui::Spacing();

            ImGui::SeparatorText("Beat Cuts");
            {
                if (ImGui::Checkbox("Beat-Reactive Cuts", &m_config->hardCutEnabled))
                    changed = true;
                ImGui::SameLine(); ImGui::TextDisabled("(?)");
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("When enabled, a loud beat can\ntrigger an instant preset change\ninstead of a smooth crossfade.");

                if (m_config->hardCutEnabled) {
                    ImGui::SetNextItemWidth(contentW * 0.65f);
                    if (ImGui::SliderFloat("Cut Sensitivity", &m_config->hardCutSensitivity, 0.5f, 4.0f, "%.1f")) {
                        m_config->mood = MoodPreset::Custom;
                        changed = true;
                    }
                    ImGui::SameLine(); ImGui::TextDisabled("(?)");
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("Volume threshold to trigger an instant\ncut. Lower = more frequent cuts.");

                    ImGui::SetNextItemWidth(contentW * 0.65f);
                    int cutDelay = static_cast<int>(m_config->hardCutDuration);
                    if (ImGui::SliderInt("Min Time Before Cut", &cutDelay, 5, 60, "%d sec")) {
                        m_config->hardCutDuration = static_cast<float>(cutDelay);
                        m_config->mood = MoodPreset::Custom;
                        changed = true;
                    }
                    ImGui::SameLine(); ImGui::TextDisabled("(?)");
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("Minimum seconds a preset plays\nbefore a beat can trigger an\ninstant cut to the next one.");
                }
            }

            ImGui::Spacing();

            ImGui::SeparatorText("Variety");
            {
                ImGui::SetNextItemWidth(contentW * 0.65f);
                int eggPct = static_cast<int>(m_config->easterEgg * 100.0f);
                if (ImGui::SliderInt("Preset Variety", &eggPct, 0, 100, "%d%%")) {
                    m_config->easterEgg = eggPct / 100.0f;
                    changed = true;
                }
                ImGui::SameLine(); ImGui::TextDisabled("(?)");
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Randomizes preset durations for variety.\n0%% = all presets same length\n100%% = highly varied durations");
            }

            ImGui::EndChild();
            ImGui::EndTabItem();
        }

        // ═══════ DISPLAY TAB ═══════
        if (ImGui::BeginTabItem("  Display & Quality  ", nullptr, tabFlagsVisuals)) {
            m_settingsTab = 2;

            ImGui::BeginChild("##displayScroll", ImVec2(0, 0), false);

            ImGui::SeparatorText("Display");
            {
                if (ImGui::Checkbox("Fullscreen", &m_config->fullscreen))
                    changed = true;
                ImGui::SameLine(); ImGui::TextDisabled("(?)");
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Toggle between windowed and fullscreen mode.\nAlso available with F11.");

                if (ImGui::Checkbox("Show FPS", &m_config->showFps))
                    changed = true;
                ImGui::SameLine(); ImGui::TextDisabled("(?)");
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Display frames-per-second counter\nin the window title bar.");

                if (ImGui::Checkbox("Aspect Correction", &m_config->aspectCorrection))
                    changed = true;
                ImGui::SameLine(); ImGui::TextDisabled("(?)");
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Fixes stretching on ultrawide monitors.\nDisable for intentional anamorphic look.");

                int opacityPct = static_cast<int>(m_config->overlayOpacity * 100.0f);
                ImGui::SetNextItemWidth(contentW * 0.65f);
                if (ImGui::SliderInt("Overlay Opacity", &opacityPct, 10, 95, "%d%%")) {
                    m_config->overlayOpacity = opacityPct / 100.0f;
                    changed = true;
                }
                ImGui::SameLine(); ImGui::TextDisabled("(?)");
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Glass transparency of this settings panel.\nLower = more see-through, so you can\nwatch the visualizer while tuning.");
            }

            ImGui::Spacing();

            ImGui::SeparatorText("Render Quality");
            {
                const char* perfModes[] = { "Battery Saver", "Balanced", "Quality" };
                int perfIdx = static_cast<int>(m_config->perfMode);
                ImGui::SetNextItemWidth(contentW * 0.65f);
                if (ImGui::Combo("Performance Mode", &perfIdx, perfModes, 3)) {
                    m_config->perfMode = static_cast<PerfMode>(perfIdx);
                    // Keep mesh + perf mode in sync so both settings "work" intuitively.
                    switch (m_config->perfMode) {
                        case PerfMode::BatterySaver: m_config->meshDetail = 32.0f;  break;
                        case PerfMode::Balanced:    m_config->meshDetail = 64.0f;  break;
                        case PerfMode::Quality:     m_config->meshDetail = 128.0f; break;
                    }
                    changed = true;
                }
                ImGui::SameLine(); ImGui::TextDisabled("(?)");
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Battery Saver: sets Mesh Detail=32, VSync off\nBalanced: sets Mesh Detail=64, VSync on\nQuality: sets Mesh Detail=128, VSync on\n\nYou can override Mesh Detail after picking a mode.");

                ImGui::SetNextItemWidth(contentW * 0.65f);
                if (ImGui::SliderFloat("Mesh Detail", &m_config->meshDetail, 32.0f, 128.0f, "%.0f"))
                    changed = true;
                ImGui::SameLine(); ImGui::TextDisabled("(?)");
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Warp mesh resolution.\nHigher = smoother warps but slower.\nLower = faster but blockier effects.\n\nChanges take effect at the next preset transition\nor on restart.");
                ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + contentW * 0.92f);
                ImGui::TextDisabled("Mesh changes take effect at the next preset transition or on restart.");
                ImGui::PopTextWrapPos();
            }

            ImGui::Spacing();

            ImGui::SeparatorText("Accessibility");
            {
                if (ImGui::Checkbox("Flash Limiter", &m_config->flashLimiter))
                    changed = true;
                ImGui::SameLine(); ImGui::TextDisabled("(?)");
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Limits rapid brightness changes\nto reduce photosensitivity risk.");

                if (ImGui::Checkbox("Reduced Motion", &m_config->reducedMotion))
                    changed = true;
                ImGui::SameLine(); ImGui::TextDisabled("(?)");
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Slows down animations and reduces\nrapid visual movement for comfort.");

                int fontPct = static_cast<int>(m_config->fontScale * 100.0f);
                ImGui::SetNextItemWidth(contentW * 0.5f);
                if (ImGui::SliderInt("Font Scale", &fontPct, 75, 200, "%d%%")) {
                    m_config->fontScale = fontPct / 100.0f;
                    changed = true;
                }
                ImGui::SameLine(); ImGui::TextDisabled("(?)");
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Scale all UI text larger or smaller.\nUseful for high-DPI displays or\ncouch/TV viewing distance.");

                ImGui::Spacing();
                ImGui::SeparatorText("Silence / Stasis");

                if (ImGui::Checkbox("Freeze on Silence", &m_config->stasisEnabled))
                    changed = true;
                ImGui::SameLine(); ImGui::TextDisabled("(?)");
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("When audio stays below the silence threshold,\nfreeze beat reactivity instead of letting visuals\nkeep pulsing like music is still playing.");

                ImGui::SetNextItemWidth(contentW * 0.5f);
                if (ImGui::SliderFloat("Silence Threshold", &m_config->stasisThreshold, 0.001f, 0.050f, "%.3f"))
                    changed = true;
                ImGui::SameLine(); ImGui::TextDisabled("(?)");
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("RMS audio level below this value counts as silence.\nRaise it if quiet music should freeze sooner; lower it\nif Vibeus enters stasis too aggressively.");

                ImGui::SetNextItemWidth(contentW * 0.5f);
                if (ImGui::SliderFloat("Stasis Delay", &m_config->stasisFadeTime, 0.10f, 5.00f, "%.2f sec"))
                    changed = true;
                ImGui::SameLine(); ImGui::TextDisabled("(?)");
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("How long audio must stay below the threshold\nbefore Vibeus enters stasis. The Live Status header\nshows when stasis is active.");
            }

            ImGui::EndChild();
            ImGui::EndTabItem();
        }

        // ═══════ ADVANCED TAB ═══════
        if (ImGui::BeginTabItem("  Advanced  ", nullptr, tabFlagsAdvanced)) {
            m_settingsTab = 3;

            ImGui::BeginChild("##advScroll", ImVec2(0, 0), false);

            ImGui::SeparatorText("Input");
            {
                ImGui::SetNextItemWidth(contentW * 0.65f);
                if (ImGui::SliderInt("Gamepad Deadzone", &m_config->gamepadDeadzone, 2000, 16000))
                    changed = true;
                ImGui::SameLine(); ImGui::TextDisabled("(?)");
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Minimum stick movement before input registers.\nIncrease if you see drift, decrease for\nmore responsive stick control.");
            }

            ImGui::Spacing();

            ImGui::SeparatorText("Preset Management");
            {
                if (ImGui::Checkbox("Auto-skip Broken Presets (Rescue)", &m_config->storyDebugRescue))
                    changed = true;
                ImGui::SameLine(); ImGui::TextDisabled("(?)");
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Automatically skip presets that trigger runtime shader errors.");

                if (ImGui::Checkbox("Validate Presets on Startup", &m_config->validatePresetsOnStartup))
                    changed = true;
                ImGui::SameLine(); ImGui::TextDisabled("(?)");
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Runs a long scan to detect broken presets.\nRecommended to run once, then disable.\nBroken presets are stored in AppData (broken_presets.txt).\n\nIf presets cannot be moved (Program Files permissions),\nthey will still be hidden via the blacklist.");

                ImGui::Spacing();
                ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + contentW * 0.92f);
                ImGui::TextColored(ImVec4(0.62f, 0.78f, 1.0f, 0.95f),
                                   "Validation status: %s on next launch",
                                   m_config->validatePresetsOnStartup ? "enabled" : "disabled");
                ImGui::TextDisabled("Broken presets are never deleted by validation. They are blacklisted and, when possible, moved to a reversible quarantine folder.");
                if (!m_userDataDir.empty()) {
                    ImGui::TextDisabled("Blacklist: %s", (m_userDataDir + "\\broken_presets.txt").c_str());
                    ImGui::TextDisabled("Quarantine: %s", (m_userDataDir + "\\broken_presets_quarantine\\").c_str());
                    ImGui::TextDisabled("Validation log: %s", (m_userDataDir + "\\preset_validation.log").c_str());
                } else {
                    ImGui::TextDisabled("Preset validation files are stored under %%APPDATA%%\\Vibeus.");
                }
                ImGui::PopTextWrapPos();
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::EndChild();
            ImGui::EndTabItem();
        }

        // ── Controls Tab ──
        if (ImGui::BeginTabItem("  Controls  ", nullptr, tabFlagsControls)) {
            ImGui::Spacing();
            ImGui::BeginChild("##controlsScroll", ImVec2(0, 0), false);
            {
                float contentW = ImGui::GetContentRegionAvail().x;

                auto keyName = [](int key) -> const char* {
                    const char* n = SDL_GetKeyName(static_cast<SDL_Keycode>(key));
                    return (n && n[0]) ? n : "Unbound";
                };

                auto gpName = [](int btn) -> const char* {
                    switch (static_cast<SDL_GameControllerButton>(btn)) {
                    case SDL_CONTROLLER_BUTTON_A:              return "A";
                    case SDL_CONTROLLER_BUTTON_B:              return "B";
                    case SDL_CONTROLLER_BUTTON_X:              return "X";
                    case SDL_CONTROLLER_BUTTON_Y:              return "Y";
                    case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:   return "LB";
                    case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:  return "RB";
                    default: {
                        const char* s = SDL_GameControllerGetStringForButton(static_cast<SDL_GameControllerButton>(btn));
                        return (s && s[0]) ? s : "Unbound";
                    }
                    }
                };
                
                ImGui::SeparatorText("Keyboard Controls");
                ImGui::Spacing();

                // Use a table for better layout
                if (ImGui::BeginTable("##keyboardControls", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
                    ImGui::TableSetupColumn("Action", ImGuiTableColumnFlags_WidthFixed, contentW * 0.5f);
                    ImGui::TableSetupColumn("Key", ImGuiTableColumnFlags_WidthStretch);
                    ImGui::TableHeadersRow();

                    // Preset Navigation
                    ImGui::TableNextRow(); ImGui::TableNextColumn();
                    ImGui::Text("Next Preset"); ImGui::TableNextColumn();
                    ImGui::TextDisabled("%s  or  %s", keyName(m_config->keyNextPreset), keyName(SDLK_RIGHT));
                    
                    ImGui::TableNextRow(); ImGui::TableNextColumn();
                    ImGui::Text("Previous Preset"); ImGui::TableNextColumn();
                    ImGui::TextDisabled("%s  or  %s", keyName(m_config->keyPrevPreset), keyName(SDLK_LEFT));
                    
                    ImGui::TableNextRow(); ImGui::TableNextColumn();
                    ImGui::Text("Random Preset"); ImGui::TableNextColumn();
                    ImGui::TextDisabled("%s", keyName(m_config->keyRandomPreset));
                    
                    ImGui::TableNextRow(); ImGui::TableNextColumn();
                    ImGui::Text("History (Go Back)"); ImGui::TableNextColumn();
                    ImGui::TextDisabled("%s", keyName(m_config->keyHistory));
                    
                    ImGui::TableNextRow(); ImGui::TableNextColumn();
                    ImGui::Text("Toggle Shuffle"); ImGui::TableNextColumn();
                    ImGui::TextDisabled("%s", keyName(m_config->keyShuffle));

                    // Audio Controls
                    ImGui::TableNextRow(); ImGui::TableNextColumn();
                    ImGui::TextColored(ImVec4(0.7f, 0.9f, 1.0f, 1.0f), "Audio Gain Up"); ImGui::TableNextColumn();
                    ImGui::TextDisabled("%s", keyName(m_config->keyAudioGainUp));
                    
                    ImGui::TableNextRow(); ImGui::TableNextColumn();
                    ImGui::TextColored(ImVec4(0.7f, 0.9f, 1.0f, 1.0f), "Audio Gain Down"); ImGui::TableNextColumn();
                    ImGui::TextDisabled("%s", keyName(m_config->keyAudioGainDown));
                    
                    ImGui::TableNextRow(); ImGui::TableNextColumn();
                    ImGui::TextColored(ImVec4(0.7f, 0.9f, 1.0f, 1.0f), "Reset Audio Gain"); ImGui::TableNextColumn();
                    ImGui::TextDisabled("%s", keyName(m_config->keyAudioGainReset));

                    // Visual Controls
                    ImGui::TableNextRow(); ImGui::TableNextColumn();
                    ImGui::TextColored(ImVec4(1.0f, 0.9f, 0.7f, 1.0f), "Beat Sensitivity Up"); ImGui::TableNextColumn();
                    ImGui::TextDisabled("%s", keyName(m_config->keyBeatSensUp));
                    
                    ImGui::TableNextRow(); ImGui::TableNextColumn();
                    ImGui::TextColored(ImVec4(1.0f, 0.9f, 0.7f, 1.0f), "Beat Sensitivity Down"); ImGui::TableNextColumn();
                    ImGui::TextDisabled("%s", keyName(m_config->keyBeatSensDown));
                    
                    ImGui::TableNextRow(); ImGui::TableNextColumn();
                    ImGui::TextColored(ImVec4(1.0f, 0.9f, 0.7f, 1.0f), "Speed Up"); ImGui::TableNextColumn();
                    ImGui::TextDisabled("%s", keyName(m_config->keySpeedUp));
                    
                    ImGui::TableNextRow(); ImGui::TableNextColumn();
                    ImGui::TextColored(ImVec4(1.0f, 0.9f, 0.7f, 1.0f), "Speed Down"); ImGui::TableNextColumn();
                    ImGui::TextDisabled("%s", keyName(m_config->keySpeedDown));
                    
                    ImGui::TableNextRow(); ImGui::TableNextColumn();
                    ImGui::TextColored(ImVec4(1.0f, 0.9f, 0.7f, 1.0f), "Reset Speed"); ImGui::TableNextColumn();
                    ImGui::TextDisabled("%s", keyName(m_config->keySpeedReset));

                    // Display
                    ImGui::TableNextRow(); ImGui::TableNextColumn();
                    ImGui::Text("Toggle Fullscreen"); ImGui::TableNextColumn();
                    ImGui::TextDisabled("%s  or  %s", keyName(m_config->keyFullscreen), keyName(SDLK_F11));
                    
                    ImGui::TableNextRow(); ImGui::TableNextColumn();
                    ImGui::Text("Toggle Debug Info"); ImGui::TableNextColumn();
                    ImGui::TextDisabled("%s", keyName(m_config->keyDebug));

                    // Menu
                    ImGui::TableNextRow(); ImGui::TableNextColumn();
                    ImGui::Text("Pause Menu"); ImGui::TableNextColumn();
                    ImGui::TextDisabled("Esc");
                    
                    ImGui::TableNextRow(); ImGui::TableNextColumn();
                    ImGui::Text("Quit"); ImGui::TableNextColumn();
                    ImGui::TextDisabled("%s", keyName(m_config->keyQuit));

                    ImGui::EndTable();
                }

                ImGui::Spacing();
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                ImGui::SeparatorText("Gamepad Controls");
                ImGui::Spacing();

                if (ImGui::BeginTable("##gamepadControls", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
                    ImGui::TableSetupColumn("Action", ImGuiTableColumnFlags_WidthFixed, contentW * 0.5f);
                    ImGui::TableSetupColumn("Button", ImGuiTableColumnFlags_WidthStretch);
                    ImGui::TableHeadersRow();

                    // Face Buttons
                    ImGui::TableNextRow(); ImGui::TableNextColumn();
                    ImGui::Text("Next Preset"); ImGui::TableNextColumn();
                    ImGui::TextDisabled("%s", gpName(m_config->gpNextPreset));
                    
                    ImGui::TableNextRow(); ImGui::TableNextColumn();
                    ImGui::Text("Previous Preset"); ImGui::TableNextColumn();
                    ImGui::TextDisabled("%s", gpName(m_config->gpPrevPreset));
                    
                    ImGui::TableNextRow(); ImGui::TableNextColumn();
                    ImGui::Text("Random Preset"); ImGui::TableNextColumn();
                    ImGui::TextDisabled("%s", gpName(m_config->gpRandomPreset));
                    
                    ImGui::TableNextRow(); ImGui::TableNextColumn();
                    ImGui::Text("Toggle Shuffle"); ImGui::TableNextColumn();
                    ImGui::TextDisabled("%s", gpName(m_config->gpShuffle));

                    // Shoulder Buttons
                    ImGui::TableNextRow(); ImGui::TableNextColumn();
                    ImGui::TextColored(ImVec4(0.7f, 0.9f, 1.0f, 1.0f), "Audio Gain Up"); ImGui::TableNextColumn();
                    ImGui::TextDisabled("%s", gpName(m_config->gpAudioGainUp));
                    
                    ImGui::TableNextRow(); ImGui::TableNextColumn();
                    ImGui::TextColored(ImVec4(0.7f, 0.9f, 1.0f, 1.0f), "Audio Gain Down"); ImGui::TableNextColumn();
                    ImGui::TextDisabled("%s", gpName(m_config->gpAudioGainDown));

                    // Triggers
                    ImGui::TableNextRow(); ImGui::TableNextColumn();
                    ImGui::TextColored(ImVec4(1.0f, 0.9f, 0.7f, 1.0f), "Speed Up (Analog)"); ImGui::TableNextColumn();
                    ImGui::TextDisabled("RT  (R2)");
                    
                    ImGui::TableNextRow(); ImGui::TableNextColumn();
                    ImGui::TextColored(ImVec4(1.0f, 0.9f, 0.7f, 1.0f), "Speed Down (Analog)"); ImGui::TableNextColumn();
                    ImGui::TextDisabled("LT  (L2)");

                    // D-Pad
                    ImGui::TableNextRow(); ImGui::TableNextColumn();
                    ImGui::TextColored(ImVec4(1.0f, 0.9f, 0.7f, 1.0f), "Beat Sensitivity Up"); ImGui::TableNextColumn();
                    ImGui::TextDisabled("D-Pad Up");
                    
                    ImGui::TableNextRow(); ImGui::TableNextColumn();
                    ImGui::TextColored(ImVec4(1.0f, 0.9f, 0.7f, 1.0f), "Beat Sensitivity Down"); ImGui::TableNextColumn();
                    ImGui::TextDisabled("D-Pad Down");

                    // Analog Sticks
                    ImGui::TableNextRow(); ImGui::TableNextColumn();
                    ImGui::TextColored(ImVec4(1.0f, 0.9f, 0.7f, 1.0f), "Speed Control"); ImGui::TableNextColumn();
                    ImGui::TextDisabled("Left Stick (Horizontal)");

                    // Menu
                    ImGui::TableNextRow(); ImGui::TableNextColumn();
                    ImGui::Text("Pause Menu"); ImGui::TableNextColumn();
                    ImGui::TextDisabled("Start");

                    ImGui::EndTable();
                }

                ImGui::Spacing();
                ImGui::Spacing();
                ImGui::TextDisabled("Note: Control remapping coming soon!");
            }
            ImGui::EndChild();
            ImGui::EndTabItem();
        }

        // ═══════ ABOUT TAB ═══════
        if (ImGui::BeginTabItem("  About  ", nullptr, tabTarget == 5 ? ImGuiTabItemFlags_SetSelected : 0)) {

            ImGui::BeginChild("##aboutScroll", ImVec2(0, 0), false);

            // Title / Version
            {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.65f, 0.78f, 1.0f, 1.0f));
                ImGui::SetWindowFontScale(1.15f);
                ImGui::Text("Vibeus  v0.3.0-dev");
                ImGui::SetWindowFontScale(1.0f);
                ImGui::PopStyleColor();
            }
            ImGui::Spacing();
            ImGui::TextWrapped("A real-time music visualizer for Windows. Reacts to whatever audio is playing on your system using the projectM rendering engine and thousands of community presets.");
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            // Third-party dependencies
            ImGui::SeparatorText("Third-Party Libraries");
            ImGui::Spacing();

            auto depRow = [](const char* name, const char* license, const char* note) {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.80f, 0.90f, 1.0f, 1.0f));
                ImGui::Text("%-28s", name);
                ImGui::PopStyleColor();
                ImGui::SameLine();
                ImGui::TextDisabled("%s", license);
                if (note && note[0]) {
                    ImGui::SameLine();
                    ImGui::TextDisabled("— %s", note);
                }
            };

            depRow("projectM 4.x",          "LGPL-2.1",   "Milkdrop-compatible visualization engine");
            depRow("SDL2",                   "zlib",       "Window, input, gamepad");
            depRow("Dear ImGui",             "MIT",        "Immediate-mode UI");
            depRow("OpenGL 3.3",             "Khronos",    "Rendering");
            depRow("nlohmann/json",          "MIT",        "Config persistence");
            depRow("presets-cream-of-the-crop", "CC / community", "Curated Milkdrop preset pack");

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            // License file status
            ImGui::SeparatorText("License File Bundling Status");
            ImGui::Spacing();
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.55f, 1.0f, 0.65f, 1.0f));
            ImGui::TextWrapped("Third-party notice files are bundled in the licenses folder beside Vibeus.exe.");
            ImGui::PopStyleColor();
            ImGui::Spacing();

            auto licRow = [](const char* file, const char* status) {
                ImGui::PushStyleColor(ImGuiCol_Text,
                    status[0] == 'M' ? ImVec4(1.0f,0.6f,0.3f,1.0f) : ImVec4(0.5f,1.0f,0.6f,1.0f));
                ImGui::Text("%-32s", file);
                ImGui::PopStyleColor();
                ImGui::SameLine();
                ImGui::TextDisabled("%s", status);
            };

            licRow("LICENSE_projectM.txt              (LGPL-2.1)", "BUNDLED");
            licRow("NOTICE_projectM.txt               (projectM)",  "BUNDLED");
            licRow("LICENSE_SDL2.txt                  (zlib)",      "BUNDLED");
            licRow("LICENSE_ImGui.txt                 (MIT)",       "BUNDLED");
            licRow("LICENSE_nlohmann_json.txt         (MIT)",       "BUNDLED");
            licRow("LICENSE_presets_cream_of_the_crop.md",          "BUNDLED");
            licRow("CREDITS_presets_cream_of_the_crop.txt",         "BUNDLED");

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            // Preset community credit
            ImGui::SeparatorText("Preset Credits");
            ImGui::Spacing();
            ImGui::TextWrapped("This build includes the 'Cream of the Crop' preset collection, curated by the projectM community. Presets were created by numerous artists including Ryan Geiss (original Milkdrop creator) and hundreds of community contributors.");
            ImGui::Spacing();
            ImGui::TextDisabled("See https://github.com/projectM-visualizer/presets-cream-of-the-crop");

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            // Project links
            ImGui::SeparatorText("Project Links");
            ImGui::TextDisabled("projectM upstream:  https://github.com/projectM-visualizer/projectm");
            ImGui::TextDisabled("Vibeus:             https://github.com/fourthdensity/vibeusvisualizer");

            ImGui::EndChild();
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
    ImGui::PopStyleColor(2); // WindowBg + ChildBg

    // If anything changed and we haven't already requested a back action
    if (changed && action == MenuAction::None)
        action = MenuAction::ApplySettings;

    return action;
}

// ─── Favorites ──────────────────────────────────────────────────────

std::string MenuOverlay::selectedPresetPath() const
{
    if (!m_selectedPresetPath.empty())
        return m_selectedPresetPath;

    if (m_presetDb && m_presetDb->isLoaded()) {
        const ::PresetEntry* pe = m_presetDb->getPreset(m_selectedPreset);
        if (pe) return pe->fullPath;
    }

    if (m_selectedPreset < m_presetList.size())
        return m_presetList[m_selectedPreset].filename;

    return {};
}

void MenuOverlay::toggleFavorite(uint32_t idx)
{
    if (m_favorites.count(idx))
        m_favorites.erase(idx);
    else
        m_favorites.insert(idx);

    // Keep config in sync for persistence in config.json
    if (m_config) {
        m_config->favoritePresetPaths.clear();
        for (uint32_t i : m_favorites) {
            if (m_presetDb && m_presetDb->isLoaded()) {
                const ::PresetEntry* pe = m_presetDb->getPreset(i);
                if (pe) m_config->favoritePresetPaths.push_back(pe->fullPath);
            } else if (i < m_presetList.size()) {
                m_config->favoritePresetPaths.push_back(m_presetList[i].filename);
            }
        }
    }
}

// Bulk add/remove for "Select All" / "Clear All" in the preset browser
void MenuOverlay::addAllVisibleToFavorites(const std::vector<uint32_t>& visibleIndices)
{
    for (uint32_t idx : visibleIndices) {
        if (!m_favorites.count(idx)) {
            m_favorites.insert(idx);
        }
    }
    // Sync to config
    if (m_config) {
        m_config->favoritePresetPaths.clear();
        for (uint32_t i : m_favorites) {
            if (m_presetDb && m_presetDb->isLoaded()) {
                const ::PresetEntry* pe = m_presetDb->getPreset(i);
                if (pe) m_config->favoritePresetPaths.push_back(pe->fullPath);
            }
        }
    }
}

void MenuOverlay::removeAllVisibleFromFavorites(const std::vector<uint32_t>& visibleIndices)
{
    for (uint32_t idx : visibleIndices) {
        m_favorites.erase(idx);
    }
    // Sync to config
    if (m_config) {
        m_config->favoritePresetPaths.clear();
        for (uint32_t i : m_favorites) {
            if (m_presetDb && m_presetDb->isLoaded()) {
                const ::PresetEntry* pe = m_presetDb->getPreset(i);
                if (pe) m_config->favoritePresetPaths.push_back(pe->fullPath);
            }
        }
    }
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

    io.FontGlobalScale = 1.40f;   // slightly larger for readability on big screens

    style.WindowRounding    = 14.0f;
    style.FrameRounding     = 9.0f;
    style.PopupRounding     = 10.0f;
    style.ScrollbarRounding = 10.0f;
    style.GrabRounding      = 7.0f;

    style.WindowPadding = ImVec2(22, 18);
    style.FramePadding  = ImVec2(14, 9);
    style.ItemSpacing   = ImVec2(10, 7);
    style.ItemInnerSpacing = ImVec2(8, 6);

    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize  = 0.0f;

    ImVec4* c = style.Colors;

    // Deep dark blue glass theme
    c[ImGuiCol_WindowBg] = ImVec4(0.045f, 0.045f, 0.10f, 0.94f);
    c[ImGuiCol_ChildBg]  = ImVec4(0.035f, 0.035f, 0.08f, 0.55f);
    c[ImGuiCol_Border]   = ImVec4(0.40f, 0.50f, 0.95f, 0.18f);

    // Buttons - soft dark with blue hover
    c[ImGuiCol_Button]        = ImVec4(0.10f, 0.11f, 0.22f, 0.65f);
    c[ImGuiCol_ButtonHovered] = ImVec4(0.18f, 0.22f, 0.45f, 0.85f);
    c[ImGuiCol_ButtonActive]  = ImVec4(0.25f, 0.32f, 0.60f, 0.95f);

    // Prominent cyan/blue accent for important actions
    c[ImGuiCol_SliderGrab]        = ImVec4(0.35f, 0.65f, 1.00f, 0.90f);
    c[ImGuiCol_SliderGrabActive]  = ImVec4(0.50f, 0.80f, 1.00f, 1.00f);

    c[ImGuiCol_Header]        = ImVec4(0.12f, 0.14f, 0.28f, 0.60f);
    c[ImGuiCol_HeaderHovered] = ImVec4(0.20f, 0.25f, 0.48f, 0.75f);
    c[ImGuiCol_HeaderActive]  = ImVec4(0.28f, 0.35f, 0.62f, 0.90f);

    c[ImGuiCol_FrameBg]        = ImVec4(0.07f, 0.07f, 0.15f, 0.75f);
    c[ImGuiCol_FrameBgHovered] = ImVec4(0.11f, 0.12f, 0.24f, 0.85f);
    c[ImGuiCol_FrameBgActive]  = ImVec4(0.15f, 0.18f, 0.32f, 0.95f);

    c[ImGuiCol_Text]         = ImVec4(0.93f, 0.94f, 0.98f, 1.0f);
    c[ImGuiCol_TextDisabled] = ImVec4(0.42f, 0.42f, 0.52f, 1.0f);

    c[ImGuiCol_Separator] = ImVec4(0.40f, 0.50f, 0.95f, 0.12f);

    c[ImGuiCol_ScrollbarBg]          = ImVec4(0.03f, 0.03f, 0.07f, 0.55f);
    c[ImGuiCol_ScrollbarGrab]        = ImVec4(0.20f, 0.22f, 0.35f, 0.75f);
    c[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.28f, 0.32f, 0.50f, 0.85f);
    c[ImGuiCol_ScrollbarGrabActive]  = ImVec4(0.35f, 0.42f, 0.65f, 0.95f);

    c[ImGuiCol_NavHighlight] = ImVec4(0.40f, 0.60f, 1.00f, 0.85f);
    c[ImGuiCol_CheckMark]    = ImVec4(0.50f, 0.75f, 1.00f, 0.95f);
}
