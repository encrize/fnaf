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
    m_phoneCallCompleted = false;
    m_phoneCallActive = true;
    m_phoneTextTimer = 0.0f;
    m_phoneTextCharCount = 0;

    SetupPhoneCallText(m_currentNight);
}

void NightManager::SetupPhoneCallText(int night) {
    switch (night) {
        case 1:
            m_phoneCallText = "Hello? Hello! Welcome to your new night shift. Monitor the security cameras, watch the door blindspots, and conserve electricity. The characters wander to keep their joints from seizing. Check on them regularly and you'll do fine.";
            break;
        case 2:
            m_phoneCallText = "Uh, hello again! Great job on Night 1. The Rabbit and Chicken will be more active tonight. Remember: check the doorway lights before leaving doors open. And keep checking Pirate Cove... Fox hates being left unwatched.";
            break;
        case 3:
            m_phoneCallText = "Hey there! Night 3 already. Watch out for the Bear tonight. He moves in the dark and sneaks up the East Hall when you're not checking. Keep your wits about you and watch that power gauge.";
            break;
        case 4:
            m_phoneCallText = "Hello? You're doing surprisingly well! Power reserves are strained tonight. Conserve usage, double-check your hall corners, and stay calm. Listen closely for footsteps in the corridors.";
            break;
        case 5:
            m_phoneCallText = "Uh, hello? Final shift. Everything is active tonight. Manage your door power, track the halls, and survive until 6 AM! Good luck.";
            break;
        default:
            m_phoneCallText = "Custom Night: Maximum security protocol engaged. Good luck, guard.";
            break;
    }
}

void NightManager::Update(float dt) {
    // Clock progression
    m_nightTimer += dt;
    m_currentHour = static_cast<int>(m_nightTimer / m_hourDuration);
    if (m_currentHour > 6) m_currentHour = 6;

    // Typewriter effect for phone call
    if (m_phoneCallActive && !m_phoneCallCompleted) {
        m_phoneTextTimer += dt;
        m_phoneTextCharCount = static_cast<size_t>(m_phoneTextTimer * 28.0f); // 28 chars/sec
        if (m_phoneTextCharCount >= m_phoneCallText.length()) {
            m_phoneTextCharCount = m_phoneCallText.length();
            // Linger for 6 seconds after completion then fade out
            if (m_phoneTextTimer > (m_phoneCallText.length() / 28.0f) + 6.0f) {
                m_phoneCallCompleted = true;
                m_phoneCallActive = false;
            }
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

std::string NightManager::GetCurrentPhoneSubtitles() const {
    if (!m_phoneCallActive || m_phoneCallCompleted) return "";
    return m_phoneCallText.substr(0, m_phoneTextCharCount);
}

void NightManager::SkipPhoneCall() {
    m_phoneCallCompleted = true;
    m_phoneCallActive = false;
}

} // namespace FNAF
