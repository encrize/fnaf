#pragma once

#include <vector>
#include <string>
#include "Common.h"

namespace FNAF {

class Audio;

class AnimatronicController {
private:
    // Individual AI levels (0 to 20)
    int m_bearAI = 0;
    int m_rabbitAI = 0;
    int m_chickenAI = 0;
    int m_foxAI = 0;

    // Locations
    LocationID m_bearLoc = LocationID::ShowStage;
    LocationID m_rabbitLoc = LocationID::ShowStage;
    LocationID m_chickenLoc = LocationID::ShowStage;

    // Movement opportunity timers
    float m_bearTimer = 0.0f;
    float m_rabbitTimer = 0.0f;
    float m_chickenTimer = 0.0f;
    float m_foxTimer = 0.0f;

    // Movement intervals (FNAF 1 accurate intervals)
    const float m_bearInterval = 3.02f;
    const float m_rabbitInterval = 4.97f;
    const float m_chickenInterval = 4.98f;
    const float m_foxInterval = 5.01f;

    // Retreat countdown timers when doors are closed
    float m_rabbitDoorTimer = 0.0f;
    float m_chickenDoorTimer = 0.0f;

    // Fox specific state
    FoxState m_foxState = FoxState::Hidden;
    float m_foxSprintTimer = 0.0f;
    int m_foxDoorHits = 0;

    // Rare Visitor (Golden Bear)
    bool m_rareVisitorActive = false;
    float m_rareVisitorTimer = 0.0f;
    bool m_rareVisitorEnabled = true;

    // Attack / Jumpscare trigger
    bool m_jumpscareReady = false;
    AnimatronicID m_jumpscareAttacker = AnimatronicID::Bear;

    // Kitchen audio timer
    float m_kitchenAudioTimer = 0.0f;

public:
    AnimatronicController();

    void Reset(int bearAI, int rabbitAI, int chickenAI, int foxAI, bool rareVisitorEnabled);
    void Update(float dt,
                bool monitorUp,
                CameraID currentCam,
                bool leftDoorClosed,
                bool rightDoorClosed,
                Audio& audio,
                class Power& power);

    // Getters
    LocationID GetBearLocation() const { return m_bearLoc; }
    LocationID GetRabbitLocation() const { return m_rabbitLoc; }
    LocationID GetChickenLocation() const { return m_chickenLoc; }
    FoxState GetFoxState() const { return m_foxState; }
    bool IsRabbitAtDoor() const { return m_rabbitLoc == LocationID::LeftDoor; }
    bool IsChickenAtDoor() const { return m_chickenLoc == LocationID::RightDoor; }
    bool IsRareVisitorActive() const { return m_rareVisitorActive; }

    bool IsJumpscareReady() const { return m_jumpscareReady; }
    AnimatronicID GetJumpscareAttacker() const { return m_jumpscareAttacker; }

    // Debug & cheats
    int GetBearAI() const { return m_bearAI; }
    int GetRabbitAI() const { return m_rabbitAI; }
    int GetChickenAI() const { return m_chickenAI; }
    int GetFoxAI() const { return m_foxAI; }
    void ForceMove(AnimatronicID id);
    void SetAILevels(int bear, int rabbit, int chicken, int fox);
    void TriggerGoldenBear() { m_rareVisitorActive = true; m_rareVisitorTimer = 0.0f; }
};

} // namespace FNAF
