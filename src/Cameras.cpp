#include "Cameras.h"
#include "Audio.h"
#include "AssetManager.h"
#include <raylib.h>
#include <cmath>
#include <iostream>

namespace FNAF {

Cameras::Cameras() {
    SetupMapButtons();
    Reset();
}

void Cameras::LoadAssets(AssetManager& assets) {
    m_camTextures[CameraID::CAM_1A] = assets.GetTexture("assets/images/cam_1a.png");
    m_camTextures[CameraID::CAM_1B] = assets.GetTexture("assets/images/cam_1b.png");
    m_camTextures[CameraID::CAM_1C] = assets.GetTexture("assets/images/cam_1c.png");
    m_camTextures[CameraID::CAM_2A] = assets.GetTexture("assets/images/cam_2a.png");
    m_camTextures[CameraID::CAM_3]  = assets.GetTexture("assets/images/cam_3.png");
    m_camTextures[CameraID::CAM_4A] = assets.GetTexture("assets/images/cam_4a.png");
    m_camTextures[CameraID::CAM_5]  = assets.GetTexture("assets/images/cam_5.png");
    m_camTextures[CameraID::CAM_7]  = assets.GetTexture("assets/images/cam_7.png");
}

void Cameras::Reset() {
    m_currentCam = CameraID::CAM_1A;
    m_monitorUp = false;
    m_monitorProgress = 0.0f;
    m_staticTimer = 0.0f;
    m_switchStaticTimer = 0.0f;
}

void Cameras::SetupMapButtons() {
    m_mapButtons.clear();
    // Layout in bottom right: Base origin (900, 360)
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

    // Monitor flip hover bar at bottom of screen
    m_monitorFlipBar = Rectangle{ 340.0f, SCREEN_HEIGHT - 45.0f, 600.0f, 40.0f };
}

void Cameras::SetCurrentCamera(CameraID cam, Audio& audio) {
    if (m_currentCam != cam) {
        m_currentCam = cam;
        m_switchStaticTimer = 0.25f; // burst of static
        audio.PlayCameraSwitch();
    }
}

void Cameras::ToggleMonitor(Audio& audio) {
    m_monitorUp = !m_monitorUp;
    m_switchStaticTimer = 0.15f;
    audio.PlayCameraSwitch();
}

void Cameras::ForceMonitorDown() {
    m_monitorUp = false;
    m_monitorProgress = 0.0f;
}

void Cameras::HandleInput(bool powerDepleted, Audio& audio) {
    if (powerDepleted) {
        ForceMonitorDown();
        return;
    }

    // Toggle monitor with SPACE or clicking the bottom flip bar
    if (IsKeyPressed(KEY_SPACE)) {
        ToggleMonitor(audio);
    }

    Vector2 mousePos = GetMousePosition();
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(mousePos, m_monitorFlipBar)) {
        ToggleMonitor(audio);
    }

    // Camera switching via map buttons or number keys
    if (m_monitorUp && m_monitorProgress > 0.8f) {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            for (const auto& btn : m_mapButtons) {
                if (CheckCollisionPointRec(mousePos, btn.rect)) {
                    SetCurrentCamera(btn.id, audio);
                    break;
                }
            }
        }

        // Hotkeys 1 to 9
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

    // Monitor flip animation (opens/closes in ~0.2s)
    float target = m_monitorUp ? 1.0f : 0.0f;
    m_monitorProgress += (target - m_monitorProgress) * std::min(1.0f, dt * 10.0f);

    m_staticTimer += dt;
    if (m_switchStaticTimer > 0.0f) {
        m_switchStaticTimer -= dt;
    }

    // Audio cue for kitchen if Bear or Chicken is in Kitchen
    // (Kitchen is handled through Game update)
}

void Cameras::Render(float /*officeCameraX*/,
                     LocationID bearLoc,
                     LocationID rabbitLoc,
                     LocationID chickenLoc,
                     FoxState foxState,
                     bool crtEnabled) {
    // If monitor is completely down, only render the flip bar
    if (m_monitorProgress <= 0.01f) {
        // Bottom flip bar
        Vector2 mousePos = GetMousePosition();
        bool hovering = CheckCollisionPointRec(mousePos, m_monitorFlipBar);
        Color barColor = hovering ? Color{ 80, 160, 220, 160 } : Color{ 40, 40, 50, 140 };
        DrawRectangleRec(m_monitorFlipBar, barColor);
        DrawRectangleLinesEx(m_monitorFlipBar, 2, hovering ? SKYBLUE : GRAY);
        DrawText("[ SPACE / CLICK ] TO OPEN CAMERA MONITOR", m_monitorFlipBar.x + 90, m_monitorFlipBar.y + 12, 18, WHITE);
        return;
    }

    // Render monitor screen sliding up from bottom
    float monitorY = (1.0f - m_monitorProgress) * SCREEN_HEIGHT;

    // Monitor Background Frame
    DrawRectangle(0, monitorY, SCREEN_WIDTH, SCREEN_HEIGHT, Color{ 12, 14, 18, 255 });

    // Room Rendering based on current active camera
    if (m_currentCam == CameraID::CAM_6) {
        // Kitchen: Audio Only Mode!
        DrawRectangle(0, monitorY, SCREEN_WIDTH, SCREEN_HEIGHT, BLACK);
        DrawText("- CAMERA DISABLED -", SCREEN_WIDTH / 2 - 160, monitorY + 280, 28, Color{ 200, 200, 60, 255 });
        DrawText("AUDIO ONLY", SCREEN_WIDTH / 2 - 80, monitorY + 330, 22, Color{ 180, 180, 180, 255 });

        // Audio visualizer pulses
        float pulse = std::sin(m_staticTimer * 8.0f) * 15.0f;
        DrawCircle(SCREEN_WIDTH / 2, monitorY + 390, 25 + pulse, Color{ 100, 100, 120, 100 });
        DrawCircle(SCREEN_WIDTH / 2, monitorY + 390, 12, Color{ 220, 220, 240, 255 });
    } else {
        // Render stylized room environment or loaded texture background
        float rw = SCREEN_WIDTH;
        float rh = SCREEN_HEIGHT;

        auto it = m_camTextures.find(m_currentCam);
        if (it != m_camTextures.end() && it->second.id != 0) {
            DrawTexture(it->second, 0, monitorY, WHITE);
        } else {
            // Base room walls & floor fallback
            DrawRectangle(0, monitorY, rw, rh * 0.65f, Color{ 24, 26, 32, 255 });
            DrawRectangle(0, monitorY + rh * 0.65f, rw, rh * 0.35f, Color{ 15, 16, 20, 255 });
        }

        // Room specific geometry and animatronics
        switch (m_currentCam) {
            case CameraID::CAM_1A: { // Show Stage
                // Star backdrop & wooden stage
                DrawRectangle(140, monitorY + 80, rw - 280, rh * 0.5f, Color{ 35, 20, 45, 255 });
                DrawRectangle(100, monitorY + rh * 0.45f, rw - 200, 140, Color{ 55, 40, 25, 255 });
                DrawText("SHOW STAGE", 160, monitorY + 100, 20, Color{ 120, 100, 140, 180 });

                // Animatronics on stage
                // Rabbit (Left)
                if (rabbitLoc == LocationID::ShowStage) {
                    float bx = 320.0f;
                    float by = monitorY + 200.0f;
                    DrawRectangle(bx + 10, by + 60, 60, 130, Color{ 60, 50, 90, 255 });
                    DrawCircle(bx + 40, by + 40, 28, Color{ 70, 60, 100, 255 });
                    DrawRectangle(bx + 24, by - 25, 10, 45, Color{ 70, 60, 100, 255 });
                    DrawRectangle(bx + 46, by - 25, 10, 45, Color{ 70, 60, 100, 255 });
                    DrawCircle(bx + 32, by + 36, 4, Color{ 220, 220, 240, 255 });
                    DrawCircle(bx + 48, by + 36, 4, Color{ 220, 220, 240, 255 });
                }

                // Bear (Center)
                if (bearLoc == LocationID::ShowStage) {
                    float bx = 580.0f;
                    float by = monitorY + 180.0f;
                    DrawRectangle(bx + 10, by + 70, 80, 140, Color{ 75, 45, 25, 255 });
                    DrawCircle(bx + 50, by + 45, 34, Color{ 85, 55, 30, 255 });
                    // Bear ears & top hat
                    DrawCircle(bx + 20, by + 25, 12, Color{ 85, 55, 30, 255 });
                    DrawCircle(bx + 80, by + 25, 12, Color{ 85, 55, 30, 255 });
                    DrawRectangle(bx + 35, by - 10, 30, 25, BLACK);
                    DrawRectangle(bx + 25, by + 12, 50, 6, BLACK);
                    // Glowing eyes
                    DrawCircle(bx + 40, by + 42, 5, Color{ 220, 240, 255, 255 });
                    DrawCircle(bx + 60, by + 42, 5, Color{ 220, 240, 255, 255 });
                }

                // Chicken (Right)
                if (chickenLoc == LocationID::ShowStage) {
                    float bx = 840.0f;
                    float by = monitorY + 205.0f;
                    DrawRectangle(bx + 10, by + 60, 60, 125, Color{ 85, 75, 30, 255 });
                    DrawCircle(bx + 40, by + 40, 28, Color{ 95, 85, 35, 255 });
                    DrawTriangle(Vector2{ bx + 40, by + 50 }, Vector2{ bx + 32, by + 42 }, Vector2{ bx + 48, by + 42 }, Color{ 220, 120, 20, 255 });
                    DrawCircle(bx + 32, by + 34, 4, Color{ 240, 240, 250, 255 });
                    DrawCircle(bx + 48, by + 34, 4, Color{ 240, 240, 250, 255 });
                }
                break;
            }

            case CameraID::CAM_1C: { // Pirate Cove
                // Curtains and Out of Order sign
                float cx = 400.0f;
                float cy = monitorY + 120.0f;
                // Purple star curtains
                DrawRectangle(cx, cy, 380, 320, Color{ 40, 20, 50, 255 });
                DrawRectangleLinesEx(Rectangle{ cx, cy, 380, 320 }, 6, Color{ 70, 45, 90, 255 });
                DrawText("OUT OF ORDER", cx + 110, cy + 280, 18, Color{ 180, 150, 80, 255 });

                if (foxState == FoxState::Hidden) {
                    // Closed curtains
                    DrawLineEx(Vector2{ cx + 190, cy }, Vector2{ cx + 190, cy + 270 }, 4, Color{ 20, 10, 25, 255 });
                } else if (foxState == FoxState::Watching) {
                    // Peeking out
                    DrawRectangle(cx + 160, cy + 60, 60, 180, Color{ 10, 10, 15, 255 });
                    DrawCircle(cx + 190, cy + 120, 22, Color{ 110, 45, 30, 255 });
                    DrawCircle(cx + 182, cy + 116, 4, Color{ 240, 230, 100, 255 });
                    DrawCircle(cx + 198, cy + 116, 4, Color{ 240, 230, 100, 255 });
                } else if (foxState == FoxState::Preparing) {
                    // Standing outside curtain
                    DrawRectangle(cx + 110, cy + 50, 160, 210, Color{ 15, 12, 20, 255 });
                    DrawRectangle(cx + 160, cy + 120, 60, 120, Color{ 100, 40, 25, 255 });
                    DrawCircle(cx + 190, cy + 90, 28, Color{ 120, 45, 30, 255 });
                    DrawTriangle(Vector2{ cx + 170, cy + 60 }, Vector2{ cx + 160, cy + 85 }, Vector2{ cx + 180, cy + 85 }, Color{ 120, 45, 30, 255 });
                    DrawTriangle(Vector2{ cx + 210, cy + 60 }, Vector2{ cx + 200, cy + 85 }, Vector2{ cx + 220, cy + 85 }, Color{ 120, 45, 30, 255 });
                    DrawCircle(cx + 182, cy + 86, 5, Color{ 255, 240, 120, 255 });
                    DrawCircle(cx + 198, cy + 86, 5, Color{ 255, 240, 120, 255 });
                } else {
                    // Curtains wide open, FOXY GONE!
                    DrawRectangle(cx + 70, cy + 40, 240, 230, Color{ 10, 10, 12, 255 });
                    DrawText("IT'S ME", cx + 140, cy + 120, 26, Color{ 180, 40, 40, 220 });
                }
                break;
            }

            case CameraID::CAM_1B: { // Dining Area
                // Tables with checkered cloths
                for (int t = 0; t < 3; ++t) {
                    float tx = 200.0f + t * 280.0f;
                    float ty = monitorY + 320.0f;
                    DrawRectangle(tx, ty, 220, 90, Color{ 45, 45, 52, 255 });
                    DrawRectangle(tx + 20, ty - 10, 180, 20, Color{ 180, 180, 190, 255 });
                }
                // Animatronics in Dining Area
                if (rabbitLoc == LocationID::DiningArea) {
                    DrawCircle(320, monitorY + 280, 30, Color{ 60, 50, 90, 255 });
                    DrawCircle(312, monitorY + 275, 4, Color{ 220, 220, 240, 255 });
                    DrawCircle(328, monitorY + 275, 4, Color{ 220, 220, 240, 255 });
                }
                if (chickenLoc == LocationID::DiningArea) {
                    DrawCircle(780, monitorY + 280, 30, Color{ 85, 75, 30, 255 });
                    DrawCircle(772, monitorY + 275, 4, Color{ 240, 240, 250, 255 });
                    DrawCircle(788, monitorY + 275, 4, Color{ 240, 240, 250, 255 });
                }
                if (bearLoc == LocationID::DiningArea) {
                    DrawCircle(560, monitorY + 240, 35, Color{ 75, 45, 25, 255 });
                    DrawCircle(550, monitorY + 235, 5, Color{ 220, 240, 255, 255 });
                    DrawCircle(570, monitorY + 235, 5, Color{ 220, 240, 255, 255 });
                }
                break;
            }

            case CameraID::CAM_2A: { // West Hall
                DrawRectangle(350, monitorY + 100, 480, 380, Color{ 20, 22, 28, 255 });
                // If Fox is running down West Hall!
                if (foxState == FoxState::Running) {
                    float fx = 560.0f;
                    float fy = monitorY + 220.0f;
                    DrawCircle(fx, fy, 40, Color{ 120, 45, 30, 255 });
                    DrawRectangle(fx - 25, fy + 35, 50, 90, Color{ 100, 40, 25, 255 });
                    DrawCircle(fx - 12, fy - 6, 6, Color{ 255, 240, 100, 255 });
                    DrawCircle(fx + 12, fy - 6, 6, Color{ 255, 240, 100, 255 });
                } else if (rabbitLoc == LocationID::WestHall) {
                    DrawCircle(580, monitorY + 260, 36, Color{ 60, 50, 90, 255 });
                    DrawCircle(570, monitorY + 252, 5, Color{ 220, 220, 240, 255 });
                    DrawCircle(590, monitorY + 252, 5, Color{ 220, 220, 240, 255 });
                }
                break;
            }

            case CameraID::CAM_2B: { // West Hall Corner
                DrawRectangle(250, monitorY + 80, 680, 420, Color{ 18, 20, 26, 255 });
                if (rabbitLoc == LocationID::WestHallCorner) {
                    // Rabbit right up close to camera!
                    float rx = 480.0f;
                    float ry = monitorY + 160.0f;
                    DrawCircle(rx, ry, 75, Color{ 70, 60, 105, 255 });
                    DrawRectangle(rx - 45, ry - 140, 25, 100, Color{ 70, 60, 105, 255 });
                    DrawRectangle(rx + 20, ry - 140, 25, 100, Color{ 70, 60, 105, 255 });
                    DrawCircle(rx - 25, ry - 15, 14, Color{ 230, 230, 245, 255 });
                    DrawCircle(rx + 25, ry - 15, 14, Color{ 230, 230, 245, 255 });
                    DrawCircle(rx - 25, ry - 15, 6, Color{ 200, 30, 80, 255 });
                    DrawCircle(rx + 25, ry - 15, 6, Color{ 200, 30, 80, 255 });
                }
                break;
            }

            case CameraID::CAM_4A: { // East Hall
                DrawRectangle(350, monitorY + 100, 480, 380, Color{ 22, 22, 26, 255 });
                if (chickenLoc == LocationID::EastHall) {
                    DrawCircle(580, monitorY + 260, 36, Color{ 85, 75, 30, 255 });
                    DrawCircle(570, monitorY + 252, 5, Color{ 240, 240, 250, 255 });
                    DrawCircle(590, monitorY + 252, 5, Color{ 240, 240, 250, 255 });
                } else if (bearLoc == LocationID::EastHall) {
                    DrawCircle(580, monitorY + 260, 38, Color{ 75, 45, 25, 255 });
                    DrawCircle(570, monitorY + 252, 5, Color{ 220, 240, 255, 255 });
                    DrawCircle(590, monitorY + 252, 5, Color{ 220, 240, 255, 255 });
                }
                break;
            }

            case CameraID::CAM_4B: { // East Hall Corner
                DrawRectangle(250, monitorY + 80, 680, 420, Color{ 20, 18, 22, 255 });
                if (chickenLoc == LocationID::EastHallCorner) {
                    float cx = 520.0f;
                    float cy = monitorY + 180.0f;
                    DrawCircle(cx, cy, 75, Color{ 95, 85, 35, 255 });
                    DrawTriangle(Vector2{ cx, cy + 45 }, Vector2{ cx - 35, cy + 5 }, Vector2{ cx + 35, cy + 5 }, Color{ 230, 130, 20, 255 });
                    DrawCircle(cx - 24, cy - 15, 14, Color{ 240, 240, 255, 255 });
                    DrawCircle(cx + 24, cy - 15, 14, Color{ 240, 240, 255, 255 });
                    DrawCircle(cx - 24, cy - 15, 6, Color{ 200, 30, 20, 255 });
                    DrawCircle(cx + 24, cy - 15, 6, Color{ 200, 30, 20, 255 });
                } else if (bearLoc == LocationID::EastHallCorner) {
                    float bx = 520.0f;
                    float by = monitorY + 180.0f;
                    DrawCircle(bx, by, 75, Color{ 80, 48, 26, 255 });
                    DrawCircle(bx - 24, by - 15, 14, Color{ 220, 240, 255, 255 });
                    DrawCircle(bx + 24, by - 15, 14, Color{ 220, 240, 255, 255 });
                    DrawCircle(bx - 24, by - 15, 5, WHITE);
                    DrawCircle(bx + 24, by - 15, 5, WHITE);
                }
                break;
            }

            case CameraID::CAM_5: { // Backstage
                DrawRectangle(200, monitorY + 80, 780, 420, Color{ 22, 20, 25, 255 });
                DrawText("SPARE PARTS", 240, monitorY + 110, 18, Color{ 100, 100, 110, 200 });
                // Shelves with endoskeleton
                DrawRectangle(650, monitorY + 180, 80, 160, Color{ 70, 72, 78, 255 });
                if (rabbitLoc == LocationID::Backstage) {
                    DrawCircle(460, monitorY + 240, 45, Color{ 60, 50, 90, 255 });
                    DrawCircle(448, monitorY + 232, 6, Color{ 220, 220, 240, 255 });
                    DrawCircle(472, monitorY + 232, 6, Color{ 220, 220, 240, 255 });
                }
                break;
            }

            case CameraID::CAM_3: { // Supply Closet
                DrawRectangle(300, monitorY + 90, 560, 400, Color{ 20, 24, 22, 255 });
                DrawText("SUPPLY CLOSET", 330, monitorY + 120, 18, Color{ 90, 110, 100, 200 });
                if (rabbitLoc == LocationID::SupplyCloset) {
                    DrawCircle(540, monitorY + 250, 45, Color{ 60, 50, 90, 255 });
                    DrawCircle(528, monitorY + 242, 6, Color{ 220, 220, 240, 255 });
                    DrawCircle(552, monitorY + 242, 6, Color{ 220, 220, 240, 255 });
                }
                break;
            }

            case CameraID::CAM_7: { // Restrooms
                DrawRectangle(250, monitorY + 90, 680, 400, Color{ 24, 22, 28, 255 });
                DrawText("RESTROOMS", 280, monitorY + 120, 18, Color{ 120, 100, 130, 200 });
                if (chickenLoc == LocationID::Restrooms) {
                    DrawCircle(540, monitorY + 250, 40, Color{ 85, 75, 30, 255 });
                    DrawCircle(530, monitorY + 242, 6, Color{ 240, 240, 250, 255 });
                    DrawCircle(550, monitorY + 242, 6, Color{ 240, 240, 250, 255 });
                } else if (bearLoc == LocationID::Restrooms) {
                    DrawCircle(540, monitorY + 250, 42, Color{ 75, 45, 25, 255 });
                    DrawCircle(530, monitorY + 242, 6, Color{ 220, 240, 255, 255 });
                    DrawCircle(550, monitorY + 242, 6, Color{ 220, 240, 255, 255 });
                }
                break;
            }
            default: break;
        }
    }

    // Static Noise Overlay
    int numSpecks = (m_switchStaticTimer > 0.0f) ? 1400 : 350;
    for (int s = 0; s < numSpecks; ++s) {
        int sx = GetRandomValue(0, SCREEN_WIDTH);
        int sy = GetRandomValue(static_cast<int>(monitorY), SCREEN_HEIGHT);
        int gray = GetRandomValue(100, 255);
        DrawPixel(sx, sy, Color{ static_cast<unsigned char>(gray), static_cast<unsigned char>(gray), static_cast<unsigned char>(gray), 140 });
    }

    // CRT Scanlines
    if (crtEnabled) {
        for (int y = static_cast<int>(monitorY); y < SCREEN_HEIGHT; y += 4) {
            DrawLine(0, y, SCREEN_WIDTH, y, Color{ 0, 0, 0, 50 });
        }
    }

    // Camera Name & REC blinking indicator
    DrawText(GetCameraName(m_currentCam), 60, monitorY + 50, 24, Color{ 220, 220, 220, 255 });

    bool recBlink = (std::fmod(m_staticTimer, 1.0f) < 0.5f);
    if (recBlink) {
        DrawCircle(SCREEN_WIDTH - 120, monitorY + 60, 10, RED);
        DrawText("REC", SCREEN_WIDTH - 100, monitorY + 50, 22, RED);
    }

    // Interactive Restaurant Map HUD in Bottom-Right
    float mapX = 900.0f;
    float mapY = monitorY + 340.0f;
    DrawRectangle(mapX - 20, mapY - 20, 360, 360, Color{ 10, 12, 16, 220 });
    DrawRectangleLinesEx(Rectangle{ mapX - 20, mapY - 20, 360, 360 }, 2, Color{ 50, 60, 75, 255 });

    // Stylized restaurant floorplan outlines
    DrawRectangleLines(mapX + 70, mapY + 20, 140, 120, Color{ 60, 75, 95, 255 }); // Dining/Stage
    DrawRectangleLines(mapX + 110, mapY + 160, 60, 130, Color{ 60, 75, 95, 255 }); // Office Hallway
    DrawText("YOU", mapX + 125, mapY + 300, 14, GREEN);

    // Map Buttons
    Vector2 mousePos = GetMousePosition();
    for (const auto& btn : m_mapButtons) {
        Rectangle drawRect = btn.rect;
        drawRect.y += (monitorY);

        bool isCurrent = (btn.id == m_currentCam);
        bool hovering = CheckCollisionPointRec(mousePos, drawRect);

        Color btnBg = isCurrent ? Color{ 50, 160, 70, 255 } : (hovering ? Color{ 80, 90, 110, 255 } : Color{ 35, 40, 50, 255 });
        DrawRectangleRec(drawRect, btnBg);
        DrawRectangleLinesEx(drawRect, isCurrent ? 2 : 1, isCurrent ? GREEN : GRAY);

        Color textCol = isCurrent ? WHITE : (hovering ? YELLOW : LIGHTGRAY);
        DrawText(btn.label, drawRect.x + 12, drawRect.y + 7, 16, textCol);
    }

    // Bottom flip bar (to close monitor)
    Rectangle closeBar = m_monitorFlipBar;
    closeBar.y = SCREEN_HEIGHT - 45.0f;
    bool hoveringClose = CheckCollisionPointRec(mousePos, closeBar);
    DrawRectangleRec(closeBar, hoveringClose ? Color{ 200, 70, 70, 160 } : Color{ 40, 40, 50, 140 });
    DrawRectangleLinesEx(closeBar, 2, hoveringClose ? RED : GRAY);
    DrawText("[ SPACE / CLICK ] TO CLOSE CAMERA MONITOR", closeBar.x + 85, closeBar.y + 12, 18, WHITE);
}

} // namespace FNAF
