#pragma once

#include <raylib.h>
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>

namespace FNAF {

struct TextureMemInfo {
    std::string path;
    int width;
    int height;
    int format;
    size_t bytes;
};

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
    bool UnloadTextureByPath(const std::string& path);
    Font GetFont(const std::string& path, int fontSize = 32);

    // Memory tracking and diagnostics
    static long GetCurrentVmRSSKb();
    static size_t CalculateTextureBytes(int width, int height, int format);
    size_t GetTotalTextureBytes() const;
    size_t GetLoadedTextureCount() const { return m_textures.size(); }
    std::vector<TextureMemInfo> GetLoadedTexturesSorted() const;
    void LogMemoryReport(const std::string& sceneName) const;
};

} // namespace FNAF
