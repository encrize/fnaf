#include "NightManager.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

namespace FNAF {

NightManager::NightManager() {
    SetupDefaultDifficulties();
    LoadConfig();
    LoadSave();
}

void NightManager::Initialize() {
    StartNight(m_currentNight);
}

void NightManager::SetupDefaultDifficulties() {
    m_nightConfigs = {
        { 1, 0, 3, 3, 0 },
        { 2, 5, 6, 6, 0 },
        { 3, 10, 11, 11, 8 },
        { 4, 15, 15, 15, 14 },
        { 5, 16, 16, 16, 16 }
    };
}

bool NightManager::LoadConfig(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cout << "[NightManager] " << path << " not found, using default balances.\n";
        return false;
    }

    std::string line;
    int currentParseNight = 0;
    NightDifficulty diff;

    while (std::getline(file, line)) {
        if (line.find("\"hour_duration_seconds\"") != std::string::npos) {
            size_t c = line.find(':');
            if (c != std::string::npos) {
                m_hourDuration = std::stof(line.substr(c + 1));
            }
        } else if (line.find("\"night\"") != std::string::npos) {
            size_t c = line.find(':');
            if (c != std::string::npos) {
                currentParseNight = std::stoi(line.substr(c + 1));
                diff.night = currentParseNight;
            }
        } else if (line.find("\"bear_ai\"") != std::string::npos) {
            size_t c = line.find(':');
            if (c != std::string::npos) diff.bearAI = std::stoi(line.substr(c + 1));
        } else if (line.find("\"rabbit_ai\"") != std::string::npos) {
            size_t c = line.find(':');
            if (c != std::string::npos) diff.rabbitAI = std::stoi(line.substr(c + 1));
        } else if (line.find("\"chicken_ai\"") != std::string::npos) {
            size_t c = line.find(':');
            if (c != std::string::npos) diff.chickenAI = std::stoi(line.substr(c + 1));
        } else if (line.find("\"fox_ai\"") != std::string::npos) {
            size_t c = line.find(':');
            if (c != std::string::npos) {
                diff.foxAI = std::stoi(line.substr(c + 1));
                if (diff.night >= 1 && diff.night <= 5) {
                    m_nightConfigs[diff.night - 1] = diff;
                }
            }
        }
    }
    return true;
}

bool NightManager::LoadSave(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        m_highestUnlockedNight = 1;
        return false;
    }

    int savedNight = 1;
    if (file >> savedNight) {
        m_highestUnlockedNight = std::clamp(savedNight, 1, 5);
        m_currentNight = m_highestUnlockedNight;
        return true;
    }
    return false;
}

bool NightManager::SaveGame(const std::string& path) const {
    std::ofstream file(path);
    if (!file.is_open()) return false;
    file << m_highestUnlockedNight << "\n";
    return true;
}

void NightManager::StartNight(int night) {
    m_currentNight = std::clamp(night, 1, 7);
    m_nightTimer = 0.0f;
    m_currentHour = 0; // 12 AM
    m_phoneCallActive = (m_currentNight <= 5);
    m_phoneCallTimer = 0.0f;
    const float callDurations[] = { 85.0f, 45.0f, 35.0f, 30.0f, 25.0f };
    m_phoneCallDuration = (m_currentNight >= 1 && m_currentNight <= 5) ? callDurations[m_currentNight - 1] : 0.0f;
}

void NightManager::Update(float dt) {
    // Clock progression
    m_nightTimer += dt;
    m_currentHour = static_cast<int>(m_nightTimer / m_hourDuration);
    if (m_currentHour > 6) m_currentHour = 6;

    if (m_phoneCallActive) {
        m_phoneCallTimer += dt;
        if (m_phoneCallTimer >= m_phoneCallDuration) {
            m_phoneCallActive = false;
        }
    }
}

const NightDifficulty& NightManager::GetCurrentDifficulty() const {
    if (m_currentNight >= 1 && m_currentNight <= 5) {
        return m_nightConfigs[m_currentNight - 1];
    }
    return m_customNightConfig;
}

void NightManager::SetCustomNightAI(int bear, int rabbit, int chicken, int fox) {
    m_customNightConfig.night = 7;
    m_customNightConfig.bearAI = std::clamp(bear, 0, 20);
    m_customNightConfig.rabbitAI = std::clamp(rabbit, 0, 20);
    m_customNightConfig.chickenAI = std::clamp(chicken, 0, 20);
    m_customNightConfig.foxAI = std::clamp(fox, 0, 20);
}

void NightManager::UnlockNextNight() {
    if (m_currentNight >= m_highestUnlockedNight && m_highestUnlockedNight < 5) {
        m_highestUnlockedNight++;
        SaveGame();
    }
}

void NightManager::SkipPhoneCall() {
    m_phoneCallActive = false;
}

} // namespace FNAF
