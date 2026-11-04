#pragma once

#include <string>

namespace FNAF {

class Settings {
public:
    float masterVolume = 1.0f;
    float sfxVolume = 1.0f;
    float musicVolume = 0.8f;
    bool fullscreen = false;
    bool crtEffects = true;
    bool rareVisitorEnabled = true;

    Settings();

    bool LoadFromFile(const std::string& filepath = "config/settings.json");
    bool SaveToFile(const std::string& filepath = "config/settings.json") const;
    void Apply();
};

} // namespace FNAF
