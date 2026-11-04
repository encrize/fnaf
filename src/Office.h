#pragma once

#include <raylib.h>
#include <vector>
#include "Common.h"

namespace FNAF {

class Audio;
class AssetManager;

class Office {
private:
    float m_cameraX = 0.0f;
    float m_targetCameraX = 0.0f;
    const float m_maxPan = 320.0f; // Office total width = SCREEN_WIDTH + m_maxPan (1600px)

    // Door states & animation progress (0.0 = fully open, 1.0 = fully closed)
    bool m_leftDoorClosed = false;
    bool m_rightDoorClosed = false;
    float m_leftDoorProgress = 0.0f;
    float m_rightDoorProgress = 0.0f;

    // Light states
    bool m_leftLightActive = false;
    bool m_rightLightActive = false;

    // Office animated frames (integrated spinning desk fan)
    std::vector<Texture2D> m_officeNormalFrames;
    int m_officeFrameIdx = 0;
    float m_officeFrameTimer = 0.0f;

    // Interactive button bounds (in office space)
    Rectangle m_leftDoorButton{};
    Rectangle m_leftLightButton{};
    Rectangle m_rightDoorButton{};
    Rectangle m_rightLightButton{};

    // Texture Sets
    Texture2D m_officeMain{};
    Texture2D m_officeLightL{};
    Texture2D m_officeLightLBonnie{};
    Texture2D m_officeLightR{};
    Texture2D m_officeLightRChica{};
    Texture2D m_officeFlicker1{};
    Texture2D m_officeFlicker2{};
    Texture2D m_officeBlackout{};

    std::vector<Texture2D> m_doorLFrames;
    std::vector<Texture2D> m_doorRFrames;

    Texture2D m_btnL_Off{};
    Texture2D m_btnL_Door{};
    Texture2D m_btnL_Light{};
    Texture2D m_btnL_Both{};

    Texture2D m_btnR_Off{};
    Texture2D m_btnR_Door{};
    Texture2D m_btnR_Light{};
    Texture2D m_btnR_Both{};

    Texture2D m_goldenBearTex{};

public:
    Office();

    void LoadAssets(AssetManager& assets);
    void Reset();
    void Update(float dt, bool powerDepleted, Audio& audio);
    void HandleInput(Vector2 mousePos, bool monitorUp, bool powerDepleted, bool rabbitAtDoor, bool chickenAtDoor, Audio& audio);
    void Render(bool rabbitAtDoor, bool chickenAtDoor, bool bearPowerOutFlicker, bool rareVisitorActive, bool powerDepleted);

    float GetCameraX() const { return m_cameraX; }
    bool IsLeftDoorClosed() const { return m_leftDoorClosed; }
    bool IsRightDoorClosed() const { return m_rightDoorClosed; }
    bool IsLeftLightActive() const { return m_leftLightActive; }
    bool IsRightLightActive() const { return m_rightLightActive; }

    void SetLeftDoor(bool closed, Audio& audio);
    void SetRightDoor(bool closed, Audio& audio);
    void SetLeftLight(bool active, bool rabbitAtDoor, Audio& audio);
    void SetRightLight(bool active, bool chickenAtDoor, Audio& audio);
};

} // namespace FNAF
