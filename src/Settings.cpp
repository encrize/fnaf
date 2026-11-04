#include "Settings.h"
#include <raylib.h>
#include <fstream>
#include <sstream>
#include <iostream>

namespace FNAF {

Settings::Settings() {
    LoadFromFile();
}

bool Settings::LoadFromFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "[Settings] Could not open " << filepath << ", using defaults.\n";
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.find("\"master_volume\"") != std::string::npos) {
            size_t colon = line.find(':');
            if (colon != std::string::npos) {
                masterVolume = std::stof(line.substr(colon + 1));
            }
        } else if (line.find("\"sfx_volume\"") != std::string::npos) {
            size_t colon = line.find(':');
            if (colon != std::string::npos) {
                sfxVolume = std::stof(line.substr(colon + 1));
            }
        } else if (line.find("\"music_volume\"") != std::string::npos) {
            size_t colon = line.find(':');
            if (colon != std::string::npos) {
                musicVolume = std::stof(line.substr(colon + 1));
            }
        } else if (line.find("\"fullscreen\"") != std::string::npos) {
            fullscreen = (line.find("true") != std::string::npos);
        } else if (line.find("\"crt_effects\"") != std::string::npos) {
            crtEffects = (line.find("true") != std::string::npos);
        } else if (line.find("\"rare_visitor_enabled\"") != std::string::npos) {
            rareVisitorEnabled = (line.find("true") != std::string::npos);
        }
    }
    return true;
}

bool Settings::SaveToFile(const std::string& filepath) const {
    std::ofstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "[Settings] Could not write to " << filepath << "\n";
        return false;
    }

    file << "{\n";
    file << "  \"master_volume\": " << masterVolume << ",\n";
    file << "  \"sfx_volume\": " << sfxVolume << ",\n";
    file << "  \"music_volume\": " << musicVolume << ",\n";
    file << "  \"fullscreen\": " << (fullscreen ? "true" : "false") << ",\n";
    file << "  \"crt_effects\": " << (crtEffects ? "true" : "false") << ",\n";
    file << "  \"rare_visitor_enabled\": " << (rareVisitorEnabled ? "true" : "false") << "\n";
    file << "}\n";

    return true;
}

void Settings::Apply() {
    SetMasterVolume(masterVolume);
    if (fullscreen != IsWindowFullscreen()) {
        ToggleFullscreen();
    }
}

} // namespace FNAF
