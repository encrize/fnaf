#include "Animatronic.h"
#include "Audio.h"
#include "Power.h"
#include <raylib.h>
#include <iostream>
#include <algorithm>

namespace FNAF {

AnimatronicController::AnimatronicController() {
    Reset(0, 0, 0, 0, true);
}

void AnimatronicController::Reset(int bearAI, int rabbitAI, int chickenAI, int foxAI, bool rareVisitorEnabled) {
    m_bearAI = bearAI;
    m_rabbitAI = rabbitAI;
    m_chickenAI = chickenAI;
    m_foxAI = foxAI;
    m_rareVisitorEnabled = rareVisitorEnabled;

    m_bearLoc = LocationID::ShowStage;
    m_rabbitLoc = LocationID::ShowStage;
    m_chickenLoc = LocationID::ShowStage;

    m_bearTimer = 0.0f;
    m_rabbitTimer = 0.0f;
    m_chickenTimer = 0.0f;
    m_foxTimer = 0.0f;

    m_rabbitDoorTimer = 0.0f;
    m_chickenDoorTimer = 0.0f;

    m_foxState = FoxState::Hidden;
    m_foxSprintTimer = 0.0f;
    m_foxDoorHits = 0;

    m_rareVisitorActive = false;
    m_rareVisitorTimer = 0.0f;

    m_jumpscareReady = false;
    m_jumpscareAttacker = AnimatronicID::Bear;
    m_kitchenAudioTimer = 0.0f;
}

void AnimatronicController::SetAILevels(int bear, int rabbit, int chicken, int fox) {
    m_bearAI = std::clamp(bear, 0, 20);
    m_rabbitAI = std::clamp(rabbit, 0, 20);
    m_chickenAI = std::clamp(chicken, 0, 20);
    m_foxAI = std::clamp(fox, 0, 20);
}

void AnimatronicController::ForceMove(AnimatronicID id) {
    switch (id) {
        case AnimatronicID::Rabbit:
            if (m_rabbitLoc == LocationID::ShowStage) m_rabbitLoc = LocationID::Backstage;
            else if (m_rabbitLoc == LocationID::Backstage) m_rabbitLoc = LocationID::DiningArea;
            else if (m_rabbitLoc == LocationID::DiningArea) m_rabbitLoc = LocationID::SupplyCloset;
            else if (m_rabbitLoc == LocationID::SupplyCloset) m_rabbitLoc = LocationID::WestHall;
            else if (m_rabbitLoc == LocationID::WestHall) m_rabbitLoc = LocationID::WestHallCorner;
            else if (m_rabbitLoc == LocationID::WestHallCorner) m_rabbitLoc = LocationID::LeftDoor;
            else m_rabbitLoc = LocationID::DiningArea;
            break;
        case AnimatronicID::Chicken:
            if (m_chickenLoc == LocationID::ShowStage) m_chickenLoc = LocationID::DiningArea;
            else if (m_chickenLoc == LocationID::DiningArea) m_chickenLoc = LocationID::Restrooms;
            else if (m_chickenLoc == LocationID::Restrooms) m_chickenLoc = LocationID::Kitchen;
            else if (m_chickenLoc == LocationID::Kitchen) m_chickenLoc = LocationID::EastHall;
            else if (m_chickenLoc == LocationID::EastHall) m_chickenLoc = LocationID::EastHallCorner;
            else if (m_chickenLoc == LocationID::EastHallCorner) m_chickenLoc = LocationID::RightDoor;
            else m_chickenLoc = LocationID::DiningArea;
            break;
        case AnimatronicID::Bear:
            if (m_bearLoc == LocationID::ShowStage) m_bearLoc = LocationID::DiningArea;
            else if (m_bearLoc == LocationID::DiningArea) m_bearLoc = LocationID::Restrooms;
            else if (m_bearLoc == LocationID::Restrooms) m_bearLoc = LocationID::Kitchen;
            else if (m_bearLoc == LocationID::Kitchen) m_bearLoc = LocationID::EastHall;
            else if (m_bearLoc == LocationID::EastHall) m_bearLoc = LocationID::EastHallCorner;
            else if (m_bearLoc == LocationID::EastHallCorner) m_bearLoc = LocationID::RightDoor;
            else m_bearLoc = LocationID::DiningArea;
            break;
        case AnimatronicID::Fox:
            if (m_foxState == FoxState::Hidden) m_foxState = FoxState::Watching;
            else if (m_foxState == FoxState::Watching) m_foxState = FoxState::Preparing;
            else if (m_foxState == FoxState::Preparing) {
                m_foxState = FoxState::Running;
                m_foxSprintTimer = 3.2f;
            } else {
                m_foxState = FoxState::Hidden;
            }
            break;
        default: break;
    }
}

void AnimatronicController::Update(float dt,
                                   bool monitorUp,
                                   CameraID currentCam,
                                   bool leftDoorClosed,
                                   bool rightDoorClosed,
                                   Audio& audio,
                                   Power& power) {
    if (power.IsDepleted() || m_jumpscareReady) return;

    // Kitchen ambient sounds if Bear or Chicken is in Kitchen
    if (m_chickenLoc == LocationID::Kitchen || m_bearLoc == LocationID::Kitchen) {
        m_kitchenAudioTimer += dt;
        if (m_kitchenAudioTimer > 4.5f) {
            audio.PlayKitchenAudio();
            m_kitchenAudioTimer = 0.0f;
        }
    }

    // ==========================================
    // 1. RABBIT (Bonnie) AI
    // ==========================================
    if (m_rabbitAI > 0) {
        m_rabbitTimer += dt;
        if (m_rabbitTimer >= m_rabbitInterval) {
            m_rabbitTimer = 0.0f;
            int roll = GetRandomValue(1, 20);
            if (roll <= m_rabbitAI) {
                // Move according to route
                switch (m_rabbitLoc) {
                    case LocationID::ShowStage:
                        m_rabbitLoc = LocationID::Backstage;
                        audio.PlayAnimatronicMovement(0.2f);
                        break;
                    case LocationID::Backstage:
                        m_rabbitLoc = LocationID::DiningArea;
                        audio.PlayFootsteps(0.3f);
                        break;
                    case LocationID::DiningArea:
                        m_rabbitLoc = LocationID::SupplyCloset;
                        audio.PlayFootsteps(0.2f);
                        break;
                    case LocationID::SupplyCloset:
                        m_rabbitLoc = LocationID::WestHall;
                        audio.PlayFootsteps(0.15f);
                        break;
                    case LocationID::WestHall:
                        m_rabbitLoc = LocationID::WestHallCorner;
                        audio.PlayFootsteps(0.1f);
                        break;
                    case LocationID::WestHallCorner:
                        m_rabbitLoc = LocationID::LeftDoor;
                        m_rabbitDoorTimer = 0.0f;
                        audio.PlayFootsteps(0.05f);
                        break;
                    case LocationID::LeftDoor:
                        // Already at door, handled below
                        break;
                    default:
                        m_rabbitLoc = LocationID::DiningArea;
                        break;
                }
            }
        }

        // Behavior while outside Left Door
        if (m_rabbitLoc == LocationID::LeftDoor) {
            m_rabbitDoorTimer += dt;
            if (leftDoorClosed) {
                // Retreat after waiting outside closed door (1.5 - 3.5s)
                if (m_rabbitDoorTimer > 2.5f) {
                    m_rabbitLoc = LocationID::DiningArea;
                    m_rabbitDoorTimer = 0.0f;
                    audio.PlayFootsteps(0.2f);
                }
            } else {
                // Attack if door remains open for too long!
                if (m_rabbitDoorTimer > 4.5f) {
                    m_jumpscareReady = true;
                    m_jumpscareAttacker = AnimatronicID::Rabbit;
                }
            }
        }
    }

    // ==========================================
    // 2. CHICKEN (Chica) AI
    // ==========================================
    if (m_chickenAI > 0) {
        m_chickenTimer += dt;
        if (m_chickenTimer >= m_chickenInterval) {
            m_chickenTimer = 0.0f;
            int roll = GetRandomValue(1, 20);
            if (roll <= m_chickenAI) {
                switch (m_chickenLoc) {
                    case LocationID::ShowStage:
                        m_chickenLoc = LocationID::DiningArea;
                        audio.PlayAnimatronicMovement(0.7f);
                        break;
                    case LocationID::DiningArea:
                        m_chickenLoc = LocationID::Restrooms;
                        audio.PlayFootsteps(0.8f);
                        break;
                    case LocationID::Restrooms:
                        m_chickenLoc = LocationID::Kitchen;
                        audio.PlayKitchenAudio();
                        break;
                    case LocationID::Kitchen:
                        m_chickenLoc = LocationID::EastHall;
                        audio.PlayFootsteps(0.85f);
                        break;
                    case LocationID::EastHall:
                        m_chickenLoc = LocationID::EastHallCorner;
                        audio.PlayFootsteps(0.9f);
                        break;
                    case LocationID::EastHallCorner:
                        m_chickenLoc = LocationID::RightDoor;
                        m_chickenDoorTimer = 0.0f;
                        audio.PlayFootsteps(0.95f);
                        break;
                    case LocationID::RightDoor:
                        break;
                    default:
                        m_chickenLoc = LocationID::DiningArea;
                        break;
                }
            }
        }

        // Behavior while outside Right Door
        if (m_chickenLoc == LocationID::RightDoor) {
            m_chickenDoorTimer += dt;
            if (rightDoorClosed) {
                if (m_chickenDoorTimer > 2.5f) {
                    m_chickenLoc = LocationID::DiningArea;
                    m_chickenDoorTimer = 0.0f;
                    audio.PlayFootsteps(0.8f);
                }
            } else {
                if (m_chickenDoorTimer > 4.5f) {
                    m_jumpscareReady = true;
                    m_jumpscareAttacker = AnimatronicID::Chicken;
                }
            }
        }
    }

    // ==========================================
    // 3. BEAR (Freddy) AI
    // ==========================================
    if (m_bearAI > 0) {
        m_bearTimer += dt;
        if (m_bearTimer >= m_bearInterval) {
            m_bearTimer = 0.0f;

            // Bear does not advance if the player is actively viewing his camera!
            bool isBeingWatched = false;
            if (monitorUp) {
                if (m_bearLoc == LocationID::ShowStage && currentCam == CameraID::CAM_1A) isBeingWatched = true;
                else if (m_bearLoc == LocationID::DiningArea && currentCam == CameraID::CAM_1B) isBeingWatched = true;
                else if (m_bearLoc == LocationID::Restrooms && currentCam == CameraID::CAM_7) isBeingWatched = true;
                else if (m_bearLoc == LocationID::Kitchen && currentCam == CameraID::CAM_6) isBeingWatched = true;
                else if (m_bearLoc == LocationID::EastHall && currentCam == CameraID::CAM_4A) isBeingWatched = true;
                else if (m_bearLoc == LocationID::EastHallCorner && currentCam == CameraID::CAM_4B) isBeingWatched = true;
            }

            if (!isBeingWatched) {
                int roll = GetRandomValue(1, 20);
                if (roll <= m_bearAI) {
                    switch (m_bearLoc) {
                        case LocationID::ShowStage:
                            // Only leaves after Rabbit and Chicken have moved
                            if (m_rabbitLoc != LocationID::ShowStage && m_chickenLoc != LocationID::ShowStage) {
                                m_bearLoc = LocationID::DiningArea;
                                audio.PlayAnimatronicMovement(0.5f);
                            }
                            break;
                        case LocationID::DiningArea:
                            m_bearLoc = LocationID::Restrooms;
                            audio.PlayAnimatronicMovement(0.7f);
                            break;
                        case LocationID::Restrooms:
                            m_bearLoc = LocationID::Kitchen;
                            audio.PlayKitchenAudio();
                            break;
                        case LocationID::Kitchen:
                            m_bearLoc = LocationID::EastHall;
                            audio.PlayAnimatronicMovement(0.8f);
                            break;
                        case LocationID::EastHall:
                            m_bearLoc = LocationID::EastHallCorner;
                            audio.PlayAnimatronicMovement(0.9f);
                            break;
                        case LocationID::EastHallCorner:
                            if (!rightDoorClosed) {
                                // Right door open: Bear sneaks inside the office!
                                m_bearLoc = LocationID::RightDoor;
                                audio.PlayAnimatronicMovement(0.95f);
                            }
                            break;
                        case LocationID::RightDoor:
                            // Attack!
                            if (!rightDoorClosed) {
                                m_jumpscareReady = true;
                                m_jumpscareAttacker = AnimatronicID::Bear;
                            } else {
                                m_bearLoc = LocationID::EastHallCorner;
                            }
                            break;
                        default:
                            m_bearLoc = LocationID::DiningArea;
                            break;
                    }
                }
            }
        }
    }

    // ==========================================
    // 4. FOX (Foxy) AI
    // ==========================================
    if (m_foxAI > 0) {
        // Looking at Pirate Cove slows / resets Fox's progression
        bool watchingPirateCove = monitorUp && (currentCam == CameraID::CAM_1C);

        if (m_foxState != FoxState::Running) {
            if (!watchingPirateCove) {
                m_foxTimer += dt;
                if (m_foxTimer >= m_foxInterval) {
                    m_foxTimer = 0.0f;
                    int roll = GetRandomValue(1, 20);
                    if (roll <= m_foxAI) {
                        if (m_foxState == FoxState::Hidden) {
                            m_foxState = FoxState::Watching;
                        } else if (m_foxState == FoxState::Watching) {
                            m_foxState = FoxState::Preparing;
                        } else if (m_foxState == FoxState::Preparing) {
                            // Fox begins his sprint down the West Hall!
                            m_foxState = FoxState::Running;
                            m_foxSprintTimer = 3.5f; // Player has ~3.5 seconds to shut Left Door!
                            audio.PlayFootsteps(0.15f);
                        }
                    }
                }
            } else {
                // Stalled by watching Pirate Cove
                m_foxTimer = 0.0f;
            }
        } else {
            // Fox is currently running down the hall!
            m_foxSprintTimer -= dt;
            if (m_foxSprintTimer <= 0.0f) {
                // Arrives at the left office door
                if (leftDoorClosed) {
                    // Blocked!
                    audio.PlayFoxKnock();
                    m_foxDoorHits++;
                    float drain = 1.0f + 5.0f * static_cast<float>(m_foxDoorHits);
                    power.DrainPower(drain);

                    // Reset back to Pirate Cove
                    m_foxState = FoxState::Hidden;
                    m_foxTimer = 0.0f;
                } else {
                    // Door open: Fox jumpscare!
                    m_jumpscareReady = true;
                    m_jumpscareAttacker = AnimatronicID::Fox;
                }
            }
        }
    }

    // ==========================================
    // 5. RARE VISITOR (Golden Bear)
    // ==========================================
    if (m_rareVisitorEnabled) {
        if (m_rareVisitorActive) {
            m_rareVisitorTimer += dt;
            // Player must flip monitor up to dismiss hallucination!
            if (monitorUp) {
                m_rareVisitorActive = false;
                m_rareVisitorTimer = 0.0f;
            } else if (m_rareVisitorTimer > 3.0f) {
                // Failure to react: sudden jumpscare!
                m_jumpscareReady = true;
                m_jumpscareAttacker = AnimatronicID::RareVisitor;
            }
        }
    }
}

} // namespace FNAF
