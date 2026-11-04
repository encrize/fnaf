#include "Office.h"
#include "Audio.h"
#include "AssetManager.h"
#include <raylib.h>
#include <cmath>
#include <algorithm>

namespace FNAF {

Office::Office() {
    Reset();
}

void Office::LoadAssets(AssetManager& assets) {
    m_officeTex = assets.GetTexture("assets/images/office_panorama.png");
    m_doorTex = assets.GetTexture("assets/images/door.png");
    m_panelTex = assets.GetTexture("assets/images/button_panel.png");
    m_fanTex = assets.GetTexture("assets/images/fan_blade.png");
    m_rabbitDoorTex = assets.GetTexture("assets/images/rabbit_door.png");
    m_chickenDoorTex = assets.GetTexture("assets/images/chicken_door.png");
    m_goldenBearTex = assets.GetTexture("assets/images/golden_bear.png");
    m_vignetteTex = assets.GetTexture("assets/images/vignette.png");
}

void Office::Reset() {
    m_cameraX = m_maxPan * 0.5f; // Start centered
    m_targetCameraX = m_cameraX;
    m_leftDoorClosed = false;
    m_rightDoorClosed = false;
    m_leftDoorProgress = 0.0f;
    m_rightDoorProgress = 0.0f;
    m_leftLightActive = false;
    m_rightLightActive = false;
    m_fanBladeAngle = 0.0f;

    // Buttons relative to office world coordinates (Office width = 1600)
    m_leftDoorButton = Rectangle{ 70.0f, 260.0f, 40.0f, 45.0f };
    m_leftLightButton = Rectangle{ 70.0f, 320.0f, 40.0f, 45.0f };

    m_rightDoorButton = Rectangle{ 1490.0f, 260.0f, 40.0f, 45.0f };
    m_rightLightButton = Rectangle{ 1490.0f, 320.0f, 40.0f, 45.0f };
}

void Office::SetLeftDoor(bool closed, Audio& audio) {
    if (m_leftDoorClosed != closed) {
        m_leftDoorClosed = closed;
        audio.PlayDoorSound(closed);
    }
}

void Office::SetRightDoor(bool closed, Audio& audio) {
    if (m_rightDoorClosed != closed) {
        m_rightDoorClosed = closed;
        audio.PlayDoorSound(closed);
    }
}

void Office::SetLeftLight(bool active, Audio& audio) {
    if (m_leftLightActive != active) {
        m_leftLightActive = active;
        audio.SetLightBuzz(m_leftLightActive || m_rightLightActive);
    }
}

void Office::SetRightLight(bool active, Audio& audio) {
    if (m_rightLightActive != active) {
        m_rightLightActive = active;
        audio.SetLightBuzz(m_leftLightActive || m_rightLightActive);
    }
}

void Office::HandleInput(bool monitorUp, bool powerDepleted, Audio& audio) {
    if (monitorUp || powerDepleted) {
        SetLeftLight(false, audio);
        SetRightLight(false, audio);
        return;
    }

    Vector2 mousePos = GetMousePosition();
    float edgeZone = 220.0f;
    if (mousePos.x < edgeZone || IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) {
        m_targetCameraX = 0.0f;
    } else if (mousePos.x > SCREEN_WIDTH - edgeZone || IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) {
        m_targetCameraX = m_maxPan;
    } else {
        float factor = (mousePos.x - edgeZone) / (SCREEN_WIDTH - 2.0f * edgeZone);
        m_targetCameraX = std::clamp(factor * m_maxPan, 0.0f, m_maxPan);
    }

    Vector2 worldMouse = { mousePos.x + m_cameraX, mousePos.y };

    if (IsKeyPressed(KEY_Q) || (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(worldMouse, m_leftDoorButton))) {
        SetLeftDoor(!m_leftDoorClosed, audio);
    }

    bool leftLightDesired = (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(worldMouse, m_leftLightButton)) || IsKeyDown(KEY_Z);
    SetLeftLight(leftLightDesired, audio);

    if (IsKeyPressed(KEY_E) || (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(worldMouse, m_rightDoorButton))) {
        SetRightDoor(!m_rightDoorClosed, audio);
    }

    bool rightLightDesired = (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(worldMouse, m_rightLightButton)) || IsKeyDown(KEY_C);
    SetRightLight(rightLightDesired, audio);
}

void Office::Update(float dt, bool powerDepleted, Audio& /*audio*/) {
    if (powerDepleted) {
        m_leftDoorClosed = false;
        m_rightDoorClosed = false;
        m_leftLightActive = false;
        m_rightLightActive = false;
    }

    m_cameraX += (m_targetCameraX - m_cameraX) * std::min(1.0f, dt * 9.0f);

    float doorSpeed = 4.0f;
    float targetLeft = m_leftDoorClosed ? 1.0f : 0.0f;
    float targetRight = m_rightDoorClosed ? 1.0f : 0.0f;

    m_leftDoorProgress += (targetLeft - m_leftDoorProgress) * std::min(1.0f, dt * doorSpeed);
    m_rightDoorProgress += (targetRight - m_rightDoorProgress) * std::min(1.0f, dt * doorSpeed);

    if (!powerDepleted) {
        m_fanBladeAngle += dt * 1200.0f;
        if (m_fanBladeAngle >= 360.0f) m_fanBladeAngle -= 360.0f;
    }
}

void Office::Render(bool rabbitAtDoor, bool chickenAtDoor, bool bearPowerOutFlicker, bool rareVisitorActive, bool powerDepleted) {
    float ox = -m_cameraX;

    // 1. Office Panorama Texture Background
    if (m_officeTex.id != 0) {
        Color tint = powerDepleted ? Color{ 25, 25, 35, 255 } : WHITE;
        DrawTexture(m_officeTex, ox, 0, tint);
    } else {
        // Fallback procedural wall
        Color wallColor = powerDepleted ? Color{ 8, 8, 12, 255 } : Color{ 32, 34, 40, 255 };
        DrawRectangle(ox, 0, 1600, SCREEN_HEIGHT, wallColor);
    }

    // 2. Left Doorway & Hallway Lighting
    float leftDoorX = ox + 140.0f;
    float leftDoorY = 120.0f;
    float doorW = 160.0f;
    float doorH = 460.0f;

    if (m_leftLightActive && !powerDepleted) {
        // Illuminated hallway
        DrawRectangle(leftDoorX, leftDoorY, doorW, doorH, Color{ 140, 140, 160, 255 });
        DrawRectangleGradientH(leftDoorX - 60, leftDoorY, 60, doorH, Color{ 0, 0, 0, 0 }, Color{ 160, 160, 180, 80 });
        DrawRectangleGradientH(leftDoorX, leftDoorY, doorW, doorH, Color{ 200, 200, 220, 120 }, Color{ 100, 100, 120, 80 });
    }

    // Rabbit at Left Door
    if (rabbitAtDoor && m_leftLightActive && !powerDepleted) {
        if (m_rabbitDoorTex.id != 0) {
            DrawTexture(m_rabbitDoorTex, leftDoorX + 20, leftDoorY + 110, WHITE);
        } else {
            // Fallback silhouette
            float rx = leftDoorX + 35.0f;
            float ry = leftDoorY + 90.0f;
            DrawRectangle(rx + 10, ry + 120, 70, 180, Color{ 45, 35, 65, 255 });
            DrawCircle(rx + 45, ry + 90, 36, Color{ 55, 45, 80, 255 });
            DrawCircle(rx + 33, ry + 85, 8, Color{ 230, 230, 240, 255 });
            DrawCircle(rx + 57, ry + 85, 8, Color{ 230, 230, 240, 255 });
            DrawCircle(rx + 33, ry + 85, 4, Color{ 200, 30, 80, 255 });
            DrawCircle(rx + 57, ry + 85, 4, Color{ 200, 30, 80, 255 });
        }
    }

    // Left Blast Door (sliding down)
    float leftDoorVisualH = doorH * m_leftDoorProgress;
    if (leftDoorVisualH > 2.0f) {
        if (m_doorTex.id != 0) {
            Rectangle src{ 0, 0, (float)m_doorTex.width, (float)m_doorTex.height * m_leftDoorProgress };
            Rectangle dst{ leftDoorX, leftDoorY, doorW, leftDoorVisualH };
            DrawTexturePro(m_doorTex, src, dst, Vector2{ 0, 0 }, 0.0f, WHITE);
        } else {
            DrawRectangle(leftDoorX, leftDoorY, doorW, leftDoorVisualH, Color{ 50, 52, 60, 255 });
        }
    }

    // 3. Right Doorway & Hallway Lighting
    float rightDoorX = ox + 1300.0f;
    float rightDoorY = 120.0f;

    if (m_rightLightActive && !powerDepleted) {
        DrawRectangle(rightDoorX, rightDoorY, doorW, doorH, Color{ 150, 140, 120, 255 });
        DrawRectangleGradientH(rightDoorX + doorW, rightDoorY, 60, doorH, Color{ 180, 160, 140, 80 }, Color{ 0, 0, 0, 0 });
        DrawRectangleGradientH(rightDoorX, rightDoorY, doorW, doorH, Color{ 120, 110, 90, 80 }, Color{ 220, 200, 160, 120 });
    }

    // Chicken at Right Door
    if (chickenAtDoor && m_rightLightActive && !powerDepleted) {
        if (m_chickenDoorTex.id != 0) {
            DrawTexture(m_chickenDoorTex, rightDoorX + 20, rightDoorY + 110, WHITE);
        } else {
            float cx = rightDoorX + 35.0f;
            float cy = rightDoorY + 95.0f;
            DrawRectangle(cx + 10, cy + 115, 70, 185, Color{ 70, 60, 30, 255 });
            DrawCircle(cx + 45, cy + 85, 38, Color{ 85, 75, 35, 255 });
            DrawCircle(cx + 33, cy + 78, 8, Color{ 240, 240, 245, 255 });
            DrawCircle(cx + 57, cy + 78, 8, Color{ 240, 240, 245, 255 });
            DrawCircle(cx + 33, cy + 78, 4, Color{ 180, 30, 20, 255 });
            DrawCircle(cx + 57, cy + 78, 4, Color{ 180, 30, 20, 255 });
        }
    }

    // Right Blast Door (sliding down)
    float rightDoorVisualH = doorH * m_rightDoorProgress;
    if (rightDoorVisualH > 2.0f) {
        if (m_doorTex.id != 0) {
            Rectangle src{ 0, 0, (float)m_doorTex.width, (float)m_doorTex.height * m_rightDoorProgress };
            Rectangle dst{ rightDoorX, rightDoorY, doorW, rightDoorVisualH };
            DrawTexturePro(m_doorTex, src, dst, Vector2{ 0, 0 }, 0.0f, WHITE);
        } else {
            DrawRectangle(rightDoorX, rightDoorY, doorW, rightDoorVisualH, Color{ 50, 52, 60, 255 });
        }
    }

    // 4. Power-Out Bear Glowing Face in Left Doorway
    if (bearPowerOutFlicker && powerDepleted) {
        float bx = leftDoorX + 45.0f;
        float by = leftDoorY + 120.0f;
        DrawCircle(bx + 35, by + 60, 42, Color{ 25, 20, 15, 255 });
        DrawCircle(bx + 10, by + 30, 14, Color{ 25, 20, 15, 255 });
        DrawCircle(bx + 60, by + 30, 14, Color{ 25, 20, 15, 255 });
        DrawCircle(bx + 24, by + 55, 6, Color{ 180, 220, 255, 255 });
        DrawCircle(bx + 46, by + 55, 6, Color{ 180, 220, 255, 255 });
        DrawCircle(bx + 24, by + 55, 3, Color{ 255, 255, 255, 255 });
        DrawCircle(bx + 46, by + 55, 3, Color{ 255, 255, 255, 255 });
    }

    // 5. Door & Light Control Panels with Textures
    // Left Control Panel
    if (m_panelTex.id != 0) {
        DrawTexture(m_panelTex, ox + m_leftDoorButton.x - 10, m_leftDoorButton.y - 15, WHITE);
    } else {
        DrawRectangle(ox + m_leftDoorButton.x - 10, m_leftDoorButton.y - 15, 60, 125, Color{ 20, 20, 24, 255 });
    }

    // Left Door Button
    Color lDoorCol = m_leftDoorClosed ? Color{ 210, 40, 40, 255 } : Color{ 120, 120, 130, 255 };
    DrawRectangle(ox + m_leftDoorButton.x, m_leftDoorButton.y, m_leftDoorButton.width, m_leftDoorButton.height, lDoorCol);
    DrawText("DOOR", ox + m_leftDoorButton.x + 3, m_leftDoorButton.y + 10, 10, WHITE);
    DrawText("[Q]", ox + m_leftDoorButton.x + 11, m_leftDoorButton.y + 26, 12, YELLOW);

    // Left Light Button
    Color lLightCol = m_leftLightActive ? Color{ 80, 200, 240, 255 } : Color{ 60, 60, 70, 255 };
    DrawRectangle(ox + m_leftLightButton.x, m_leftLightButton.y, m_leftLightButton.width, m_leftLightButton.height, lLightCol);
    DrawText("LIGHT", ox + m_leftLightButton.x + 3, m_leftLightButton.y + 10, 10, WHITE);
    DrawText("[Z]", ox + m_leftLightButton.x + 11, m_leftLightButton.y + 26, 12, YELLOW);

    // Right Control Panel
    if (m_panelTex.id != 0) {
        DrawTexture(m_panelTex, ox + m_rightDoorButton.x - 10, m_rightDoorButton.y - 15, WHITE);
    } else {
        DrawRectangle(ox + m_rightDoorButton.x - 10, m_rightDoorButton.y - 15, 60, 125, Color{ 20, 20, 24, 255 });
    }

    // Right Door Button
    Color rDoorCol = m_rightDoorClosed ? Color{ 210, 40, 40, 255 } : Color{ 120, 120, 130, 255 };
    DrawRectangle(ox + m_rightDoorButton.x, m_rightDoorButton.y, m_rightDoorButton.width, m_rightDoorButton.height, rDoorCol);
    DrawText("DOOR", ox + m_rightDoorButton.x + 3, m_rightDoorButton.y + 10, 10, WHITE);
    DrawText("[E]", ox + m_rightDoorButton.x + 11, m_rightDoorButton.y + 26, 12, YELLOW);

    // Right Light Button
    Color rLightCol = m_rightLightActive ? Color{ 80, 200, 240, 255 } : Color{ 60, 60, 70, 255 };
    DrawRectangle(ox + m_rightLightButton.x, m_rightLightButton.y, m_rightLightButton.width, m_rightLightButton.height, rLightCol);
    DrawText("LIGHT", ox + m_rightLightButton.x + 3, m_rightLightButton.y + 10, 10, WHITE);
    DrawText("[C]", ox + m_rightLightButton.x + 11, m_rightLightButton.y + 26, 12, YELLOW);

    // 6. Rare Visitor (Golden Bear)
    if (rareVisitorActive && !powerDepleted) {
        float gX = ox + 680.0f;
        float gY = 480.0f;
        if (m_goldenBearTex.id != 0) {
            DrawTexture(m_goldenBearTex, gX, gY, WHITE);
        } else {
            DrawRectangle(gX + 20, gY + 40, 80, 90, Color{ 140, 120, 20, 255 });
            DrawCircle(gX + 60, gY + 30, 36, Color{ 160, 135, 25, 255 });
        }
    }

    // 7. Desk Fan with Texture & Continuous Rotation
    float fanBaseX = ox + 760.0f;
    float fanBaseY = 490.0f;
    float fanCenterX = fanBaseX + 40.0f;
    float fanCenterY = fanBaseY - 55.0f;

    // Fan stand
    DrawRectangle(fanBaseX + 35, fanBaseY - 45, 10, 45, Color{ 30, 30, 35, 255 });
    DrawCircle(fanBaseX + 40, fanBaseY, 22, Color{ 25, 25, 30, 255 });

    // Fan head housing
    DrawCircle(fanCenterX, fanCenterY, 38, Color{ 20, 20, 25, 180 });
    DrawCircleLines(fanCenterX, fanCenterY, 38, Color{ 70, 70, 80, 255 });

    if (m_fanTex.id != 0) {
        Rectangle src{ 0, 0, (float)m_fanTex.width, (float)m_fanTex.height };
        Rectangle dst{ fanCenterX, fanCenterY, 70, 70 };
        Vector2 origin{ 35, 35 };
        DrawTexturePro(m_fanTex, src, dst, origin, m_fanBladeAngle, WHITE);
    } else {
        // Fallback procedural fan blades
        for (int b = 0; b < 3; ++b) {
            float angleRad = (m_fanBladeAngle + b * 120.0f) * (PI / 180.0f);
            float bx1 = fanCenterX;
            float by1 = fanCenterY;
            float bx2 = fanCenterX + std::cos(angleRad) * 32.0f;
            float by2 = fanCenterY + std::sin(angleRad) * 32.0f;
            DrawLineEx(Vector2{ bx1, by1 }, Vector2{ bx2, by2 }, 6, Color{ 140, 145, 155, 255 });
        }
    }
    DrawCircle(fanCenterX, fanCenterY, 8, Color{ 40, 42, 48, 255 });

    // 8. Vignette Shadow Overlay
    if (m_vignetteTex.id != 0) {
        DrawTexture(m_vignetteTex, 0, 0, WHITE);
    } else {
        DrawRectangleGradientV(0, 0, SCREEN_WIDTH, 140, Color{ 0, 0, 0, 180 }, Color{ 0, 0, 0, 0 });
        DrawRectangleGradientV(0, SCREEN_HEIGHT - 120, SCREEN_WIDTH, 120, Color{ 0, 0, 0, 0 }, Color{ 0, 0, 0, 160 });
    }
}

} // namespace FNAF
