#include <cassert>
#include <iostream>
#include <cmath>
#include "Common.h"
#include "Settings.h"
#include "NightManager.h"
#include "Power.h"
#include "Animatronic.h"
#include "Office.h"
#include "Cameras.h"
#include "Audio.h"
#include "AssetManager.h"

void TestSettings() {
    std::cout << "[TEST] Running Settings Test...\n";
    FNAF::Settings settings;
    settings.masterVolume = 0.75f;
    settings.fullscreen = false;
    settings.crtEffects = true;
    assert(settings.SaveToFile("test_settings.json"));

    FNAF::Settings loaded;
    assert(loaded.LoadFromFile("test_settings.json"));
    assert(std::fabs(loaded.masterVolume - 0.75f) < 0.01f);
    assert(!loaded.fullscreen);
    assert(loaded.crtEffects);
    std::remove("test_settings.json");
    std::cout << "[TEST] Settings Test PASSED.\n";
}

void TestNightManager() {
    std::cout << "[TEST] Running NightManager Test...\n";
    FNAF::NightManager nm;
    nm.SetHourDuration(1.0f); // 1 second per hour for fast test
    nm.StartNight(1);

    assert(nm.GetCurrentHour() == 0);
    assert(!nm.IsNightComplete());

    // Advance 3.5 seconds -> should be 3 AM
    nm.Update(3.5f);
    assert(nm.GetCurrentHour() == 3);
    assert(!nm.IsNightComplete());

    // Advance to 6 seconds -> 6 AM!
    nm.Update(3.0f);
    assert(nm.GetCurrentHour() >= 6);
    assert(nm.IsNightComplete());

    // Test phone call active and skip
    nm.StartNight(2);
    assert(nm.IsPhoneCallActive());
    nm.Update(0.5f);
    assert(nm.IsPhoneCallActive());
    nm.SkipPhoneCall();
    assert(!nm.IsPhoneCallActive());

    // Test save/load
    nm.UnlockNextNight();
    assert(nm.GetHighestUnlockedNight() >= 2);
    assert(nm.SaveGame("test_save.dat"));

    FNAF::NightManager nm2;
    assert(nm2.LoadSave("test_save.dat"));
    assert(nm2.GetHighestUnlockedNight() == nm.GetHighestUnlockedNight());
    std::remove("test_save.dat");

    std::cout << "[TEST] NightManager Test PASSED.\n";
}

void TestPowerSystem(FNAF::Audio& audio) {
    std::cout << "[TEST] Running Power System Test...\n";
    FNAF::Power power;
    assert(power.GetPower() == 100.0f);
    assert(power.GetUsageLevel() == 1);
    assert(!power.IsDepleted());

    // Base usage drain: 1 usage bar = 0.105% per second
    power.Update(10.0f, false, false, false, false, false, audio);
    assert(power.GetPower() < 100.0f);
    assert(power.GetPower() > 98.0f);

    // Multi-system usage drain: Left Door closed + Right Door closed + Monitor up = 4 bars
    power.Update(10.0f, true, true, false, false, true, audio);
    assert(power.GetUsageLevel() == 4);

    // Deplete power to 0
    power.DepleteInstantly();
    assert(power.IsDepleted());
    assert(power.GetPower() == 0.0f);

    // Update through blackout sequence phases
    // Phase 1 (flickering eyes): after 3 seconds
    power.Update(3.0f, false, false, false, false, false, audio);
    // Phase 3 (jumpscare ready): after blackout timer expires (~20s)
    power.Update(25.0f, false, false, false, false, false, audio);
    assert(power.IsJumpscareReady());

    std::cout << "[TEST] Power System Test PASSED.\n";
}

void TestCameras(FNAF::Audio& audio) {
    std::cout << "[TEST] Running Cameras Test...\n";
    FNAF::Cameras cams;
    assert(cams.GetCurrentCamera() == FNAF::CameraID::CAM_1A);
    assert(!cams.IsMonitorUp());

    cams.ToggleMonitor(audio);
    assert(cams.IsMonitorUp());

    cams.SetCurrentCamera(FNAF::CameraID::CAM_1C, audio);
    assert(cams.GetCurrentCamera() == FNAF::CameraID::CAM_1C);

    cams.SetCurrentCamera(FNAF::CameraID::CAM_6, audio); // Kitchen
    assert(cams.GetCurrentCamera() == FNAF::CameraID::CAM_6);

    // Verify all 11 cameras have valid non-empty names
    for (int i = 0; i < static_cast<int>(FNAF::CameraID::COUNT); ++i) {
        const char* name = FNAF::GetCameraName(static_cast<FNAF::CameraID>(i));
        assert(name != nullptr && std::string(name).length() > 0);
        (void)name;
    }

    // Verify map buttons and room hitboxes are correctly aligned
    const auto& buttons = cams.GetMapButtons();
    assert(buttons.size() == 11);

    // Verify CAM_1C (Pirate Cove) button and hitbox covers the room rectangle
    for (const auto& btn : buttons) {
        if (btn.id == FNAF::CameraID::CAM_1C) {
            assert(CheckCollisionPointRec(Vector2{ 920.0f, 500.0f }, btn.hitRect));
            assert(CheckCollisionPointRec(Vector2{ 920.0f, 500.0f }, btn.rect));
        }
        if (btn.id == FNAF::CameraID::CAM_6) {
            // Kitchen room
            assert(CheckCollisionPointRec(Vector2{ 1180.0f, 580.0f }, btn.hitRect));
            assert(CheckCollisionPointRec(Vector2{ 1180.0f, 580.0f }, btn.rect));
        }
    }

    std::cout << "[TEST] Cameras Test PASSED.\n";
}

void TestAnimatronicAI(FNAF::Audio& audio) {
    std::cout << "[TEST] Running Animatronic AI Test...\n";
    FNAF::AnimatronicController ai;
    FNAF::Power power;

    // Test 1: Rabbit movement and door retreat
    ai.Reset(0, 20, 0, 0, false); // Rabbit max difficulty
    assert(ai.GetRabbitLocation() == FNAF::LocationID::ShowStage);

    // Advance Rabbit step by step
    while (ai.GetRabbitLocation() != FNAF::LocationID::LeftDoor) {
        ai.Update(5.0f, false, FNAF::CameraID::CAM_1A, false, false, audio, power);
    }
    assert(ai.IsRabbitAtDoor());

    // Door closed -> Rabbit retreats
    ai.Update(3.0f, false, FNAF::CameraID::CAM_1A, true, false, audio, power);
    assert(!ai.IsRabbitAtDoor());
    assert(ai.GetRabbitLocation() == FNAF::LocationID::DiningArea);

    // Test 2: Chicken movement and door defense
    ai.Reset(0, 0, 20, 0, false); // Chicken max difficulty
    assert(ai.GetChickenLocation() == FNAF::LocationID::ShowStage);
    while (ai.GetChickenLocation() != FNAF::LocationID::RightDoor) {
        ai.Update(5.0f, false, FNAF::CameraID::CAM_1A, false, false, audio, power);
    }
    assert(ai.IsChickenAtDoor());

    // Door closed -> Chicken retreats
    ai.Update(3.0f, false, FNAF::CameraID::CAM_1A, false, true, audio, power);
    assert(!ai.IsChickenAtDoor());

    // Test 3: Fox Pirate Cove progression & Door Blocking
    ai.Reset(0, 0, 0, 20, false); // Fox max difficulty
    assert(ai.GetFoxState() == FNAF::FoxState::Hidden);

    // Stalling by watching Pirate Cove
    ai.Update(6.0f, true, FNAF::CameraID::CAM_1C, false, false, audio, power);
    assert(ai.GetFoxState() == FNAF::FoxState::Hidden); // Did not progress because camera was on CAM 1C!

    // Not watching Pirate Cove -> Progresses to Watching -> Preparing -> Running
    while (ai.GetFoxState() != FNAF::FoxState::Running) {
        ai.Update(6.0f, false, FNAF::CameraID::CAM_1A, false, false, audio, power);
    }
    assert(ai.GetFoxState() == FNAF::FoxState::Running);

    // Left door closed during sprint -> Fox knocks and resets to Hidden!
    float initialPower = power.GetPower();
    ai.Update(4.0f, false, FNAF::CameraID::CAM_1A, true, false, audio, power);
    assert(ai.GetFoxState() == FNAF::FoxState::Hidden);
    assert(power.GetPower() < initialPower); // Power drained by knock!
    (void)initialPower;

    // Test 4: Open door attack triggers jumpscare
    ai.Reset(0, 20, 0, 0, false);
    while (ai.GetRabbitLocation() != FNAF::LocationID::LeftDoor) {
        ai.Update(5.0f, false, FNAF::CameraID::CAM_1A, false, false, audio, power);
    }
    // Leave door open for > 4.5s
    ai.Update(5.0f, false, FNAF::CameraID::CAM_1A, false, false, audio, power);
    assert(ai.IsJumpscareReady());
    assert(ai.GetJumpscareAttacker() == FNAF::AnimatronicID::Rabbit);

    // Test 5: Golden Bear hallucination dismissal
    ai.Reset(0, 0, 0, 0, true);
    ai.TriggerGoldenBear();
    assert(ai.IsRareVisitorActive());
    // Player flips monitor up -> dismisses Golden Bear!
    ai.Update(0.1f, true, FNAF::CameraID::CAM_1A, false, false, audio, power);
    assert(!ai.IsRareVisitorActive());

    std::cout << "[TEST] Animatronic AI Test PASSED.\n";
}

int main() {
    std::cout << "========================================\n";
    std::cout << "  FNAF RECREATION - GAMEPLAY TEST SUITE \n";
    std::cout << "========================================\n";

    InitAudioDevice();

    FNAF::Audio audio;
    audio.Initialize();

    TestSettings();
    TestNightManager();
    TestPowerSystem(audio);
    TestCameras(audio);
    TestAnimatronicAI(audio);

    audio.Cleanup();

    std::cout << "========================================\n";
    std::cout << "  ALL GAMEPLAY TESTS PASSED PERFECTLY!  \n";
    std::cout << "========================================\n";
    return 0;
}
