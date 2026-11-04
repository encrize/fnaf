#include "Office.h"
#include "Audio.h"
#include <raylib.h>
#include <cmath>
#include <algorithm>

namespace FNAF {

Office::Office() {
    Reset();
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
    // Left wall controls
    m_leftDoorButton = Rectangle{ 70.0f, 260.0f, 40.0f, 45.0f };
    m_leftLightButton = Rectangle{ 70.0f, 320.0f, 40.0f, 45.0f };

    // Right wall controls
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
    // Office panning with mouse edge zones or A/D keyboard controls
    float edgeZone = 220.0f;
    if (mousePos.x < edgeZone || IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) {
        m_targetCameraX = 0.0f;
    } else if (mousePos.x > SCREEN_WIDTH - edgeZone || IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) {
        m_targetCameraX = m_maxPan;
    } else {
        // Intermediate mouse panning
        float factor = (mousePos.x - edgeZone) / (SCREEN_WIDTH - 2.0f * edgeZone);
        m_targetCameraX = std::clamp(factor * m_maxPan, 0.0f, m_maxPan);
    }

    // World mouse coordinates taking camera panning into account
    Vector2 worldMouse = { mousePos.x + m_cameraX, mousePos.y };

    // Left door button click (Q or mouse click)
    if (IsKeyPressed(KEY_Q) || (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(worldMouse, m_leftDoorButton))) {
        SetLeftDoor(!m_leftDoorClosed, audio);
    }

    // Left light button hold (A or mouse hold)
    bool leftLightDesired = (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(worldMouse, m_leftLightButton)) || IsKeyDown(KEY_Z);
    SetLeftLight(leftLightDesired, audio);

    // Right door button click (E or mouse click)
    if (IsKeyPressed(KEY_E) || (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(worldMouse, m_rightDoorButton))) {
        SetRightDoor(!m_rightDoorClosed, audio);
    }

    // Right light button hold (D or mouse hold)
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

    // Smooth horizontal panning interpolation
    m_cameraX += (m_targetCameraX - m_cameraX) * std::min(1.0f, dt * 9.0f);

    // Door animation interpolation (opens/closes in ~0.25 seconds)
    float doorSpeed = 4.0f;
    float targetLeft = m_leftDoorClosed ? 1.0f : 0.0f;
    float targetRight = m_rightDoorClosed ? 1.0f : 0.0f;

    m_leftDoorProgress += (targetLeft - m_leftDoorProgress) * std::min(1.0f, dt * doorSpeed);
    m_rightDoorProgress += (targetRight - m_rightDoorProgress) * std::min(1.0f, dt * doorSpeed);

    // Desk fan rotation animation
    if (!powerDepleted) {
        m_fanBladeAngle += dt * 1200.0f;
        if (m_fanBladeAngle >= 360.0f) m_fanBladeAngle -= 360.0f;
    }
}

void Office::Render(bool rabbitAtDoor, bool chickenAtDoor, bool bearPowerOutFlicker, bool rareVisitorActive, bool powerDepleted) {
    // Offset everything by m_cameraX
    float ox = -m_cameraX;

    // 1. Office Walls & Background
    Color wallColor = powerDepleted ? Color{ 8, 8, 12, 255 } : Color{ 32, 34, 40, 255 };
    DrawRectangle(ox, 0, 1600, SCREEN_HEIGHT, wallColor);

    // Checkerboard floor trim
    Color floorColor1 = powerDepleted ? Color{ 14, 14, 18, 255 } : Color{ 48, 48, 56, 255 };
    Color floorColor2 = powerDepleted ? Color{ 6, 6, 8, 255 } : Color{ 22, 22, 26, 255 };
    DrawRectangle(ox, 560, 1600, 160, floorColor2);
    for (int i = 0; i < 40; ++i) {
        for (int j = 0; j < 4; ++j) {
            if ((i + j) % 2 == 0) {
                DrawRectangle(ox + i * 40, 560 + j * 40, 40, 40, floorColor1);
            }
        }
    }

    // 2. Left Doorway & Hallway
    float leftDoorX = ox + 140.0f;
    float leftDoorY = 120.0f;
    float doorW = 160.0f;
    float doorH = 460.0f;

    // Hallway corridor opening (dark void unless light active)
    Color leftHallColor = Color{ 4, 4, 6, 255 };
    if (m_leftLightActive && !powerDepleted) {
        leftHallColor = Color{ 140, 140, 160, 255 }; // Illuminated hallway
    }
    DrawRectangle(leftDoorX, leftDoorY, doorW, doorH, leftHallColor);

    // Left Hallway Light Beam / Wall Illumination
    if (m_leftLightActive && !powerDepleted) {
        DrawRectangleGradientH(leftDoorX - 60, leftDoorY, 60, doorH, Color{ 0, 0, 0, 0 }, Color{ 160, 160, 180, 80 });
        DrawRectangleGradientH(leftDoorX, leftDoorY, doorW, doorH, Color{ 200, 200, 220, 120 }, Color{ 100, 100, 120, 80 });
    }

    // Rabbit standing at the left door window/doorway!
    if (rabbitAtDoor && m_leftLightActive && !powerDepleted) {
        // Tall silhouette with long ears and glowing magenta eyes
        float rx = leftDoorX + 35.0f;
        float ry = leftDoorY + 90.0f;

        // Silhouette body & head
        DrawRectangle(rx + 10, ry + 120, 70, 180, Color{ 45, 35, 65, 255 });
        DrawCircle(rx + 45, ry + 90, 36, Color{ 55, 45, 80, 255 });

        // Long rabbit ears
        DrawRectangle(rx + 22, ry - 10, 14, 80, Color{ 55, 45, 80, 255 });
        DrawRectangle(rx + 54, ry - 10, 14, 80, Color{ 55, 45, 80, 255 });
        DrawRectangle(rx + 25, ry + 10, 8, 55, Color{ 85, 70, 110, 255 });
        DrawRectangle(rx + 57, ry + 10, 8, 55, Color{ 85, 70, 110, 255 });

        // Glowing menacing eyes staring right at you
        DrawCircle(rx + 33, ry + 85, 8, Color{ 230, 230, 240, 255 });
        DrawCircle(rx + 57, ry + 85, 8, Color{ 230, 230, 240, 255 });
        DrawCircle(rx + 33, ry + 85, 4, Color{ 200, 30, 80, 255 });
        DrawCircle(rx + 57, ry + 85, 4, Color{ 200, 30, 80, 255 });
        // Snout
        DrawCircle(rx + 45, ry + 102, 12, Color{ 40, 30, 60, 255 });
    }

    // Left Blast Door (sliding down)
    float leftDoorVisualH = doorH * m_leftDoorProgress;
    if (leftDoorVisualH > 2.0f) {
        DrawRectangle(leftDoorX, leftDoorY, doorW, leftDoorVisualH, Color{ 50, 52, 60, 255 });
        // Hazard stripes on door bottom
        for (int s = 0; s < 5; ++s) {
            DrawRectangle(leftDoorX + s * 32, leftDoorY + leftDoorVisualH - 24, 16, 24, Color{ 190, 160, 20, 255 });
            DrawRectangle(leftDoorX + s * 32 + 16, leftDoorY + leftDoorVisualH - 24, 16, 24, Color{ 20, 20, 20, 255 });
        }
        // Rivets and steel bars
        DrawRectangle(leftDoorX + 10, leftDoorY, 6, leftDoorVisualH, Color{ 35, 37, 42, 255 });
        DrawRectangle(leftDoorX + doorW - 16, leftDoorY, 6, leftDoorVisualH, Color{ 35, 37, 42, 255 });
    }
    // Doorframe outline
    DrawRectangleLinesEx(Rectangle{ leftDoorX, leftDoorY, doorW, doorH }, 6, Color{ 25, 25, 30, 255 });

    // 3. Right Doorway & Hallway
    float rightDoorX = ox + 1300.0f;
    float rightDoorY = 120.0f;

    Color rightHallColor = Color{ 4, 4, 6, 255 };
    if (m_rightLightActive && !powerDepleted) {
        rightHallColor = Color{ 150, 140, 120, 255 }; // Illuminated hallway
    }
    DrawRectangle(rightDoorX, rightDoorY, doorW, doorH, rightHallColor);

    // Right Hallway Light Beam
    if (m_rightLightActive && !powerDepleted) {
        DrawRectangleGradientH(rightDoorX + doorW, rightDoorY, 60, doorH, Color{ 180, 160, 140, 80 }, Color{ 0, 0, 0, 0 });
        DrawRectangleGradientH(rightDoorX, rightDoorY, doorW, doorH, Color{ 120, 110, 90, 80 }, Color{ 220, 200, 160, 120 });
    }

    // Chicken standing at the right door window!
    if (chickenAtDoor && m_rightLightActive && !powerDepleted) {
        float cx = rightDoorX + 35.0f;
        float cy = rightDoorY + 95.0f;

        // Silhouette body & rounded head
        DrawRectangle(cx + 10, cy + 115, 70, 185, Color{ 70, 60, 30, 255 });
        DrawCircle(cx + 45, cy + 85, 38, Color{ 85, 75, 35, 255 });
        // Head tuft feathers
        DrawTriangle(Vector2{ cx + 45, cy + 30 }, Vector2{ cx + 38, cy + 55 }, Vector2{ cx + 52, cy + 55 }, Color{ 90, 80, 35, 255 });

        // Glowing eyes & beak
        DrawCircle(cx + 33, cy + 78, 8, Color{ 240, 240, 245, 255 });
        DrawCircle(cx + 57, cy + 78, 8, Color{ 240, 240, 245, 255 });
        DrawCircle(cx + 33, cy + 78, 4, Color{ 180, 30, 20, 255 });
        DrawCircle(cx + 57, cy + 78, 4, Color{ 180, 30, 20, 255 });
        // Wide orange triangular beak
        DrawTriangle(Vector2{ cx + 45, cy + 105 }, Vector2{ cx + 30, cy + 88 }, Vector2{ cx + 60, cy + 88 }, Color{ 230, 130, 20, 255 });
    }

    // Right Blast Door (sliding down)
    float rightDoorVisualH = doorH * m_rightDoorProgress;
    if (rightDoorVisualH > 2.0f) {
        DrawRectangle(rightDoorX, rightDoorY, doorW, rightDoorVisualH, Color{ 50, 52, 60, 255 });
        for (int s = 0; s < 5; ++s) {
            DrawRectangle(rightDoorX + s * 32, rightDoorY + rightDoorVisualH - 24, 16, 24, Color{ 190, 160, 20, 255 });
            DrawRectangle(rightDoorX + s * 32 + 16, rightDoorY + rightDoorVisualH - 24, 16, 24, Color{ 20, 20, 20, 255 });
        }
        DrawRectangle(rightDoorX + 10, rightDoorY, 6, rightDoorVisualH, Color{ 35, 37, 42, 255 });
        DrawRectangle(rightDoorX + doorW - 16, rightDoorY, 6, rightDoorVisualH, Color{ 35, 37, 42, 255 });
    }
    DrawRectangleLinesEx(Rectangle{ rightDoorX, rightDoorY, doorW, doorH }, 6, Color{ 25, 25, 30, 255 });

    // 4. Power-Out Bear Glowing Face in Left Doorway
    if (bearPowerOutFlicker && powerDepleted) {
        float bx = leftDoorX + 45.0f;
        float by = leftDoorY + 120.0f;
        // Faint glowing silhouette
        DrawCircle(bx + 35, by + 60, 42, Color{ 25, 20, 15, 255 });
        // Bear ears
        DrawCircle(bx + 10, by + 30, 14, Color{ 25, 20, 15, 255 });
        DrawCircle(bx + 60, by + 30, 14, Color{ 25, 20, 15, 255 });
        // Piercing glowing blue-white eyes flickering in the dark
        DrawCircle(bx + 24, by + 55, 6, Color{ 180, 220, 255, 255 });
        DrawCircle(bx + 46, by + 55, 6, Color{ 180, 220, 255, 255 });
        DrawCircle(bx + 24, by + 55, 3, Color{ 255, 255, 255, 255 });
        DrawCircle(bx + 46, by + 55, 3, Color{ 255, 255, 255, 255 });
    }

    // 5. Door & Light Control Panels
    // Left Control Panel
    DrawRectangle(ox + m_leftDoorButton.x - 10, m_leftDoorButton.y - 15, 60, 125, Color{ 20, 20, 24, 255 });
    DrawRectangleLinesEx(Rectangle{ ox + m_leftDoorButton.x - 10, m_leftDoorButton.y - 15, 60, 125 }, 2, Color{ 40, 40, 48, 255 });

    // Left Door Button (DOOR)
    Color lDoorCol = m_leftDoorClosed ? Color{ 210, 40, 40, 255 } : Color{ 120, 120, 130, 255 };
    DrawRectangle(ox + m_leftDoorButton.x, m_leftDoorButton.y, m_leftDoorButton.width, m_leftDoorButton.height, lDoorCol);
    DrawText("DOOR", ox + m_leftDoorButton.x + 3, m_leftDoorButton.y + 10, 10, WHITE);
    DrawText("[Q]", ox + m_leftDoorButton.x + 11, m_leftDoorButton.y + 26, 12, YELLOW);

    // Left Light Button (LIGHT)
    Color lLightCol = m_leftLightActive ? Color{ 80, 200, 240, 255 } : Color{ 60, 60, 70, 255 };
    DrawRectangle(ox + m_leftLightButton.x, m_leftLightButton.y, m_leftLightButton.width, m_leftLightButton.height, lLightCol);
    DrawText("LIGHT", ox + m_leftLightButton.x + 3, m_leftLightButton.y + 10, 10, WHITE);
    DrawText("[Z]", ox + m_leftLightButton.x + 11, m_leftLightButton.y + 26, 12, YELLOW);

    // Right Control Panel
    DrawRectangle(ox + m_rightDoorButton.x - 10, m_rightDoorButton.y - 15, 60, 125, Color{ 20, 20, 24, 255 });
    DrawRectangleLinesEx(Rectangle{ ox + m_rightDoorButton.x - 10, m_rightDoorButton.y - 15, 60, 125 }, 2, Color{ 40, 40, 48, 255 });

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

    // 6. Wall Posters & Decorations
    // Central "CELEBRATE!" Poster
    float posterX = ox + 720.0f;
    float posterY = 120.0f;
    DrawRectangle(posterX, posterY, 160, 110, Color{ 20, 20, 25, 255 });
    DrawRectangleLinesEx(Rectangle{ posterX, posterY, 160, 110 }, 4, Color{ 160, 140, 50, 255 });
    DrawText("CELEBRATE!", posterX + 22, posterY + 12, 18, Color{ 230, 220, 100, 255 });
    // Three small character silhouettes on poster
    DrawCircle(posterX + 45, posterY + 68, 14, Color{ 80, 60, 110, 255 }); // Rabbit
    DrawCircle(posterX + 80, posterY + 62, 17, Color{ 100, 70, 40, 255 }); // Bear
    DrawCircle(posterX + 115, posterY + 68, 14, Color{ 110, 100, 40, 255 }); // Chicken

    // Party streamer banner across the wall
    for (int p = 0; p < 12; ++p) {
        float px = ox + 420.0f + p * 64.0f;
        Color triCol = (p % 3 == 0) ? RED : ((p % 3 == 1) ? YELLOW : SKYBLUE);
        DrawTriangle(Vector2{ px, 70.0f }, Vector2{ px + 32.0f, 100.0f }, Vector2{ px + 64.0f, 70.0f }, ColorAlpha(triCol, 0.7f));
    }

    // 7. Rare Visitor (Golden Bear hallucination) in office floor
    if (rareVisitorActive && !powerDepleted) {
        float gX = ox + 680.0f;
        float gY = 480.0f;
        // Slumped golden bear sitting lifelessly
        DrawRectangle(gX + 20, gY + 40, 80, 90, Color{ 140, 120, 20, 255 });
        DrawCircle(gX + 60, gY + 30, 36, Color{ 160, 135, 25, 255 });
        // Empty black eye sockets with tiny glowing white pinpoints
        DrawCircle(gX + 48, gY + 25, 8, BLACK);
        DrawCircle(gX + 72, gY + 25, 8, BLACK);
        DrawCircle(gX + 48, gY + 25, 2, WHITE);
        DrawCircle(gX + 72, gY + 25, 2, WHITE);
        // Slumped limbs
        DrawRectangle(gX - 5, gY + 70, 30, 60, Color{ 130, 110, 20, 255 });
        DrawRectangle(gX + 95, gY + 70, 30, 60, Color{ 130, 110, 20, 255 });
    }

    // 8. Main Desk & Equipment
    float deskX = ox + 400.0f;
    float deskY = 500.0f;
    float deskW = 800.0f;
    float deskH = 220.0f;

    Color deskColor = powerDepleted ? Color{ 18, 18, 22, 255 } : Color{ 40, 42, 48, 255 };
    DrawRectangle(deskX, deskY, deskW, deskH, deskColor);
    DrawRectangle(deskX, deskY, deskW, 16, Color{ 55, 58, 66, 255 }); // Desk rim

    // Desk monitors & CRT equipment
    // Left mini monitor
    DrawRectangle(ox + 520, 420, 90, 80, Color{ 25, 25, 28, 255 });
    DrawRectangle(ox + 528, 428, 74, 64, powerDepleted ? Color{ 8, 8, 10, 255 } : Color{ 15, 30, 20, 255 });
    DrawRectangleLinesEx(Rectangle{ ox + 520, 420, 90, 80 }, 3, Color{ 45, 45, 50, 255 });

    // Right stacked electronics & telephone
    DrawRectangle(ox + 980, 440, 110, 60, Color{ 28, 28, 32, 255 });
    DrawRectangle(ox + 1000, 410, 70, 30, Color{ 35, 35, 40, 255 });
    // Blinking red telephone LED if phone call is active
    DrawCircle(ox + 1010, 425, 4, Color{ 220, 40, 40, 255 });

    // Crumpled soda cup & papers on desk
    DrawRectangle(ox + 460, 470, 22, 30, Color{ 180, 180, 190, 255 });
    DrawRectangle(ox + 880, 485, 35, 15, Color{ 160, 160, 160, 255 });

    // 9. Desk Fan with continuous rotation
    float fanBaseX = ox + 760.0f;
    float fanBaseY = 490.0f;

    // Fan stand
    DrawRectangle(fanBaseX + 35, fanBaseY - 45, 10, 45, Color{ 30, 30, 35, 255 });
    DrawCircle(fanBaseX + 40, fanBaseY, 22, Color{ 25, 25, 30, 255 });

    // Fan head / motor housing
    float fanCenterX = fanBaseX + 40.0f;
    float fanCenterY = fanBaseY - 55.0f;
    DrawCircle(fanCenterX, fanCenterY, 38, Color{ 20, 20, 25, 180 });
    DrawCircleLines(fanCenterX, fanCenterY, 38, Color{ 70, 70, 80, 255 });

    // 3 Rotating Fan Blades
    for (int b = 0; b < 3; ++b) {
        float angleRad = (m_fanBladeAngle + b * 120.0f) * (PI / 180.0f);
        float bx1 = fanCenterX;
        float by1 = fanCenterY;
        float bx2 = fanCenterX + std::cos(angleRad) * 32.0f;
        float by2 = fanCenterY + std::sin(angleRad) * 32.0f;
        DrawLineEx(Vector2{ bx1, by1 }, Vector2{ bx2, by2 }, 6, Color{ 140, 145, 155, 255 });
    }
    // Fan center hub
    DrawCircle(fanCenterX, fanCenterY, 8, Color{ 40, 42, 48, 255 });

    // 10. Vignette & Shadow gradient overlay
    DrawRectangleGradientV(0, 0, SCREEN_WIDTH, 140, Color{ 0, 0, 0, 180 }, Color{ 0, 0, 0, 0 });
    DrawRectangleGradientV(0, SCREEN_HEIGHT - 120, SCREEN_WIDTH, 120, Color{ 0, 0, 0, 0 }, Color{ 0, 0, 0, 160 });
}

} // namespace FNAF
