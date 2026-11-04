#pragma once

#include <string>
#include <vector>
#include <array>
#include <cstdint>

namespace FNAF {

constexpr int SCREEN_WIDTH = 1280;
constexpr int SCREEN_HEIGHT = 720;
constexpr int TARGET_FPS = 60;

// Game State Machine
enum class GameState {
    MainMenu,
    NightIntro,
    Playing,
    Jumpscare,
    GameOver,
    NightComplete,
    Win,
    CustomNightSelect,
    SettingsMenu
};

// Security Camera Identifiers
enum class CameraID {
    CAM_1A = 0, // Show Stage
    CAM_1B,     // Dining Area
    CAM_1C,     // Pirate Cove
    CAM_2A,     // West Hall
    CAM_2B,     // West Hall Corner
    CAM_3,      // Supply Closet
    CAM_4A,     // East Hall
    CAM_4B,     // East Hall Corner
    CAM_5,      // Backstage
    CAM_6,      // Kitchen (Audio Only)
    CAM_7,      // Restrooms
    COUNT
};

inline const char* GetCameraName(CameraID cam) {
    switch (cam) {
        case CameraID::CAM_1A: return "CAM 1A - Show Stage";
        case CameraID::CAM_1B: return "CAM 1B - Dining Area";
        case CameraID::CAM_1C: return "CAM 1C - Pirate Cove";
        case CameraID::CAM_2A: return "CAM 2A - West Hall";
        case CameraID::CAM_2B: return "CAM 2B - West Hall Corner";
        case CameraID::CAM_3:  return "CAM 3 - Supply Closet";
        case CameraID::CAM_4A: return "CAM 4A - East Hall";
        case CameraID::CAM_4B: return "CAM 4B - East Hall Corner";
        case CameraID::CAM_5:  return "CAM 5 - Backstage";
        case CameraID::CAM_6:  return "CAM 6 - Kitchen";
        case CameraID::CAM_7:  return "CAM 7 - Restrooms";
        default: return "UNKNOWN CAM";
    }
}

// Animatronics
enum class AnimatronicID {
    Bear = 0,
    Rabbit,
    Chicken,
    Fox,
    RareVisitor,
    COUNT
};

inline const char* GetAnimatronicName(AnimatronicID id) {
    switch (id) {
        case AnimatronicID::Bear: return "Bear";
        case AnimatronicID::Rabbit: return "Rabbit";
        case AnimatronicID::Chicken: return "Chicken";
        case AnimatronicID::Fox: return "Fox";
        case AnimatronicID::RareVisitor: return "Rare Visitor";
        default: return "Unknown";
    }
}

// Foxy's progression stages
enum class FoxState {
    Hidden = 0,
    Watching,
    Preparing,
    Running,
    Blocked
};

// Internal authoritative locations in the restaurant
enum class LocationID {
    ShowStage = 0,
    DiningArea,
    PirateCove,
    WestHall,
    WestHallCorner,
    SupplyCloset,
    EastHall,
    EastHallCorner,
    Backstage,
    Kitchen,
    Restrooms,
    LeftDoor,
    RightDoor,
    InOffice
};

inline const char* GetLocationName(LocationID loc) {
    switch (loc) {
        case LocationID::ShowStage: return "Show Stage";
        case LocationID::DiningArea: return "Dining Area";
        case LocationID::PirateCove: return "Pirate Cove";
        case LocationID::WestHall: return "West Hall";
        case LocationID::WestHallCorner: return "West Hall Corner";
        case LocationID::SupplyCloset: return "Supply Closet";
        case LocationID::EastHall: return "East Hall";
        case LocationID::EastHallCorner: return "East Hall Corner";
        case LocationID::Backstage: return "Backstage";
        case LocationID::Kitchen: return "Kitchen";
        case LocationID::Restrooms: return "Restrooms";
        case LocationID::LeftDoor: return "Outside Left Door";
        case LocationID::RightDoor: return "Outside Right Door";
        case LocationID::InOffice: return "Inside Office";
        default: return "Unknown";
    }
}

} // namespace FNAF
