#pragma once

#include <memory>
#include <string>
#include "Common.h"
#include "AssetManager.h"
#include "Audio.h"
#include "Settings.h"
#include "NightManager.h"
#include "Office.h"
#include "Cameras.h"
#include "Power.h"
#include "Animatronic.h"
#include "UI.h"

namespace FNAF {

class Game {
private:
    GameState m_state = GameState::MainMenu;
    bool m_running = true;

    // Subsystems
    AssetManager m_assets;
    Audio m_audio;
    Settings m_settings;
    NightManager m_nightManager;
    Office m_office;
    Cameras m_cameras;
    Power m_power;
    AnimatronicController m_animatronics;
    UI m_ui;

    // Transition & Intro timers
    float m_nightIntroTimer = 0.0f;
    AnimatronicID m_activeJumpscareAttacker = AnimatronicID::Bear;

    // Render target for resolution scaling and aspect ratio letterboxing
    RenderTexture2D m_screenTarget{};

    Vector2 GetVirtualMousePosition() const;

    // Debug Mode
    bool m_debugMode = false;

    void ProcessInput();
    void Update(float dt);
    void Render();
    void RenderDebugOverlay();

    void StartNightSession();

public:
    Game();
    ~Game();

    // Prevent copies
    Game(const Game&) = delete;
    Game& operator=(const Game&) = delete;

    bool Initialize();
    void Run();
    void Shutdown();

    bool IsRunning() const { return m_running; }
};

} // namespace FNAF
