#include "BundleManager.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <filesystem>
#include <raylib.h>

extern "C" {
    extern const uint8_t g_embeddedAssetsBundle[];
    extern const uint8_t g_embeddedAssetsBundleEnd[];
}

namespace fs = std::filesystem;

namespace FNAF {

static std::string g_initialExeDir;

void LogDebug(const std::string& msg) {
    std::cout << "[FNAF] " << msg << std::endl;

    if (g_initialExeDir.empty()) {
        const char* dir = GetApplicationDirectory();
        if (dir && *dir) {
            g_initialExeDir = dir;
        } else {
            g_initialExeDir = ".";
        }
    }

    auto writeToFile = [](const fs::path& filepath, const std::string& text) {
        std::error_code ec;
        std::ofstream logFile(filepath, std::ios::app);
        if (logFile.is_open()) {
            logFile << text << "\n";
            logFile.flush();
        }
    };

    writeToFile(fs::path(g_initialExeDir) / "fnaf_log.txt", msg);

#if defined(_WIN32)
    const char* tempDir = std::getenv("TEMP");
    if (tempDir && *tempDir) {
        writeToFile(fs::path(tempDir) / "fnaf_log.txt", msg);
    }
    const char* localApp = std::getenv("LOCALAPPDATA");
    if (localApp && *localApp) {
        writeToFile(fs::path(localApp) / "fnaf_log.txt", msg);
    }
#else
    writeToFile("/tmp/fnaf_log.txt", msg);
#endif
}

static bool HasValidAssets(const fs::path& baseDir) {
    std::error_code ec;
    bool hasConfig = fs::exists(baseDir / "config" / "settings.json", ec) && !ec;
    bool hasOffice = fs::exists(baseDir / "assets" / "images" / "office" / "office_main.png", ec) && !ec;
    return hasConfig && hasOffice;
}

static fs::path GetCacheDirectory() {
#if defined(_WIN32)
    const char* localApp = std::getenv("LOCALAPPDATA");
    if (localApp && *localApp) {
        return fs::path(localApp) / "FNAF_Game";
    }
    const char* temp = std::getenv("TEMP");
    if (temp && *temp) {
        return fs::path(temp) / "FNAF_Game";
    }
    return fs::path("C:\\Temp\\FNAF_Game");
#else
    const char* xdgCache = std::getenv("XDG_CACHE_HOME");
    if (xdgCache && *xdgCache) {
        return fs::path(xdgCache) / "fnaf_game";
    }
    const char* home = std::getenv("HOME");
    if (home && *home) {
        return fs::path(home) / ".cache" / "fnaf_game";
    }
    return fs::path("/tmp/fnaf_game");
#endif
}

bool EnsureAssetsReady() {
    try {
        LogDebug("EnsureAssetsReady: Checking for local assets...");

        // 1. Check current directory
        if (HasValidAssets(".")) {
            LogDebug("Found valid assets in current working directory.");
            return true;
        }

        // 2. Check application directory (folder containing the executable)
        const char* appDir = GetApplicationDirectory();
        if (appDir && *appDir) {
            fs::path appDirPath(appDir);
            if (HasValidAssets(appDirPath)) {
                LogDebug("Found valid assets in application directory: " + appDirPath.string());
                ChangeDirectory(appDir);
                return true;
            }
        }

        // 3. Standalone mode: running as single-file executable without external folders
        fs::path cacheDir = GetCacheDirectory();
        LogDebug("Standalone single-file mode. Target cache: " + cacheDir.string());

        fs::path markerPath = cacheDir / ".extracted_v3";
        std::error_code ec;

        if (fs::exists(markerPath, ec) && HasValidAssets(cacheDir)) {
            LogDebug("Cache already contains extracted assets. Switching directory.");
            ChangeDirectory(cacheDir.string().c_str());
            fs::current_path(cacheDir, ec);
            return true;
        }

        // 4. Extract embedded bundle into cache directory
        const size_t bundleSize = g_embeddedAssetsBundleEnd - g_embeddedAssetsBundle;
        LogDebug("Embedded bundle address: " + std::to_string(reinterpret_cast<uintptr_t>(g_embeddedAssetsBundle)) +
                 ", size: " + std::to_string(bundleSize) + " bytes");

        if (bundleSize < 12) {
            LogDebug("Error: Embedded assets bundle is too small or missing.");
            return false;
        }

        const uint8_t* data = g_embeddedAssetsBundle;
        if (data[0] != 'F' || data[1] != 'N' || data[2] != 'A' || data[3] != 'F') {
            LogDebug("Error: Invalid bundle signature: " + std::string(reinterpret_cast<const char*>(data), 4));
            return false;
        }

        size_t offset = 8;
        uint32_t fileCount = 0;
        std::memcpy(&fileCount, data + offset, sizeof(fileCount));
        offset += sizeof(fileCount);

        LogDebug("Extracting " + std::to_string(fileCount) + " embedded files...");

        fs::create_directories(cacheDir, ec);
        if (ec) {
            LogDebug("Error creating cache directory: " + ec.message());
            return false;
        }

        for (uint32_t i = 0; i < fileCount; ++i) {
            if (offset + sizeof(uint16_t) > bundleSize) break;
            uint16_t pathLen = 0;
            std::memcpy(&pathLen, data + offset, sizeof(pathLen));
            offset += sizeof(pathLen);

            if (offset + pathLen > bundleSize) break;
            std::string relPath(reinterpret_cast<const char*>(data + offset), pathLen);
            offset += pathLen;

            if (offset + sizeof(uint32_t) > bundleSize) break;
            uint32_t fileSize = 0;
            std::memcpy(&fileSize, data + offset, sizeof(fileSize));
            offset += sizeof(fileSize);

            if (offset + fileSize > bundleSize) break;

            // Normalize path separators and prevent directory traversal
            fs::path filePath = cacheDir;
            std::string component;
            for (char ch : relPath) {
                if (ch == '/' || ch == '\\') {
                    if (!component.empty()) {
                        if (component != ".." && component != ".") {
                            filePath /= component;
                        }
                        component.clear();
                    }
                } else {
                    component += ch;
                }
            }
            if (!component.empty() && component != ".." && component != ".") {
                filePath /= component;
            }

            fs::create_directories(filePath.parent_path(), ec);

            std::ofstream out(filePath, std::ios::binary);
            if (out.is_open()) {
                out.write(reinterpret_cast<const char*>(data + offset), fileSize);
            }
            offset += fileSize;
        }

        if (!HasValidAssets(cacheDir)) {
            LogDebug("Error: Extraction completed but assets validation failed in: " + cacheDir.string());
            return false;
        }

        // Write completion marker
        std::ofstream marker(markerPath);
        if (marker.is_open()) {
            marker << "1\n";
        }

        ChangeDirectory(cacheDir.string().c_str());
        fs::current_path(cacheDir, ec);
        LogDebug("Extraction complete! Current dir: " + cacheDir.string());
        return true;
    } catch (const std::exception& ex) {
        LogDebug(std::string("Exception in EnsureAssetsReady: ") + ex.what());
        return false;
    } catch (...) {
        LogDebug("Unknown exception in EnsureAssetsReady!");
        return false;
    }
}

} // namespace FNAF
