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

        // Power drain intervals (matching canonical FNAF 1 / FNaF-1-3DS):
        // Usage 1: 1% every 8.0s (0.125%/s)
        // Usage 2: 1% every 4.5s (0.222%/s)
        // Usage 3: 1% every 2.3s (0.435%/s)
        // Usage 4: 1% every 1.1s (0.909%/s)
        // Usage 5: 1% every 0.6s (1.667%/s)
        float drainInterval = 8.0f;
        if (m_usageLevel == 2) drainInterval = 4.5f;
        else if (m_usageLevel == 3) drainInterval = 2.3f;
        else if (m_usageLevel == 4) drainInterval = 1.1f;
        else if (m_usageLevel >= 5) drainInterval = 0.6f;

        m_power -= (1.0f / drainInterval) * dt;
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
