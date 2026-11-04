#include "Power.h"
#include "Audio.h"
#include <raylib.h>
#include <algorithm>

namespace FNAF {

Power::Power() {
    Reset();
}

void Power::Reset() {
    m_power = 100.0f;
    m_usageLevel = 1;
    m_depleted = false;
    m_blackoutTimer = 0.0f;
    m_musicBoxDuration = static_cast<float>(GetRandomValue(4, 12));
    m_silenceDuration = static_cast<float>(GetRandomValue(2, 5));
    m_musicBoxPlaying = false;
    m_bearFlicker = false;
    m_jumpscareReady = false;
}

void Power::DrainPower(float amount) {
    m_power = std::max(0.0f, m_power - amount);
    if (m_power <= 0.0f && !m_depleted) {
        m_depleted = true;
    }
}

void Power::DepleteInstantly() {
    DrainPower(100.0f);
}

void Power::Update(float dt, bool leftDoor, bool rightDoor, bool leftLight, bool rightLight, bool monitorUp, Audio& audio) {
    if (!m_depleted) {
        // Calculate usage level
        m_usageLevel = 1; // Base
        if (leftDoor) m_usageLevel++;
        if (rightDoor) m_usageLevel++;
        if (leftLight) m_usageLevel++;
        if (rightLight) m_usageLevel++;
        if (monitorUp) m_usageLevel++;

        // Base drain rate per usage bar: ~0.105% per second
        float drainRate = m_usageLevel * 0.105f;
        m_power -= drainRate * dt;

        if (m_power <= 0.0f) {
            m_power = 0.0f;
            m_depleted = true;
            m_blackoutTimer = 0.0f;
            m_musicBoxDuration = static_cast<float>(GetRandomValue(4, 12));
            m_silenceDuration = static_cast<float>(GetRandomValue(2, 5));
            m_musicBoxPlaying = false;
            m_bearFlicker = false;
            m_jumpscareReady = false;

            audio.StopAllSounds();
            audio.PlayPowerOut();
        }
    } else {
        // Power-out sequence
        m_blackoutTimer += dt;

        // Phase 1: Wait 2 seconds in dark, then Bear appears flickering with music box
        if (m_blackoutTimer > 2.0f && m_blackoutTimer <= 2.0f + m_musicBoxDuration) {
            if (!m_musicBoxPlaying) {
                audio.PlayPowerOutMusic(true);
                m_musicBoxPlaying = true;
            }
            // Flickering eyes effect
            m_bearFlicker = (GetRandomValue(0, 10) > 3);
        }
        // Phase 2: Music stops, complete darkness & silence
        else if (m_blackoutTimer > 2.0f + m_musicBoxDuration &&
                 m_blackoutTimer <= 2.0f + m_musicBoxDuration + m_silenceDuration) {
            if (m_musicBoxPlaying) {
                audio.PlayPowerOutMusic(false);
                m_musicBoxPlaying = false;
            }
            m_bearFlicker = false;
        }
        // Phase 3: Jumpscare triggered!
        else if (m_blackoutTimer > 2.0f + m_musicBoxDuration + m_silenceDuration) {
            m_jumpscareReady = true;
        }
    }
}

} // namespace FNAF
