#include "Game.h"
#include <raylib.h>
#include <iostream>
#include <iomanip>
#include <sstream>

namespace FNAF {

Game::Game() = default;

Game::~Game() {
    Shutdown();
}

bool Game::Initialize() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Five Nights at Freddy's - C++ Fan Recreation");
    SetTargetFPS(TARGET_FPS);

    m_assets.Initialize();
    m_audio.Initialize();
    m_settings.Apply();
    m_nightManager.Initialize();

    m_audio.PlayAmbient(true);

    return true;
}

void Game::StartNightSession() {
    m_office.Reset();
    m_cameras.Reset();
    m_power.Reset();
    m_ui.Reset();

    const auto& diff = m_nightManager.GetCurrentDifficulty();
    m_animatronics.Reset(diff.bearAI, diff.rabbitAI, diff.chickenAI, diff.foxAI, m_settings.rareVisitorEnabled);

    m_audio.StopAllSounds();
    m_audio.PlayAmbient(true);
    m_audio.PlayFanHum(true);
}

void Game::ProcessInput() {
    // Debug toggle
    if (IsKeyPressed(KEY_F1)) {
        m_debugMode = !m_debugMode;
        m_audio.PlayBlip();
    }

    // Debug cheats
    if (m_debugMode) {
        if (IsKeyPressed(KEY_P)) {
            m_power.DepleteInstantly();
        }
        if (IsKeyPressed(KEY_N)) {
            m_nightManager.SetHourDuration(0.01f); // fast forward to 6 AM
        }
        if (IsKeyPressed(KEY_G)) {
            m_animatronics.TriggerGoldenBear();
        }
        if (IsKeyPressed(KEY_J)) {
            m_activeJumpscareAttacker = AnimatronicID::Bear;
            m_ui.StartJumpscare();
            m_state = GameState::Jumpscare;
        }
        if (IsKeyPressed(KEY_ONE))   m_animatronics.ForceMove(AnimatronicID::Rabbit);
        if (IsKeyPressed(KEY_TWO))   m_animatronics.ForceMove(AnimatronicID::Chicken);
        if (IsKeyPressed(KEY_THREE)) m_animatronics.ForceMove(AnimatronicID::Bear);
        if (IsKeyPressed(KEY_FOUR))  m_animatronics.ForceMove(AnimatronicID::Fox);
    }

    // State specific input
    switch (m_state) {
        case GameState::MainMenu:
            if (!m_ui.HandleMainMenuInput(m_state, m_nightManager, m_settings, m_audio)) {
                m_running = false;
            }
            if (m_state == GameState::NightIntro) {
                m_nightIntroTimer = 0.0f;
            }
            break;

        case GameState::SettingsMenu:
            m_ui.HandleSettingsInput(m_state, m_settings, m_audio);
            break;

        case GameState::CustomNightSelect:
            m_ui.HandleCustomNightInput(m_state, m_nightManager, m_audio);
            if (m_state == GameState::NightIntro) {
                m_nightIntroTimer = 0.0f;
            }
            break;

        case GameState::NightIntro:
            if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ENTER) || m_nightIntroTimer > 2.5f) {
                StartNightSession();
                m_state = GameState::Playing;
            }
            break;

        case GameState::Playing: {
            bool monitorUp = m_cameras.IsMonitorUp();
            bool powerDepleted = m_power.IsDepleted();

            // Mute / Skip phone call
            if (IsKeyPressed(KEY_M) || IsKeyPressed(KEY_ENTER)) {
                m_nightManager.SkipPhoneCall();
            }

            // Input routing to Office and Cameras
            m_cameras.HandleInput(powerDepleted, m_audio);
            m_office.HandleInput(monitorUp, powerDepleted, m_audio);

            if (IsKeyPressed(KEY_ESCAPE)) {
                m_audio.StopAllSounds();
                m_state = GameState::MainMenu;
                m_audio.PlayAmbient(true);
            }
            break;
        }

        case GameState::GameOver:
            m_ui.HandleGameOverInput(m_state, m_nightManager, m_audio);
            if (m_state == GameState::Playing) {
                StartNightSession();
            }
            break;

        case GameState::NightComplete:
            m_ui.Handle6AMInput(m_state, m_nightManager, m_audio);
            if (m_state == GameState::Playing) {
                StartNightSession();
            }
            break;

        case GameState::Win:
            m_ui.HandleWinScreenInput(m_state, m_audio);
            break;

        default:
            break;
    }
}

void Game::Update(float dt) {
    m_audio.Update();

    switch (m_state) {
        case GameState::NightIntro:
            m_nightIntroTimer += dt;
            break;

        case GameState::Playing: {
            bool leftDoor = m_office.IsLeftDoorClosed();
            bool rightDoor = m_office.IsRightDoorClosed();
            bool leftLight = m_office.IsLeftLightActive();
            bool rightLight = m_office.IsRightLightActive();
            bool monitorUp = m_cameras.IsMonitorUp();

            // 1. Update Power System
            m_power.Update(dt, leftDoor, rightDoor, leftLight, rightLight, monitorUp, m_audio);

            // 2. Update Office & Camera animations
            m_office.Update(dt, m_power.IsDepleted(), m_audio);
            m_cameras.Update(dt, m_power.IsDepleted(), m_audio);

            // 3. Update Night Manager & clock
            m_nightManager.Update(dt);
            if (m_nightManager.IsNightComplete()) {
                m_audio.StopAllSounds();
                m_ui.Reset();
                m_state = GameState::NightComplete;
                return;
            }

            // 4. Update Animatronics AI
            m_animatronics.Update(dt,
                                  monitorUp,
                                  m_cameras.GetCurrentCamera(),
                                  leftDoor,
                                  rightDoor,
                                  m_audio,
                                  m_power);

            // Check attack jumpscare
            if (m_animatronics.IsJumpscareReady()) {
                m_activeJumpscareAttacker = m_animatronics.GetJumpscareAttacker();
                m_audio.StopAllSounds();
                m_ui.StartJumpscare();
                m_state = GameState::Jumpscare;
                return;
            }

            // Check blackout power-out jumpscare
            if (m_power.IsJumpscareReady()) {
                m_activeJumpscareAttacker = AnimatronicID::Bear;
                m_audio.StopAllSounds();
                m_ui.StartJumpscare();
                m_state = GameState::Jumpscare;
                return;
            }

            break;
        }

        case GameState::Jumpscare:
            if (m_ui.IsJumpscareFinished()) {
                m_state = GameState::GameOver;
            }
            break;

        default:
            break;
    }
}

void Game::Render() {
    BeginDrawing();
    ClearBackground(BLACK);

    switch (m_state) {
        case GameState::MainMenu:
            m_ui.RenderMainMenu(m_nightManager.GetHighestUnlockedNight(),
                                (m_nightManager.GetHighestUnlockedNight() >= 5),
                                m_settings,
                                m_audio);
            break;

        case GameState::SettingsMenu:
            m_ui.RenderSettingsMenu(m_settings, m_audio);
            break;

        case GameState::CustomNightSelect:
            m_ui.RenderCustomNightMenu(m_nightManager, m_audio);
            break;

        case GameState::NightIntro: {
            ClearBackground(BLACK);
            std::string nStr = (m_nightManager.GetCurrentNight() == 7) ? "CUSTOM NIGHT" :
                               ("NIGHT " + std::to_string(m_nightManager.GetCurrentNight()));
            DrawText(nStr.c_str(), SCREEN_WIDTH / 2 - 120, SCREEN_HEIGHT / 2 - 40, 48, WHITE);
            DrawText("12:00 AM", SCREEN_WIDTH / 2 - 60, SCREEN_HEIGHT / 2 + 30, 26, Color{ 160, 160, 170, 255 });
            DrawText("Press [ SPACE ] to Skip", SCREEN_WIDTH / 2 - 110, SCREEN_HEIGHT - 60, 18, GRAY);
            break;
        }

        case GameState::Playing: {
            // Render 3D-perspective Office
            m_office.Render(m_animatronics.IsRabbitAtDoor(),
                            m_animatronics.IsChickenAtDoor(),
                            m_power.IsBearFlickering(),
                            m_animatronics.IsRareVisitorActive(),
                            m_power.IsDepleted());

            // Render Camera monitor feed & UI
            m_cameras.Render(m_office.GetCameraX(),
                             m_animatronics.GetBearLocation(),
                             m_animatronics.GetRabbitLocation(),
                             m_animatronics.GetChickenLocation(),
                             m_animatronics.GetFoxState(),
                             m_settings.crtEffects);

            // Render HUD
            m_ui.RenderHUD(m_nightManager, m_power, m_cameras.IsMonitorUp());

            // Render phone call subtitles if active
            if (m_nightManager.IsPhoneCallActive()) {
                m_ui.RenderPhoneSubtitles(m_nightManager.GetCurrentPhoneSubtitles());
            }

            // CRT Scanlines across screen if enabled
            if (m_settings.crtEffects) {
                for (int y = 0; y < SCREEN_HEIGHT; y += 4) {
                    DrawLine(0, y, SCREEN_WIDTH, y, Color{ 0, 0, 0, 30 });
                }
            }
            break;
        }

        case GameState::Jumpscare:
            m_ui.RenderJumpscare(m_activeJumpscareAttacker, GetFrameTime(), m_audio);
            break;

        case GameState::GameOver:
            m_ui.RenderGameOver(GetFrameTime());
            break;

        case GameState::NightComplete:
            m_ui.Render6AM(GetFrameTime(), m_audio);
            break;

        case GameState::Win:
            m_ui.RenderWinScreen();
            break;
    }

    if (m_debugMode) {
        RenderDebugOverlay();
    }

    EndDrawing();
}

void Game::RenderDebugOverlay() {
    DrawRectangle(10, 10, 420, 260, Color{ 0, 0, 0, 200 });
    DrawRectangleLinesEx(Rectangle{ 10, 10, 420, 260 }, 1, GREEN);

    DrawText("=== DEBUG MODE (F1 to Toggle) ===", 20, 16, 14, GREEN);

    std::string pStr = "Power: " + std::to_string(static_cast<int>(m_power.GetPower())) + "% | Usage: " + std::to_string(m_power.GetUsageLevel());
    DrawText(pStr.c_str(), 20, 38, 14, YELLOW);

    std::string bStr = "Bear AI: " + std::to_string(m_animatronics.GetBearAI()) + " | Loc: " + GetLocationName(m_animatronics.GetBearLocation());
    DrawText(bStr.c_str(), 20, 58, 14, WHITE);

    std::string rStr = "Rabbit AI: " + std::to_string(m_animatronics.GetRabbitAI()) + " | Loc: " + GetLocationName(m_animatronics.GetRabbitLocation());
    DrawText(rStr.c_str(), 20, 78, 14, WHITE);

    std::string cStr = "Chicken AI: " + std::to_string(m_animatronics.GetChickenAI()) + " | Loc: " + GetLocationName(m_animatronics.GetChickenLocation());
    DrawText(cStr.c_str(), 20, 98, 14, WHITE);

    std::string fStr = "Fox AI: " + std::to_string(m_animatronics.GetFoxAI()) + " | State: " + std::to_string(static_cast<int>(m_animatronics.GetFoxState()));
    DrawText(fStr.c_str(), 20, 118, 14, WHITE);

    DrawText("Keys: [1] Move Rabbit  [2] Move Chicken", 20, 150, 13, SKYBLUE);
    DrawText("      [3] Move Bear    [4] Advance Fox", 20, 170, 13, SKYBLUE);
    DrawText("      [P] Drain Power  [G] Golden Bear", 20, 190, 13, SKYBLUE);
    DrawText("      [N] Skip to 6AM  [J] Test Jumpscare", 20, 210, 13, SKYBLUE);
    DrawText("      [Q/E] Doors      [Z/C] Lights", 20, 230, 13, SKYBLUE);
}

void Game::Run() {
    while (!WindowShouldClose() && m_running) {
        ProcessInput();
        Update(GetFrameTime());
        Render();
    }
}

void Game::Shutdown() {
    m_audio.Cleanup();
    m_assets.Cleanup();
    if (IsWindowReady()) {
        CloseWindow();
    }
}

} // namespace FNAF
