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
    m_officeMain = assets.GetTexture("assets/images/office/office_main.png");
    m_officeLightL = assets.GetTexture("assets/images/office/office_light_l.png");
    m_officeLightLBonnie = assets.GetTexture("assets/images/office/office_light_l_bonnie.png");
    m_officeLightR = assets.GetTexture("assets/images/office/office_light_r.png");
    m_officeLightRChica = assets.GetTexture("assets/images/office/office_light_r_chica.png");
    m_officeFlicker1 = assets.GetTexture("assets/images/office/office_freddy_flicker1.png");
    m_officeFlicker2 = assets.GetTexture("assets/images/office/office_freddy_flicker2.png");
    m_officeBlackout = assets.GetTexture("assets/images/office/office_blackout.png");

    m_officeNormalFrames.clear();
    m_officeNormalFrames.push_back(assets.GetTexture("assets/images/office/office_frame1.png"));
    m_officeNormalFrames.push_back(assets.GetTexture("assets/images/office/office_frame2.png"));
    m_officeNormalFrames.push_back(assets.GetTexture("assets/images/office/office_frame3.png"));

    m_doorLFrames.clear();
    const char* doorLFiles[] = {
        "88.png", "89.png", "91.png", "92.png", "93.png", "94.png", "95.png",
        "96.png", "97.png", "98.png", "99.png", "100.png", "101.png", "102.png"
    };
    for (const char* f : doorLFiles) {
        m_doorLFrames.push_back(assets.GetTexture(std::string("assets/images/doors/") + f));
    }

    m_doorRFrames.clear();
    const char* doorRFiles[] = {
        "104.png", "106.png", "107.png", "108.png", "109.png", "110.png", "111.png",
        "112.png", "113.png", "114.png", "115.png", "116.png", "117.png", "118.png"
    };
    for (const char* f : doorRFiles) {
        m_doorRFrames.push_back(assets.GetTexture(std::string("assets/images/doors/") + f));
    }

    m_btnL_Off   = assets.GetTexture("assets/images/buttons/122.png");
    m_btnL_Door  = assets.GetTexture("assets/images/buttons/124.png");
    m_btnL_Light = assets.GetTexture("assets/images/buttons/125.png");
    m_btnL_Both  = assets.GetTexture("assets/images/buttons/130.png");

    m_btnR_Off   = assets.GetTexture("assets/images/buttons/134.png");
    m_btnR_Door  = assets.GetTexture("assets/images/buttons/135.png");
    m_btnR_Light = assets.GetTexture("assets/images/buttons/131.png");
    m_btnR_Both  = assets.GetTexture("assets/images/buttons/47.png");

    m_goldenBearTex = assets.GetTexture("assets/images/golden_bear.png");
}

void Office::Reset() {
    m_cameraX = m_maxPan * 0.5f;
    m_targetCameraX = m_cameraX;
    m_leftDoorClosed = false;
    m_rightDoorClosed = false;
    m_leftDoorProgress = 0.0f;
    m_rightDoorProgress = 0.0f;
    m_leftLightActive = false;
    m_rightLightActive = false;
    m_officeFrameIdx = 0;
    m_officeFrameTimer = 0.0f;

    m_leftDoorButton  = Rectangle{ 55.0f, 230.0f, 92.0f, 120.0f };
    m_leftLightButton = Rectangle{ 55.0f, 350.0f, 92.0f, 120.0f };

    m_rightDoorButton  = Rectangle{ 1495.0f, 230.0f, 92.0f, 120.0f };
    m_rightLightButton = Rectangle{ 1495.0f, 350.0f, 92.0f, 120.0f };
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

void Office::SetLeftLight(bool active, bool rabbitAtDoor, Audio& audio) {
    if (m_leftLightActive != active) {
        m_leftLightActive = active;
        audio.SetLightBuzz(m_leftLightActive || m_rightLightActive);
        if (active && rabbitAtDoor) {
            audio.PlayWindowScare();
        }
    }
}

void Office::SetRightLight(bool active, bool chickenAtDoor, Audio& audio) {
    if (m_rightLightActive != active) {
        m_rightLightActive = active;
        audio.SetLightBuzz(m_leftLightActive || m_rightLightActive);
        if (active && chickenAtDoor) {
            audio.PlayWindowScare();
        }
    }
}

void Office::HandleInput(Vector2 mousePos, bool monitorUp, bool powerDepleted, bool rabbitAtDoor, bool chickenAtDoor, Audio& audio) {
    if (monitorUp || powerDepleted) {
        SetLeftLight(false, false, audio);
        SetRightLight(false, false, audio);
        return;
    }

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
    SetLeftLight(leftLightDesired, rabbitAtDoor, audio);

    if (IsKeyPressed(KEY_E) || (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(worldMouse, m_rightDoorButton))) {
        SetRightDoor(!m_rightDoorClosed, audio);
    }

    bool rightLightDesired = (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(worldMouse, m_rightLightButton)) || IsKeyDown(KEY_C);
    SetRightLight(rightLightDesired, chickenAtDoor, audio);
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

    if (!powerDepleted && !m_officeNormalFrames.empty()) {
        m_officeFrameTimer += dt;
        if (m_officeFrameTimer >= 0.045f) {
            m_officeFrameTimer = 0.0f;
            m_officeFrameIdx = (m_officeFrameIdx + 1) % m_officeNormalFrames.size();
        }
    }
}

void Office::Render(bool rabbitAtDoor, bool chickenAtDoor, bool bearPowerOutFlicker, bool rareVisitorActive, bool powerDepleted) {
    float ox = -m_cameraX;

    // 1. Office Background Selection (with natural animated fan)
    Texture2D activeBg = m_officeNormalFrames.empty() ? m_officeMain : m_officeNormalFrames[m_officeFrameIdx];

    if (powerDepleted) {
        if (bearPowerOutFlicker) {
            activeBg = (GetRandomValue(0, 10) > 4) ? m_officeFlicker1 : m_officeFlicker2;
        } else {
            activeBg = m_officeBlackout;
        }
    } else if (m_leftLightActive) {
        activeBg = rabbitAtDoor ? m_officeLightLBonnie : m_officeLightL;
    } else if (m_rightLightActive) {
        activeBg = chickenAtDoor ? m_officeLightRChica : m_officeLightR;
    }

    int drawOx = static_cast<int>(std::round(ox));

    if (activeBg.id != 0) {
        DrawTexture(activeBg, drawOx, 0, WHITE);
    } else {
        DrawRectangle(drawOx, 0, 1600, SCREEN_HEIGHT, powerDepleted ? BLACK : Color{ 30, 32, 38, 255 });
    }

    // 2. Animated Doors
    if (m_leftDoorProgress > 0.01f && !m_doorLFrames.empty()) {
        int idx = std::clamp(static_cast<int>(m_leftDoorProgress * (m_doorLFrames.size() - 1)), 0, static_cast<int>(m_doorLFrames.size() - 1));
        if (m_doorLFrames[idx].id != 0) {
            DrawTexture(m_doorLFrames[idx], drawOx + 72, 0, WHITE);
        }
    }

    if (m_rightDoorProgress > 0.01f && !m_doorRFrames.empty()) {
        int idx = std::clamp(static_cast<int>(m_rightDoorProgress * (m_doorRFrames.size() - 1)), 0, static_cast<int>(m_doorRFrames.size() - 1));
        if (m_doorRFrames[idx].id != 0) {
            DrawTexture(m_doorRFrames[idx], drawOx + 1305, 0, WHITE);
        }
    }

    // 3. Animated Button Panels
    if (!powerDepleted) {
        // Left Button Panel (55, 230)
        Texture2D lBtn = m_btnL_Off;
        if (m_leftDoorClosed && m_leftLightActive) lBtn = m_btnL_Both;
        else if (m_leftDoorClosed) lBtn = m_btnL_Door;
        else if (m_leftLightActive) lBtn = m_btnL_Light;

        if (lBtn.id != 0) {
            DrawTexture(lBtn, drawOx + 55, 230, WHITE);
        }

        // Right Button Panel (1495, 230)
        Texture2D rBtn = m_btnR_Off;
        if (m_rightDoorClosed && m_rightLightActive) rBtn = m_btnR_Both;
        else if (m_rightDoorClosed) rBtn = m_btnR_Door;
        else if (m_rightLightActive) rBtn = m_btnR_Light;

        if (rBtn.id != 0) {
            DrawTexture(rBtn, drawOx + 1495, 230, WHITE);
        }
    }

    // 4. Golden Bear
    if (rareVisitorActive && !powerDepleted) {
        if (m_goldenBearTex.id != 0) {
            DrawTexture(m_goldenBearTex, drawOx + 680, 480, WHITE);
        }
    }
}

} // namespace FNAF
