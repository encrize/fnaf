#pragma once

#include "Common.h"

namespace FNAF {

class Audio;

class Power {
private:
    float m_power = 100.0f;
    int m_usageLevel = 1; // 1 to 5
    bool m_depleted = false;

    // Power-out sequence variables
    float m_blackoutTimer = 0.0f;
    float m_musicBoxDuration = 0.0f;
    float m_silenceDuration = 0.0f;
    bool m_musicBoxPlaying = false;
    bool m_bearFlicker = false;
    bool m_jumpscareReady = false;

public:
    Power();

    void Reset();
    void Update(float dt, bool leftDoor, bool rightDoor, bool leftLight, bool rightLight, bool monitorUp, Audio& audio);

    float GetPower() const { return m_power; }
    int GetUsageLevel() const { return m_usageLevel; }
    bool IsDepleted() const { return m_depleted; }

    // Power out blackout sequence getters
    bool IsBearFlickering() const { return m_bearFlicker; }
    bool IsJumpscareReady() const { return m_jumpscareReady; }

    // Cheat / debug methods
    void DrainPower(float amount);
    void DepleteInstantly();
};

} // namespace FNAF
