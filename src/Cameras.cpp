#include "Cameras.h"
#include "Audio.h"
#include "AssetManager.h"
#include <raylib.h>
#include <cmath>
#include <algorithm>
#include <iostream>

namespace FNAF {

Cameras::Cameras() {
    SetupMapButtons();
    Reset();
}

void Cameras::LoadAssets(AssetManager& assets) {
    m_mapTex = assets.GetTexture("assets/images/cams/map.png");

    m_flipFrames.clear();
    for (int i = 1; i <= 10; ++i) {
        m_flipFrames.push_back(assets.GetTexture("assets/images/flip/" + std::to_string(i) + ".png"));
    }

    m_glitchFrames.clear();
    const char* glitches[] = { "430.png", "434.png", "435.png", "436.png", "437.png", "438.png", "439.png" };
    for (const char* g : glitches) {
        m_glitchFrames.push_back(assets.GetTexture(std::string("assets/images/cams/Glitch/") + g));
    }

    // Stage
    m_stageAll = assets.GetTexture("assets/images/cams/Stage/19.png");
    m_stageBonnieGone = assets.GetTexture("assets/images/cams/Stage/68.png");
    m_stageChicaGone = assets.GetTexture("assets/images/cams/Stage/224.png");
    m_stageEmpty = assets.GetTexture("assets/images/cams/Stage/484.png");

    // Dining
    m_diningEmpty = assets.GetTexture("assets/images/cams/Dining/48.png");
    m_diningBonnie = assets.GetTexture("assets/images/cams/Dining/90.png");
    m_diningChica = assets.GetTexture("assets/images/cams/Dining/120.png");
    m_diningFreddy = assets.GetTexture("assets/images/cams/Dining/215.png");

    // Cove
    m_coveClosed = assets.GetTexture("assets/images/cams/Cove/66.png");
    m_covePeeking = assets.GetTexture("assets/images/cams/Cove/211.png");
    m_coveOpen = assets.GetTexture("assets/images/cams/Cove/240.png");

    // West Hall & Corner
    m_westHallEmpty = assets.GetTexture("assets/images/cams/WestHall/336.png");
    m_westHallBonnie = assets.GetTexture("assets/images/cams/WestHall/206.png");
    m_westCornerEmpty = assets.GetTexture("assets/images/cams/WestHall/549.png");
    m_westCornerBonnie = assets.GetTexture("assets/images/cams/WestHall/188.png");

    // Closet
    m_closetEmpty = assets.GetTexture("assets/images/cams/Closet/62.png");
    m_closetBonnie = assets.GetTexture("assets/images/cams/Closet/190.png");

    // East Hall & Corner
    m_eastHallEmpty = assets.GetTexture("assets/images/cams/EastHall/67.png");
    m_eastHallChica = assets.GetTexture("assets/images/cams/EastHall/221.png");
    m_eastCornerEmpty = assets.GetTexture("assets/images/cams/EastHall/571.png");
    m_eastCornerChica = assets.GetTexture("assets/images/cams/EastHall/451.png");
    m_eastCornerFreddy = assets.GetTexture("assets/images/cams/EastHall/486.png");

    // Backstage
    m_backstageEmpty = assets.GetTexture("assets/images/cams/Backstage/354.png");
    m_backstageBonnie = assets.GetTexture("assets/images/cams/Backstage/205.png");

    // Restroom
    m_restroomEmpty = assets.GetTexture("assets/images/cams/Restroom/41.png");
    m_restroomChica = assets.GetTexture("assets/images/cams/Restroom/217.png");
    m_restroomFreddy = assets.GetTexture("assets/images/cams/Restroom/219.png");

    // Kitchen
    m_kitchenTex = assets.GetTexture("assets/images/cams/Kitchen/kitchen.png");
}

void Cameras::Reset() {
    m_currentCam = CameraID::CAM_1A;
    m_monitorUp = false;
    m_monitorProgress = 0.0f;
    m_staticTimer = 0.0f;
    m_switchStaticTimer = 0.0f;
    m_glitchFrame = 0;
}

void Cameras::SetupMapButtons() {
    m_mapButtons.clear();
    float bx = 900.0f;
    float by = 350.0f;

    m_mapButtons.push_back({ CameraID::CAM_1A, Rectangle{ bx + 110, by + 10,  55, 30 }, "1A" });
    m_mapButtons.push_back({ CameraID::CAM_1B, Rectangle{ bx + 110, by + 65,  55, 30 }, "1B" });
    m_mapButtons.push_back({ CameraID::CAM_5,  Rectangle{ bx + 15,  by + 70,  55, 30 }, "5"  });
    m_mapButtons.push_back({ CameraID::CAM_1C, Rectangle{ bx + 45,  by + 130, 55, 30 }, "1C" });
    m_mapButtons.push_back({ CameraID::CAM_7,  Rectangle{ bx + 275, by + 75,  55, 30 }, "7"  });
    m_mapButtons.push_back({ CameraID::CAM_6,  Rectangle{ bx + 275, by + 160, 55, 30 }, "6"  });
    m_mapButtons.push_back({ CameraID::CAM_2A, Rectangle{ bx + 80,  by + 200, 55, 30 }, "2A" });
    m_mapButtons.push_back({ CameraID::CAM_3,  Rectangle{ bx + 15,  by + 215, 55, 30 }, "3"  });
    m_mapButtons.push_back({ CameraID::CAM_2B, Rectangle{ bx + 80,  by + 250, 55, 30 }, "2B" });
    m_mapButtons.push_back({ CameraID::CAM_4A, Rectangle{ bx + 205, by + 200, 55, 30 }, "4A" });
    m_mapButtons.push_back({ CameraID::CAM_4B, Rectangle{ bx + 205, by + 250, 55, 30 }, "4B" });

    m_monitorFlipBar = Rectangle{ 340.0f, SCREEN_HEIGHT - 45.0f, 600.0f, 40.0f };
}

void Cameras::SetCurrentCamera(CameraID cam, Audio& audio) {
    if (m_currentCam != cam) {
        m_currentCam = cam;
        m_switchStaticTimer = 0.25f;
        audio.PlayCameraSwitch();
    }
}

void Cameras::ToggleMonitor(Audio& audio) {
    m_monitorUp = !m_monitorUp;
    m_switchStaticTimer = 0.15f;
    audio.PlayCameraFlip();
}

void Cameras::ForceMonitorDown() {
    m_monitorUp = false;
    m_monitorProgress = 0.0f;
}

void Cameras::HandleInput(Vector2 mousePos, bool powerDepleted, Audio& audio) {
    if (powerDepleted) {
        ForceMonitorDown();
        return;
    }

    if (IsKeyPressed(KEY_SPACE)) {
        ToggleMonitor(audio);
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(mousePos, m_monitorFlipBar)) {
        ToggleMonitor(audio);
    }

    if (m_monitorUp && m_monitorProgress > 0.8f) {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            for (const auto& btn : m_mapButtons) {
                if (CheckCollisionPointRec(mousePos, btn.rect)) {
                    SetCurrentCamera(btn.id, audio);
                    break;
                }
            }
        }

        if (IsKeyPressed(KEY_ONE))   SetCurrentCamera(CameraID::CAM_1A, audio);
        if (IsKeyPressed(KEY_TWO))   SetCurrentCamera(CameraID::CAM_1B, audio);
        if (IsKeyPressed(KEY_THREE)) SetCurrentCamera(CameraID::CAM_1C, audio);
        if (IsKeyPressed(KEY_FOUR))  SetCurrentCamera(CameraID::CAM_2A, audio);
        if (IsKeyPressed(KEY_FIVE))  SetCurrentCamera(CameraID::CAM_2B, audio);
        if (IsKeyPressed(KEY_SIX))   SetCurrentCamera(CameraID::CAM_3,  audio);
        if (IsKeyPressed(KEY_SEVEN)) SetCurrentCamera(CameraID::CAM_4A, audio);
        if (IsKeyPressed(KEY_EIGHT)) SetCurrentCamera(CameraID::CAM_4B, audio);
        if (IsKeyPressed(KEY_NINE))  SetCurrentCamera(CameraID::CAM_5,  audio);
    }
}

void Cameras::Update(float dt, bool powerDepleted, Audio& /*audio*/) {
    if (powerDepleted) {
        ForceMonitorDown();
        return;
    }

    float target = m_monitorUp ? 1.0f : 0.0f;
    m_monitorProgress += (target - m_monitorProgress) * std::min(1.0f, dt * 10.0f);

    m_camSweepTimer += dt * 0.4f;
    m_staticTimer += dt;
    if (m_switchStaticTimer > 0.0f) {
        m_switchStaticTimer -= dt;
    }

    if (!m_glitchFrames.empty()) {
        m_glitchFrame = (static_cast<int>(m_staticTimer * 15.0f)) % m_glitchFrames.size();
    }
}

void Cameras::Render(float /*officeCameraX*/,
                     LocationID bearLoc,
                     LocationID rabbitLoc,
                     LocationID chickenLoc,
                     FoxState foxState,
                     bool crtEnabled) {
    if (m_monitorProgress <= 0.01f) {
        Vector2 mousePos = GetVirtualMousePosition();
        bool hovering = CheckCollisionPointRec(mousePos, m_monitorFlipBar);
        Color barColor = hovering ? Color{ 80, 160, 220, 160 } : Color{ 40, 40, 50, 140 };
        DrawRectangleRec(m_monitorFlipBar, barColor);
        DrawRectangleLinesEx(m_monitorFlipBar, 2, hovering ? SKYBLUE : GRAY);
        DrawText("[ SPACE / CLICK ] TO OPEN CAMERA MONITOR", m_monitorFlipBar.x + 90, m_monitorFlipBar.y + 12, 18, WHITE);
        return;
    }

    // Monitor Flip Animation Frames if transitioning
    if (m_monitorProgress < 0.95f && !m_flipFrames.empty()) {
        int frameIdx = std::clamp(static_cast<int>(m_monitorProgress * (m_flipFrames.size() - 1)), 0, static_cast<int>(m_flipFrames.size() - 1));
        Texture2D flipTex = m_flipFrames[frameIdx];
        if (flipTex.id != 0) {
            Rectangle src{ 0, 0, (float)flipTex.width, (float)flipTex.height };
            Rectangle dst{ 0, 0, (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT };
            DrawTexturePro(flipTex, src, dst, Vector2{ 0, 0 }, 0.0f, WHITE);
            return;
        }
    }

    float monitorY = (1.0f - m_monitorProgress) * SCREEN_HEIGHT;

    // 1. Select Camera Background Texture based on animatronic state
    Texture2D camBg{};

    if (m_currentCam == CameraID::CAM_6) {
        // Kitchen: Audio Only
        DrawRectangle(0, monitorY, SCREEN_WIDTH, SCREEN_HEIGHT, BLACK);
        DrawText("- CAMERA DISABLED -", SCREEN_WIDTH / 2 - 160, monitorY + 280, 28, Color{ 200, 200, 60, 255 });
        DrawText("AUDIO ONLY", SCREEN_WIDTH / 2 - 80, monitorY + 330, 22, Color{ 180, 180, 180, 255 });
        float pulse = std::sin(m_staticTimer * 8.0f) * 15.0f;
        DrawCircle(SCREEN_WIDTH / 2, monitorY + 390, 25 + pulse, Color{ 100, 100, 120, 100 });
        DrawCircle(SCREEN_WIDTH / 2, monitorY + 390, 12, Color{ 220, 220, 240, 255 });
    } else {
        switch (m_currentCam) {
            case CameraID::CAM_1A: // Show Stage
                if (bearLoc != LocationID::ShowStage) {
                    camBg = m_stageEmpty;
                } else if (chickenLoc != LocationID::ShowStage && rabbitLoc != LocationID::ShowStage) {
                    camBg = m_stageChicaGone; // Only Freddy remains
                } else if (rabbitLoc != LocationID::ShowStage) {
                    camBg = m_stageBonnieGone; // Freddy and Chica remain
                } else {
                    camBg = m_stageAll; // All 3 on stage
                }
                break;

            case CameraID::CAM_1B: // Dining Area
                if (bearLoc == LocationID::DiningArea) camBg = m_diningFreddy;
                else if (chickenLoc == LocationID::DiningArea) camBg = m_diningChica;
                else if (rabbitLoc == LocationID::DiningArea) camBg = m_diningBonnie;
                else camBg = m_diningEmpty;
                break;

            case CameraID::CAM_1C: // Pirate Cove
                if (foxState == FoxState::Hidden) camBg = m_coveClosed;
                else if (foxState == FoxState::Watching) camBg = m_covePeeking;
                else camBg = m_coveOpen;
                break;

            case CameraID::CAM_2A: // West Hall
                if (rabbitLoc == LocationID::WestHall) camBg = m_westHallBonnie;
                else camBg = m_westHallEmpty;
                break;

            case CameraID::CAM_2B: // West Hall Corner
                if (rabbitLoc == LocationID::WestHallCorner) camBg = m_westCornerBonnie;
                else camBg = m_westCornerEmpty;
                break;

            case CameraID::CAM_3: // Supply Closet
                if (rabbitLoc == LocationID::SupplyCloset) camBg = m_closetBonnie;
                else camBg = m_closetEmpty;
                break;

            case CameraID::CAM_4A: // East Hall
                if (chickenLoc == LocationID::EastHall) camBg = m_eastHallChica;
                else camBg = m_eastHallEmpty;
                break;

            case CameraID::CAM_4B: // East Hall Corner
                if (bearLoc == LocationID::EastHallCorner) camBg = m_eastCornerFreddy;
                else if (chickenLoc == LocationID::EastHallCorner) camBg = m_eastCornerChica;
                else camBg = m_eastCornerEmpty;
                break;

            case CameraID::CAM_5: // Backstage
                if (rabbitLoc == LocationID::Backstage) camBg = m_backstageBonnie;
                else camBg = m_backstageEmpty;
                break;

            case CameraID::CAM_7: // Restroom
                if (bearLoc == LocationID::Restrooms) camBg = m_restroomFreddy;
                else if (chickenLoc == LocationID::Restrooms) camBg = m_restroomChica;
                else camBg = m_restroomEmpty;
                break;

            default:
                break;
        }

        if (camBg.id != 0) {
            float panT = (std::sin(m_camSweepTimer) + 1.0f) * 0.5f;
            float maxPan = std::max(0.0f, static_cast<float>(camBg.width) - static_cast<float>(SCREEN_WIDTH));
            float camPan = panT * maxPan;

            Rectangle src{ camPan, 0.0f, static_cast<float>(SCREEN_WIDTH), static_cast<float>(camBg.height) };
            Rectangle dst{ 0.0f, monitorY, static_cast<float>(SCREEN_WIDTH), static_cast<float>(SCREEN_HEIGHT) };
            DrawTexturePro(camBg, src, dst, Vector2{ 0, 0 }, 0.0f, WHITE);
        } else {
            DrawRectangle(0, monitorY, SCREEN_WIDTH, SCREEN_HEIGHT, Color{ 20, 22, 28, 255 });
        }
    }

    // 2. Glitch / Static Overlay
    if (!m_glitchFrames.empty()) {
        Texture2D gTex = m_glitchFrames[m_glitchFrame];
        if (gTex.id != 0) {
            float alpha = (m_switchStaticTimer > 0.0f) ? 0.85f : 0.25f;
            Rectangle src{ 0, 0, (float)gTex.width, (float)gTex.height };
            Rectangle dst{ 0, monitorY, (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT };
            DrawTexturePro(gTex, src, dst, Vector2{ 0, 0 }, 0.0f, ColorAlpha(WHITE, alpha));
        }
    }

    // 3. CRT Scanlines
    if (crtEnabled) {
        for (int y = static_cast<int>(monitorY); y < SCREEN_HEIGHT; y += 4) {
            DrawLine(0, y, SCREEN_WIDTH, y, Color{ 0, 0, 0, 45 });
        }
    }

    // 4. Camera Name & REC indicator
    DrawText(GetCameraName(m_currentCam), 60, monitorY + 50, 24, Color{ 220, 220, 220, 255 });
    bool recBlink = (std::fmod(m_staticTimer, 1.0f) < 0.5f);
    if (recBlink) {
        DrawCircle(SCREEN_WIDTH - 120, monitorY + 60, 10, RED);
        DrawText("REC", SCREEN_WIDTH - 100, monitorY + 50, 22, RED);
    }

    // 5. Interactive Surveillance Minimap in Bottom-Right
    float mapX = 900.0f;
    float mapY = monitorY + 340.0f;

    if (m_mapTex.id != 0) {
        DrawTexture(m_mapTex, mapX - 20, mapY - 20, WHITE);
    } else {
        DrawRectangle(mapX - 20, mapY - 20, 360, 360, Color{ 10, 12, 16, 220 });
        DrawRectangleLinesEx(Rectangle{ mapX - 20, mapY - 20, 360, 360 }, 2, Color{ 50, 60, 75, 255 });
    }

    Vector2 mousePos = GetVirtualMousePosition();
    for (const auto& btn : m_mapButtons) {
        Rectangle drawRect = btn.rect;
        drawRect.y += monitorY;

        bool isCurrent = (btn.id == m_currentCam);
        bool hovering = CheckCollisionPointRec(mousePos, drawRect);

        Color btnBg = isCurrent ? Color{ 50, 160, 70, 255 } : (hovering ? Color{ 80, 90, 110, 255 } : Color{ 35, 40, 50, 255 });
        DrawRectangleRec(drawRect, btnBg);
        DrawRectangleLinesEx(drawRect, isCurrent ? 2 : 1, isCurrent ? GREEN : GRAY);

        Color textCol = isCurrent ? WHITE : (hovering ? YELLOW : LIGHTGRAY);
        DrawText(btn.label, drawRect.x + 12, drawRect.y + 7, 16, textCol);
    }

    // Bottom close bar
    Rectangle closeBar = m_monitorFlipBar;
    closeBar.y = SCREEN_HEIGHT - 45.0f;
    bool hoveringClose = CheckCollisionPointRec(mousePos, closeBar);
    DrawRectangleRec(closeBar, hoveringClose ? Color{ 200, 70, 70, 160 } : Color{ 40, 40, 50, 140 });
    DrawRectangleLinesEx(closeBar, 2, hoveringClose ? RED : GRAY);
    DrawText("[ SPACE / CLICK ] TO CLOSE CAMERA MONITOR", closeBar.x + 85, closeBar.y + 12, 18, WHITE);
}

} // namespace FNAF
