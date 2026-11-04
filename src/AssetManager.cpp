#include "AssetManager.h"
#include <iostream>

namespace FNAF {

AssetManager::AssetManager() = default;

AssetManager::~AssetManager() {
    Cleanup();
}

void AssetManager::Initialize() {
    CreateFallbackTexture();
}

void AssetManager::CreateFallbackTexture() {
    if (m_hasFallbackTexture) return;

    Image img = GenImageChecked(64, 64, 8, 8, Color{40, 40, 40, 255}, Color{20, 20, 20, 255});
    m_fallbackTexture = LoadTextureFromImage(img);
    UnloadImage(img);
    m_hasFallbackTexture = true;
}

Texture2D AssetManager::GetTexture(const std::string& path) {
    auto it = m_textures.find(path);
    if (it != m_textures.end()) {
        return it->second;
    }

    if (FileExists(path.c_str())) {
        Texture2D tex = LoadTexture(path.c_str());
        if (tex.id != 0) {
            m_textures[path] = tex;
            return tex;
        }
    }

    // Fallback if file missing or failed to load
    CreateFallbackTexture();
    return m_fallbackTexture;
}

Font AssetManager::GetFont(const std::string& path, int fontSize) {
    std::string key = path + "_" + std::to_string(fontSize);
    auto it = m_fonts.find(key);
    if (it != m_fonts.end()) {
        return it->second;
    }

    if (FileExists(path.c_str())) {
        Font font = LoadFontEx(path.c_str(), fontSize, nullptr, 0);
        if (font.texture.id != 0) {
            SetTextureFilter(font.texture, TEXTURE_FILTER_BILINEAR);
            m_fonts[key] = font;
            return font;
        }
    }

    return GetFontDefault();
}

Texture2D AssetManager::CreateProceduralNoiseTexture(int width, int height) {
    Image img = GenImageWhiteNoise(width, height, 0.5f);
    Texture2D tex = LoadTextureFromImage(img);
    UnloadImage(img);
    return tex;
}

void AssetManager::Cleanup() {
    for (auto& pair : m_textures) {
        if (pair.second.id != 0) {
            UnloadTexture(pair.second);
        }
    }
    m_textures.clear();

    for (auto& pair : m_fonts) {
        if (pair.second.texture.id != 0 && pair.second.glyphs != nullptr) {
            UnloadFont(pair.second);
        }
    }
    m_fonts.clear();

    if (m_hasFallbackTexture && m_fallbackTexture.id != 0) {
        UnloadTexture(m_fallbackTexture);
        m_fallbackTexture = Texture2D{};
        m_hasFallbackTexture = false;
    }
}

} // namespace FNAF
