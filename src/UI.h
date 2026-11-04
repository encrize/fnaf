#pragma once

#include <raylib.h>
#include <string>
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

    Texture2D m_bearPortrait{};
    Texture2D m_rabbitPortrait{};
    Texture2D m_chickenPortrait{};
    Texture2D m_foxPortrait{};

public:
    UI();

    void LoadAssets(AssetManager& assets);
    void Reset();

    // Render methods
    void RenderMainMenu(int currentNight, bool customNightUnlocked, Settings& settings, Audio& audio);
    void RenderHUD(const NightManager& nightMgr, const Power& power, bool monitorUp);
    void RenderPhoneSubtitles(const std::string& text);
    void RenderJumpscare(AnimatronicID attacker, float dt, Audio& audio);
    void RenderGameOver(float dt);
    void Render6AM(float dt, Audio& audio);
    void RenderWinScreen();
    void RenderSettingsMenu(Settings& settings, Audio& audio);
    void RenderCustomNightMenu(NightManager& nightMgr, Audio& audio);

    // Input handlers
    bool HandleMainMenuInput(GameState& state, NightManager& nightMgr, Settings& settings, Audio& audio);
    bool HandleSettingsInput(GameState& state, Settings& settings, Audio& audio);
    bool HandleCustomNightInput(GameState& state, NightManager& nightMgr, Audio& audio);
    bool HandleGameOverInput(GameState& state, NightManager& nightMgr, Audio& audio);
    bool Handle6AMInput(GameState& state, NightManager& nightMgr, Audio& audio);
    bool HandleWinScreenInput(GameState& state, Audio& audio);

    // Jumpscare helpers
    void StartJumpscare();
    bool IsJumpscareFinished() const { return m_jumpscareTimer > 1.6f; }
};

} // namespace FNAF
