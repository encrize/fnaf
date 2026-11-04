#include "AssetManager.h"
#include <iostream>
#include <algorithm>
#include <cstdio>
#include <cstring>

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
        Image img = LoadImage(path.c_str());
        if (img.data != nullptr) {
            // Camera feeds: convert to 8-bit single-channel grayscale
            if (path.find("/cams/") != std::string::npos && path.find("map.png") == std::string::npos) {
                if (img.format != PIXELFORMAT_UNCOMPRESSED_GRAYSCALE) {
                    ImageFormat(&img, PIXELFORMAT_UNCOMPRESSED_GRAYSCALE);
                }
            } else if (img.format == PIXELFORMAT_UNCOMPRESSED_R8G8B8A8) {
                // Check if transparency is actually used
                bool hasAlpha = false;
                const unsigned char* pixels = static_cast<const unsigned char*>(img.data);
                size_t totalPixels = static_cast<size_t>(img.width * img.height);
                for (size_t i = 0; i < totalPixels; ++i) {
                    if (pixels[i * 4 + 3] < 255) {
                        hasAlpha = true;
                        break;
                    }
                }
                if (!hasAlpha) {
                    // No transparent pixels: store as R8G8B8 to save 25% VRAM
                    ImageFormat(&img, PIXELFORMAT_UNCOMPRESSED_R8G8B8);
                }
            }

            Texture2D tex = LoadTextureFromImage(img);
            SetTextureFilter(tex, TEXTURE_FILTER_BILINEAR);
            UnloadImage(img); // Requirement 5: Free CPU Image memory immediately!

            if (tex.id != 0) {
                m_textures[path] = tex;
                return tex;
            }
        }
    }

    // Fallback if file missing or failed to load
    CreateFallbackTexture();
    return m_fallbackTexture;
}

bool AssetManager::UnloadTextureByPath(const std::string& path) {
    auto it = m_textures.find(path);
    if (it != m_textures.end()) {
        if (it->second.id != 0) {
            UnloadTexture(it->second);
        }
        m_textures.erase(it);
        return true;
    }
    return false;
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

long AssetManager::GetCurrentVmRSSKb() {
    FILE* file = fopen("/proc/self/status", "r");
    if (!file) return 0;
    long rss = 0;
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "VmRSS:", 6) == 0) {
            sscanf(line + 6, "%ld", &rss);
            break;
        }
    }
    fclose(file);
    return rss;
}

size_t AssetManager::CalculateTextureBytes(int width, int height, int format) {
    if (width <= 0 || height <= 0) return 0;
    float bpp = 4.0f;
    switch (format) {
        case PIXELFORMAT_UNCOMPRESSED_GRAYSCALE: bpp = 1.0f; break;
        case PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA: bpp = 2.0f; break;
        case PIXELFORMAT_UNCOMPRESSED_R5G6B5: bpp = 2.0f; break;
        case PIXELFORMAT_UNCOMPRESSED_R8G8B8: bpp = 3.0f; break;
        case PIXELFORMAT_UNCOMPRESSED_R5G5B5A1: bpp = 2.0f; break;
        case PIXELFORMAT_UNCOMPRESSED_R4G4B4A4: bpp = 2.0f; break;
        case PIXELFORMAT_UNCOMPRESSED_R8G8B8A8: bpp = 4.0f; break;
        case PIXELFORMAT_COMPRESSED_DXT1_RGB:
        case PIXELFORMAT_COMPRESSED_DXT1_RGBA:
        case PIXELFORMAT_COMPRESSED_ETC1_RGB:
        case PIXELFORMAT_COMPRESSED_ETC2_RGB: bpp = 0.5f; break;
        case PIXELFORMAT_COMPRESSED_DXT3_RGBA:
        case PIXELFORMAT_COMPRESSED_DXT5_RGBA:
        case PIXELFORMAT_COMPRESSED_ETC2_EAC_RGBA: bpp = 1.0f; break;
        default: bpp = 4.0f; break;
    }
    return static_cast<size_t>(static_cast<float>(width * height) * bpp);
}

size_t AssetManager::GetTotalTextureBytes() const {
    size_t total = 0;
    for (const auto& pair : m_textures) {
        total += CalculateTextureBytes(pair.second.width, pair.second.height, pair.second.format);
    }
    return total;
}

std::vector<TextureMemInfo> AssetManager::GetLoadedTexturesSorted() const {
    std::vector<TextureMemInfo> list;
    list.reserve(m_textures.size());
    for (const auto& pair : m_textures) {
        TextureMemInfo info;
        info.path = pair.first;
        info.width = pair.second.width;
        info.height = pair.second.height;
        info.format = pair.second.format;
        info.bytes = CalculateTextureBytes(info.width, info.height, info.format);
        list.push_back(info);
    }
    std::sort(list.begin(), list.end(), [](const TextureMemInfo& a, const TextureMemInfo& b) {
        return a.bytes > b.bytes;
    });
    return list;
}

void AssetManager::LogMemoryReport(const std::string& sceneName) const {
    long rssKb = GetCurrentVmRSSKb();
    size_t totalTexBytes = GetTotalTextureBytes();
    auto sorted = GetLoadedTexturesSorted();

    std::cout << "\n=======================================================\n";
    std::cout << " [MEMORY REPORT] Scene: " << sceneName << "\n";
    std::cout << " VmRSS:               " << (rssKb / 1024.0) << " MB (" << rssKb << " kB)\n";
    std::cout << " Loaded Textures:     " << sorted.size() << "\n";
    std::cout << " Estimated Tex Memory:" << (totalTexBytes / (1024.0 * 1024.0)) << " MB (" << totalTexBytes << " bytes)\n";
    std::cout << " Top 10 Textures by Size:\n";
    for (size_t i = 0; i < std::min<size_t>(10, sorted.size()); ++i) {
        std::cout << "  " << (i + 1) << ". " << sorted[i].path 
                  << " [" << sorted[i].width << "x" << sorted[i].height << ", fmt " << sorted[i].format << "] "
                  << (sorted[i].bytes / 1024.0) << " kB\n";
    }
    std::cout << "=======================================================\n" << std::flush;
}

} // namespace FNAF
