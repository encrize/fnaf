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
    Rectangle rect;
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

    // Map texture & Flip frames
    Texture2D m_mapTex{};
    std::vector<Texture2D> m_flipFrames;
    std::vector<Texture2D> m_glitchFrames;

    // Room Textures
    Texture2D m_stageAll{};
    Texture2D m_stageBonnieGone{};
    Texture2D m_stageChicaGone{};
    Texture2D m_stageEmpty{};

    Texture2D m_diningEmpty{};
    Texture2D m_diningBonnie{};
    Texture2D m_diningChica{};
    Texture2D m_diningFreddy{};

    Texture2D m_coveClosed{};
    Texture2D m_covePeeking{};
    Texture2D m_coveOpen{};

    Texture2D m_westHallEmpty{};
    Texture2D m_westHallBonnie{};
    Texture2D m_westCornerEmpty{};
    Texture2D m_westCornerBonnie{};

    Texture2D m_closetEmpty{};
    Texture2D m_closetBonnie{};

    Texture2D m_eastHallEmpty{};
    Texture2D m_eastHallChica{};
    Texture2D m_eastCornerEmpty{};
    Texture2D m_eastCornerChica{};
    Texture2D m_eastCornerFreddy{};

    Texture2D m_backstageEmpty{};
    Texture2D m_backstageBonnie{};

    Texture2D m_restroomEmpty{};
    Texture2D m_restroomChica{};
    Texture2D m_restroomFreddy{};

    Texture2D m_kitchenTex{};

    void SetupMapButtons();

public:
    Cameras();

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
    void SetCurrentCamera(CameraID cam, Audio& audio);
    void ToggleMonitor(Audio& audio);
    void ForceMonitorDown();
};

} // namespace FNAF
