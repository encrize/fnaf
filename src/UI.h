#pragma once

#include <raylib.h>
#include <string>
#include <vector>
#include "Common.h"

namespace FNAF {

class Audio;
class AssetManager;
class Settings;
class NightManager;
class Power;

class UI {
private:
    float m_menuTimer = 0.0f;
    float m_jumpscareTimer = 0.0f;
    float m_sixAmTimer = 0.0f;
    int m_customBear = 0;
    int m_customRabbit = 0;
    int m_customChicken = 0;
    int m_customFox = 0;

    // Custom Night portraits
    Texture2D m_bearPortrait{};
    Texture2D m_rabbitPortrait{};
    Texture2D m_chickenPortrait{};
    Texture2D m_foxPortrait{};

    // Menu Animated Freddy frames
    std::vector<Texture2D> m_menuFreddyFrames;

    // On-demand Jumpscare frames (lazy-loaded ONLY during active jumpscare, then immediately unloaded)
    std::vector<Texture2D> m_activeJumpscareFrames;
    std::vector<std::string> m_activeJumpscarePaths;
    AnimatronicID m_activeJumpscareAttacker = AnimatronicID::Bear;
    AssetManager* m_assets = nullptr;

    void LoadJumpscareFrames(AnimatronicID attacker);
    void UnloadJumpscareFrames();

    // Newspaper
    Texture2D m_newspaperTex{};

    // Power Usage Bars
    std::vector<Texture2D> m_usageTex;

    // Font
    Font m_gameFont{};

public:
    UI();
    ~UI();

    void LoadAssets(AssetManager& assets);
    void Reset();

    // Render methods
    void RenderMainMenu(int currentNight, bool customNightUnlocked, Settings& settings, Audio& audio);
    void RenderHUD(const NightManager& nightMgr, const Power& power, bool monitorUp);
    void RenderPhoneMuteButton(Vector2 mousePos);
    void RenderJumpscare(AnimatronicID attacker, float dt, Audio& audio);
    void RenderGameOver(float dt);
    void Render6AM(float dt, Audio& audio);
    void RenderWinScreen();
    void RenderSettingsMenu(Settings& settings, Audio& audio);
    void RenderCustomNightMenu(NightManager& nightMgr, Audio& audio);

    // Input handlers
    bool HandleMainMenuInput(GameState& state, NightManager& nightMgr, Settings& settings, Audio& audio, Vector2 mousePos);
    bool HandleSettingsInput(GameState& state, Settings& settings, Audio& audio, Vector2 mousePos);
    bool HandleCustomNightInput(GameState& state, NightManager& nightMgr, Audio& audio, Vector2 mousePos);
    bool HandleGameOverInput(GameState& state, NightManager& nightMgr, Audio& audio, Vector2 mousePos);
    bool Handle6AMInput(GameState& state, NightManager& nightMgr, Audio& audio);
    bool HandleWinScreenInput(GameState& state, Audio& audio, Vector2 mousePos);

    // Jumpscare helpers
    void StartJumpscare();
    bool IsJumpscareFinished() const { return m_jumpscareTimer > 1.5f; }
};

} // namespace FNAF
