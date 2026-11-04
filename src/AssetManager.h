#pragma once

#include <raylib.h>
#include <string>
#include <unordered_map>
#include <memory>

namespace FNAF {

class AssetManager {
private:
    std::unordered_map<std::string, Texture2D> m_textures;
    std::unordered_map<std::string, Font> m_fonts;
    Texture2D m_fallbackTexture{};
    bool m_hasFallbackTexture = false;

    void CreateFallbackTexture();

public:
    AssetManager();
    ~AssetManager();

    // Prevent copies
    AssetManager(const AssetManager&) = delete;
    AssetManager& operator=(const AssetManager&) = delete;

    void Initialize();
    void Cleanup();

    Texture2D GetTexture(const std::string& path);
    Font GetFont(const std::string& path, int fontSize = 32);
};

} // namespace FNAF
