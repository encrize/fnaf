#pragma once

#include <vector>
#include <string>
#include "Common.h"

namespace FNAF {

struct NightDifficulty {
    int night = 1;
    int bearAI = 0;
    int rabbitAI = 0;
    int chickenAI = 0;
    int foxAI = 0;
};

class NightManager {
private:
    int m_currentNight = 1;
    int m_highestUnlockedNight = 1;
    float m_nightTimer = 0.0f;
    float m_hourDuration = 60.0f; // 60 seconds per hour (360 seconds per night)
    int m_currentHour = 0;        // 0: 12 AM, 1: 1 AM, ..., 6: 6 AM

    std::vector<NightDifficulty> m_nightConfigs;
    NightDifficulty m_customNightConfig{ 7, 0, 0, 0, 0 };

    // Phone call tracking
    float m_phoneCallTimer = 0.0f;
    float m_phoneCallDuration = 60.0f;
    bool m_phoneCallActive = false;

    void SetupDefaultDifficulties();

public:
    NightManager();

    void Initialize();
    bool LoadConfig(const std::string& path = "config/nights.json");
    bool LoadSave(const std::string& path = "save.dat");
    bool SaveGame(const std::string& path = "save.dat") const;

    void StartNight(int night);
    void Update(float dt);

    int GetCurrentNight() const { return m_currentNight; }
    int GetHighestUnlockedNight() const { return m_highestUnlockedNight; }
    void UnlockNextNight();

    int GetCurrentHour() const { return m_currentHour; }
    float GetNightTimer() const { return m_nightTimer; }
    float GetHourDuration() const { return m_hourDuration; }
    void SetHourDuration(float dur) { m_hourDuration = dur; }
    bool IsNightComplete() const { return m_currentHour >= 6; }

    const NightDifficulty& GetCurrentDifficulty() const;
    void SetCustomNightAI(int bear, int rabbit, int chicken, int fox);
    const NightDifficulty& GetCustomNightConfig() const { return m_customNightConfig; }

    // Phone call
    bool IsPhoneCallActive() const { return m_phoneCallActive; }
    void SkipPhoneCall();
};

} // namespace FNAF
