#pragma once

#include <raylib.h>
#include "Common.h"

namespace FNAF {

class Audio;

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

    // Fan animation
    float m_fanBladeAngle = 0.0f;

    // Interactive button bounds (in office space)
    Rectangle m_leftDoorButton{};
    Rectangle m_leftLightButton{};
    Rectangle m_rightDoorButton{};
    Rectangle m_rightLightButton{};

public:
    Office();

    void Reset();
    void Update(float dt, bool powerDepleted, Audio& audio);
    void HandleInput(bool monitorUp, bool powerDepleted, Audio& audio);
    void Render(bool rabbitAtDoor, bool chickenAtDoor, bool bearPowerOutFlicker, bool rareVisitorActive, bool powerDepleted);

    float GetCameraX() const { return m_cameraX; }
    bool IsLeftDoorClosed() const { return m_leftDoorClosed; }
    bool IsRightDoorClosed() const { return m_rightDoorClosed; }
    bool IsLeftLightActive() const { return m_leftLightActive; }
    bool IsRightLightActive() const { return m_rightLightActive; }

    void SetLeftDoor(bool closed, Audio& audio);
    void SetRightDoor(bool closed, Audio& audio);
    void SetLeftLight(bool active, Audio& audio);
    void SetRightLight(bool active, Audio& audio);
};

} // namespace FNAF
