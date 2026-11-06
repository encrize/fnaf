#pragma once

#include <raylib.h>
#include <vector>
#include <unordered_map>
#include "Common.h"

namespace FNAF {

class Audio;
class AssetManager;

struct CameraButton {
    CameraID id;
    Rectangle rect;     // Visual button rectangle (centered directly on camera room)
    Rectangle hitRect;  // Click and hover hitbox covering the room rectangle
    const char* label;
};

class Cameras {
private:
    CameraID m_currentCam = CameraID::CAM_1A;
    bool m_monitorUp = false;
    float m_monitorProgress = 0.0f; // 0.0 = down, 1.0 = up
    float m_staticTimer = 0.0f;
    float m_switchStaticTimer = 0.0f;
    float m_camSweepTimer = 0.0f;
    int m_glitchFrame = 0;

    std::vector<CameraButton> m_mapButtons;
    Rectangle m_monitorFlipBar{};
    bool m_mouseWasInFlipBar = false;
    float m_flipCooldown = 0.0f;

    // Map texture & Flip frames
    Texture2D m_mapTex{};
    Font m_font{};
    std::vector<Texture2D> m_flipFrames;
    std::vector<Texture2D> m_noiseTextures;

    // LRU Cache for room camera feeds (holds max 3 feeds in VRAM)
    struct CameraCacheEntry {
        std::string path;
        Texture2D texture;
        uint32_t lastAccess;
    };
    static constexpr size_t CAMERA_CACHE_CAPACITY = 3;
    mutable std::vector<CameraCacheEntry> m_camCache;
    mutable uint32_t m_cacheClock = 0;
    AssetManager* m_assets = nullptr;

    Texture2D GetCachedCameraTexture(const std::string& path) const;
    void ClearCameraCache();
    std::string DetermineCameraFeedPath(LocationID bearLoc, LocationID rabbitLoc, LocationID chickenLoc, FoxState foxState) const;

    void SetupMapButtons();

public:
    Cameras();
    ~Cameras();

    void LoadAssets(AssetManager& assets);
    void Reset();
    void Update(float dt, bool powerDepleted, Audio& audio);
    void HandleInput(Vector2 mousePos, bool powerDepleted, Audio& audio);
    void Render(float officeCameraX,
                LocationID bearLoc,
                LocationID rabbitLoc,
                LocationID chickenLoc,
                FoxState foxState,
                bool crtEnabled);

    bool IsMonitorUp() const { return m_monitorUp; }
    float GetMonitorProgress() const { return m_monitorProgress; }
    CameraID GetCurrentCamera() const { return m_currentCam; }
    const std::vector<CameraButton>& GetMapButtons() const { return m_mapButtons; }
    void SetCurrentCamera(CameraID cam, Audio& audio);
    void ToggleMonitor(Audio& audio);
    void ForceMonitorDown();
};

} // namespace FNAF
