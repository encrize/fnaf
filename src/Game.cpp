#include "Game.h"
#include "BundleManager.h"
#include <raylib.h>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <sstream>

namespace FNAF {

Game::Game() = default;

Game::~Game() {
    Shutdown();
}

bool Game::Initialize() {
    LogDebug("Game::Initialize: Setting window flags...");
    SetConfigFlags(FLAG_VSYNC_HINT);

    LogDebug("Game::Initialize: Calling InitWindow(1280, 720)...");
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Five Nights at Freddy's");
    if (!IsWindowReady()) {
        LogDebug("Game::Initialize: InitWindow failed! IsWindowReady() returned false (OpenGL context could not be created).");
        return false;
    }
    LogDebug("Game::Initialize: Window and OpenGL context ready.");
    SetTargetFPS(TARGET_FPS);

    LogDebug("Game::Initialize: Initializing assets and audio...");
    m_assets.Initialize();
    m_audio.Initialize();
    m_settings.Apply();
    m_nightManager.Initialize();

    m_office.LoadAssets(m_assets);
    m_cameras.LoadAssets(m_assets);
    m_ui.LoadAssets(m_assets);

    LogDebug("Game::Initialize: Creating render target...");
    m_screenTarget = LoadRenderTexture(SCREEN_WIDTH, SCREEN_HEIGHT);
    SetTextureFilter(m_screenTarget.texture, TEXTURE_FILTER_BILINEAR);

    m_audio.PlayAmbient(true);

    LogDebug("Game::Initialize: Initialization complete.");
    return true;
}

Vector2 Game::GetVirtualMousePosition() const {
    return FNAF::GetVirtualMousePosition();
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
    if (m_nightManager.GetCurrentNight() <= 5) {
        m_audio.PlayPhoneCall(m_nightManager.GetCurrentNight());
    }
}

void Game::ProcessInput() {
    // Borderless fullscreen toggle (window size == monitor size, no WM resize games)
    if (IsKeyPressed(KEY_F11)) {
        ToggleBorderlessWindowed();
    }

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

    Vector2 vMouse = GetVirtualMousePosition();

    // State specific input
    switch (m_state) {
        case GameState::MainMenu:
            if (!m_ui.HandleMainMenuInput(m_state, m_nightManager, m_settings, m_audio, vMouse)) {
                m_running = false;
            }
            if (m_state == GameState::NightIntro) {
                m_nightIntroTimer = 0.0f;
            }
            break;

        case GameState::SettingsMenu:
            m_ui.HandleSettingsInput(m_state, m_settings, m_audio, vMouse);
            break;

        case GameState::CustomNightSelect:
            m_ui.HandleCustomNightInput(m_state, m_nightManager, m_audio, vMouse);
            if (m_state == GameState::NightIntro) {
                m_nightIntroTimer = 0.0f;
            }
            break;

        case GameState::NightIntro: {
            float maxIntroTime = (m_nightManager.GetCurrentNight() == 1) ? 5.0f : 2.5f;
            if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ENTER) || m_nightIntroTimer > maxIntroTime) {
                StartNightSession();
                m_state = GameState::Playing;
            }
            break;
        }

        case GameState::Playing: {
            bool powerDepleted = m_power.IsDepleted();

            // Mute / Skip phone call
            Rectangle muteBtn{ 40.0f, 40.0f, 120.0f, 32.0f };
            if (m_nightManager.IsPhoneCallActive() &&
                ((IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(vMouse, muteBtn)) || IsKeyPressed(KEY_M))) {
                m_nightManager.SkipPhoneCall();
                m_audio.StopPhoneCall();
            }

            // Input routing to Office and Cameras
            m_cameras.HandleInput(vMouse, powerDepleted, m_audio);
            m_office.HandleInput(vMouse,
                                 m_cameras.IsMonitorUp(),
                                 powerDepleted,
                                 m_animatronics.IsRabbitAtDoor(),
                                 m_animatronics.IsChickenAtDoor(),
                                 m_audio);

            if (IsKeyPressed(KEY_ESCAPE)) {
                m_audio.StopAllSounds();
                m_state = GameState::MainMenu;
                m_audio.PlayAmbient(true);
            }
            break;
        }

        case GameState::GameOver:
            m_ui.HandleGameOverInput(m_state, m_nightManager, m_audio, vMouse);
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
            m_ui.HandleWinScreenInput(m_state, m_audio, vMouse);
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
                m_ui.Reset();
                m_state = GameState::GameOver;
            }
            break;

        default:
            break;
    }
}

void Game::Render() {
    // 1. Render all game content into virtual 1280x720 render target
    BeginTextureMode(m_screenTarget);
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
            m_ui.RenderNightIntro(m_nightManager.GetCurrentNight(), m_nightIntroTimer, m_assets);
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

            // Render phone call mute button if active and monitor is down
            if (m_nightManager.IsPhoneCallActive() && !m_cameras.IsMonitorUp()) {
                m_ui.RenderPhoneMuteButton(GetVirtualMousePosition());
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

    EndTextureMode();

    // 2. Draw scaled render target with letterboxing/pillarboxing maintaining 16:9 aspect ratio
    BeginDrawing();
    ClearBackground(BLACK);

    const GameViewport vp = GetGameViewport();
    const float scale = vp.scale;
    const float offsetX = vp.offsetX;
    const float offsetY = vp.offsetY;

    // Flip texture vertically (render textures in OpenGL / raylib have inverted Y)
    Rectangle srcRec = { 0.0f, 0.0f, static_cast<float>(m_screenTarget.texture.width), -static_cast<float>(m_screenTarget.texture.height) };
    Rectangle dstRec = { offsetX, offsetY, static_cast<float>(SCREEN_WIDTH) * scale, static_cast<float>(SCREEN_HEIGHT) * scale };

    DrawTexturePro(m_screenTarget.texture, srcRec, dstRec, Vector2{ 0, 0 }, 0.0f, WHITE);

    // F10: mouse mapping diagnostics (drawn in window space, after the blit)
    static bool s_mouseDiag = false;
    if (IsKeyPressed(KEY_F10)) s_mouseDiag = !s_mouseDiag;
    if (s_mouseDiag) {
        Vector2 raw = GetMousePosition();
        Vector2 v = GetVirtualMousePositionRaw();
        Vector2 back = { v.x * scale + offsetX, v.y * scale + offsetY }; // virtual -> window
        Vector2 dpi = GetWindowScaleDPI();
        DrawRectangle(0, 0, 760, 74, Color{ 0, 0, 0, 200 });
        DrawText(TextFormat("screen %dx%d  render %dx%d  dpi %.2f,%.2f  scale %.3f",
                            GetScreenWidth(), GetScreenHeight(), GetRenderWidth(), GetRenderHeight(),
                            dpi.x, dpi.y, scale), 8, 6, 18, RED);
        DrawText(TextFormat("raw %.0f,%.0f  virtual %.0f,%.0f  off %.0f,%.0f",
                            raw.x, raw.y, v.x, v.y, offsetX, offsetY), 8, 30, 18, RED);
        DrawText("blue = raw cursor, green = virtual mapped back (must overlap)", 8, 52, 16, WHITE);
        DrawCircleLines(static_cast<int>(raw.x), static_cast<int>(raw.y), 8, BLUE);
        DrawCircleLines(static_cast<int>(back.x), static_cast<int>(back.y), 12, GREEN);
    }

    EndDrawing();
}

void Game::RenderDebugOverlay() {
    DrawRectangle(10, 10, 440, 310, Color{ 0, 0, 0, 210 });
    DrawRectangleLinesEx(Rectangle{ 10, 10, 440, 310 }, 1, GREEN);

    DrawText("=== DEBUG MODE (F1 to Toggle | F2 Log Mem) ===", 20, 16, 14, GREEN);

    long rssKb = AssetManager::GetCurrentVmRSSKb();
    size_t texBytes = m_assets.GetTotalTextureBytes();
    size_t texCount = m_assets.GetLoadedTextureCount();
    std::string memStr = "VmRSS: " + std::to_string(rssKb / 1024) + " MB | TexMem: " + 
                         std::to_string(texBytes / (1024 * 1024)) + " MB (" + std::to_string(texCount) + " tex)";
    DrawText(memStr.c_str(), 20, 38, 14, ORANGE);

    std::string pStr = "Power: " + std::to_string(static_cast<int>(m_power.GetPower())) + "% | Usage: " + std::to_string(m_power.GetUsageLevel());
    DrawText(pStr.c_str(), 20, 58, 14, YELLOW);

    std::string bStr = "Bear AI: " + std::to_string(m_animatronics.GetBearAI()) + " | Loc: " + GetLocationName(m_animatronics.GetBearLocation());
    DrawText(bStr.c_str(), 20, 78, 14, WHITE);

    std::string rStr = "Rabbit AI: " + std::to_string(m_animatronics.GetRabbitAI()) + " | Loc: " + GetLocationName(m_animatronics.GetRabbitLocation());
    DrawText(rStr.c_str(), 20, 98, 14, WHITE);

    std::string cStr = "Chicken AI: " + std::to_string(m_animatronics.GetChickenAI()) + " | Loc: " + GetLocationName(m_animatronics.GetChickenLocation());
    DrawText(cStr.c_str(), 20, 118, 14, WHITE);

    std::string fStr = "Fox AI: " + std::to_string(m_animatronics.GetFoxAI()) + " | State: " + std::to_string(static_cast<int>(m_animatronics.GetFoxState()));
    DrawText(fStr.c_str(), 20, 138, 14, WHITE);

    DrawText("Keys: [1] Move Rabbit  [2] Move Chicken", 20, 170, 13, SKYBLUE);
    DrawText("      [3] Move Bear    [4] Advance Fox", 20, 190, 13, SKYBLUE);
    DrawText("      [P] Drain Power  [G] Golden Bear", 20, 210, 13, SKYBLUE);
    DrawText("      [N] Skip to 6AM  [J] Test Jumpscare", 20, 230, 13, SKYBLUE);
    DrawText("      [Q/E] Doors      [Z/C] Lights", 20, 250, 13, SKYBLUE);
    DrawText("      [F2] Dump Detailed Texture Memory to Log", 20, 275, 13, GREEN);

    if (IsKeyPressed(KEY_F2)) {
        m_assets.LogMemoryReport("Interactive F2 Debug");
    }
}

void Game::RunMeasurement() {
    std::cout << "\n=======================================================\n";
    std::cout << "        RUNNING COMPREHENSIVE MEMORY MEASUREMENT       \n";
    std::cout << "=======================================================\n";

    // 1. Main Menu
    m_state = GameState::MainMenu;
    for (int i = 0; i < 30; ++i) {
        Update(1.0f / 60.0f);
        Render();
    }
    m_assets.LogMemoryReport("Main Menu");

    // 2. Office (Playing)
    m_state = GameState::Playing;
    StartNightSession();
    for (int i = 0; i < 30; ++i) {
        Update(1.0f / 60.0f);
        Render();
    }
    m_assets.LogMemoryReport("Office");

    // 3. Cameras Open
    if (!m_cameras.IsMonitorUp()) {
        m_cameras.ToggleMonitor(m_audio);
    }
    m_cameras.SetCurrentCamera(CameraID::CAM_1A, m_audio);
    for (int i = 0; i < 15; ++i) {
        Update(1.0f / 60.0f);
        Render();
    }
    m_cameras.SetCurrentCamera(CameraID::CAM_1B, m_audio);
    for (int i = 0; i < 15; ++i) {
        Update(1.0f / 60.0f);
        Render();
    }
    m_assets.LogMemoryReport("Cameras Open");

    // 4. Jumpscare
    if (m_cameras.IsMonitorUp()) {
        m_cameras.ToggleMonitor(m_audio);
    }
    m_activeJumpscareAttacker = AnimatronicID::Bear;
    m_ui.StartJumpscare();
    m_state = GameState::Jumpscare;
    for (int i = 0; i < 30; ++i) {
        Update(1.0f / 60.0f);
        Render();
    }
    m_assets.LogMemoryReport("Jumpscare");

    // 5. Post-Jumpscare (Unload test)
    m_ui.Reset();
    m_assets.LogMemoryReport("Post-Jumpscare (Unloaded)");

    std::cout << "\n[MEASUREMENT COMPLETE]\n" << std::flush;
}

void Game::Run() {
    while (!WindowShouldClose() && m_running) {
        ProcessInput();
        Update(GetFrameTime());
        Render();
    }
}

void Game::Shutdown() {
    if (m_screenTarget.id != 0) {
        UnloadRenderTexture(m_screenTarget);
    }
    m_audio.Cleanup();
    m_assets.Cleanup();
    if (IsWindowReady()) {
        CloseWindow();
    }
}

} // namespace FNAF
