#pragma once

#include <raylib.h>
#include <vector>
#include "Common.h"

namespace FNAF {

class Audio;

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

    std::vector<CameraButton> m_mapButtons;
    Rectangle m_monitorFlipBar{};

    void SetupMapButtons();

public:
    Cameras();

    void Reset();
    void Update(float dt, bool powerDepleted, Audio& audio);
    void HandleInput(bool powerDepleted, Audio& audio);
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
