#include "Cameras.h"
#include "Audio.h"
#include "AssetManager.h"
#include <raylib.h>
#include <cmath>
#include <algorithm>
#include <iostream>

namespace FNAF {

Cameras::Cameras() {
    SetupMapButtons();
    Reset();
}

Cameras::~Cameras() {
    ClearCameraCache();
}

void Cameras::ClearCameraCache() {
    for (auto& entry : m_camCache) {
        if (m_assets) {
            m_assets->UnloadTextureByPath(entry.path);
        } else if (entry.texture.id != 0) {
            UnloadTexture(entry.texture);
        }
    }
    m_camCache.clear();
}

Texture2D Cameras::GetCachedCameraTexture(const std::string& path) const {
    if (path.empty()) return Texture2D{};

    m_cacheClock++;
    for (auto& entry : m_camCache) {
        if (entry.path == path) {
            entry.lastAccess = m_cacheClock;
            return entry.texture;
        }
    }

    if (!FileExists(path.c_str())) {
        return Texture2D{};
    }

    // If cache reached capacity, evict the least recently used texture
    if (m_camCache.size() >= CAMERA_CACHE_CAPACITY) {
        size_t lruIdx = 0;
        uint32_t oldest = m_camCache[0].lastAccess;
        for (size_t i = 1; i < m_camCache.size(); ++i) {
            if (m_camCache[i].lastAccess < oldest) {
                oldest = m_camCache[i].lastAccess;
                lruIdx = i;
            }
        }
        if (m_assets) {
            m_assets->UnloadTextureByPath(m_camCache[lruIdx].path);
        } else if (m_camCache[lruIdx].texture.id != 0) {
            UnloadTexture(m_camCache[lruIdx].texture);
        }
        m_camCache.erase(m_camCache.begin() + lruIdx);
    }

    Texture2D tex{};
    if (m_assets) {
        tex = m_assets->GetTexture(path);
    } else {
        Image img = LoadImage(path.c_str());
        if (img.data == nullptr) return Texture2D{};
        if (img.format != PIXELFORMAT_UNCOMPRESSED_GRAYSCALE) {
            ImageFormat(&img, PIXELFORMAT_UNCOMPRESSED_GRAYSCALE);
        }
        tex = LoadTextureFromImage(img);
        SetTextureFilter(tex, TEXTURE_FILTER_BILINEAR);
        UnloadImage(img);
    }

    m_camCache.push_back({ path, tex, m_cacheClock });
    return tex;
}

std::string Cameras::DetermineCameraFeedPath(LocationID bearLoc, LocationID rabbitLoc, LocationID chickenLoc, FoxState foxState) const {
    switch (m_currentCam) {
        case CameraID::CAM_1A: // Show Stage
            if (bearLoc != LocationID::ShowStage) return "assets/images/cams/Stage/484.png";
            if (chickenLoc != LocationID::ShowStage && rabbitLoc != LocationID::ShowStage) return "assets/images/cams/Stage/224.png";
            if (rabbitLoc != LocationID::ShowStage) return "assets/images/cams/Stage/68.png";
            return "assets/images/cams/Stage/19.png";

        case CameraID::CAM_1B: // Dining Area
            if (bearLoc == LocationID::DiningArea) return "assets/images/cams/Dining/215.png";
            if (chickenLoc == LocationID::DiningArea) return "assets/images/cams/Dining/120.png";
            if (rabbitLoc == LocationID::DiningArea) return "assets/images/cams/Dining/90.png";
            return "assets/images/cams/Dining/48.png";

        case CameraID::CAM_1C: // Pirate Cove
            if (foxState == FoxState::Hidden) return "assets/images/cams/Cove/66.png";
            if (foxState == FoxState::Watching) return "assets/images/cams/Cove/211.png";
            return "assets/images/cams/Cove/240.png";

        case CameraID::CAM_2A: // West Hall
            if (rabbitLoc == LocationID::WestHall) return "assets/images/cams/WestHall/206.png";
            return "assets/images/cams/WestHall/336.png";

        case CameraID::CAM_2B: // West Hall Corner
            if (rabbitLoc == LocationID::WestHallCorner) return "assets/images/cams/WestHall/188.png";
            return "assets/images/cams/WestHall/549.png";

        case CameraID::CAM_3: // Supply Closet
            if (rabbitLoc == LocationID::SupplyCloset) return "assets/images/cams/Closet/190.png";
            return "assets/images/cams/Closet/62.png";

        case CameraID::CAM_4A: // East Hall
            if (chickenLoc == LocationID::EastHall) return "assets/images/cams/EastHall/221.png";
            return "assets/images/cams/EastHall/67.png";

        case CameraID::CAM_4B: // East Hall Corner
            if (bearLoc == LocationID::EastHallCorner) return "assets/images/cams/EastHall/486.png";
            if (chickenLoc == LocationID::EastHallCorner) return "assets/images/cams/EastHall/451.png";
            return "assets/images/cams/EastHall/571.png";

        case CameraID::CAM_5: // Backstage
            if (rabbitLoc == LocationID::Backstage) return "assets/images/cams/Backstage/205.png";
            return "assets/images/cams/Backstage/354.png";

        case CameraID::CAM_7: // Restroom
            if (bearLoc == LocationID::Restrooms) return "assets/images/cams/Restroom/219.png";
            if (chickenLoc == LocationID::Restrooms) return "assets/images/cams/Restroom/217.png";
            return "assets/images/cams/Restroom/41.png";

        default:
            return "";
    }
}

void Cameras::LoadAssets(AssetManager& assets) {
    m_assets = &assets;
    m_mapTex = assets.GetTexture("assets/images/cams/map.png");

    m_flipFrames.clear();
    for (int i = 1; i <= 10; ++i) {
        m_flipFrames.push_back(assets.GetTexture("assets/images/flip/" + std::to_string(i) + ".png"));
    }

    m_glitchFrames.clear();
    const char* glitches[] = { "430.png", "434.png", "435.png", "436.png", "437.png", "438.png", "439.png" };
    for (const char* g : glitches) {
        m_glitchFrames.push_back(assets.GetTexture(std::string("assets/images/cams/Glitch/") + g));
    }
}

void Cameras::Reset() {
    m_currentCam = CameraID::CAM_1A;
    m_monitorUp = false;
    m_monitorProgress = 0.0f;
    m_staticTimer = 0.0f;
    m_switchStaticTimer = 0.0f;
    m_glitchFrame = 0;
    m_mouseWasInFlipBar = false;
    m_flipCooldown = 0.0f;
}

void Cameras::SetupMapButtons() {
    m_mapButtons.clear();
    float bx = 900.0f;
    float by = 350.0f;

    m_mapButtons.push_back({ CameraID::CAM_1A, Rectangle{ bx + 110, by + 10,  55, 30 }, "1A" });
    m_mapButtons.push_back({ CameraID::CAM_1B, Rectangle{ bx + 110, by + 65,  55, 30 }, "1B" });
    m_mapButtons.push_back({ CameraID::CAM_5,  Rectangle{ bx + 15,  by + 70,  55, 30 }, "5"  });
    m_mapButtons.push_back({ CameraID::CAM_1C, Rectangle{ bx + 45,  by + 130, 55, 30 }, "1C" });
    m_mapButtons.push_back({ CameraID::CAM_7,  Rectangle{ bx + 275, by + 75,  55, 30 }, "7"  });
    m_mapButtons.push_back({ CameraID::CAM_6,  Rectangle{ bx + 275, by + 160, 55, 30 }, "6"  });
    m_mapButtons.push_back({ CameraID::CAM_2A, Rectangle{ bx + 80,  by + 200, 55, 30 }, "2A" });
    m_mapButtons.push_back({ CameraID::CAM_3,  Rectangle{ bx + 15,  by + 215, 55, 30 }, "3"  });
    m_mapButtons.push_back({ CameraID::CAM_2B, Rectangle{ bx + 80,  by + 250, 55, 30 }, "2B" });
    m_mapButtons.push_back({ CameraID::CAM_4A, Rectangle{ bx + 205, by + 200, 55, 30 }, "4A" });
    m_mapButtons.push_back({ CameraID::CAM_4B, Rectangle{ bx + 205, by + 250, 55, 30 }, "4B" });

    m_monitorFlipBar = Rectangle{ 340.0f, SCREEN_HEIGHT - 45.0f, 600.0f, 40.0f };
}

void Cameras::SetCurrentCamera(CameraID cam, Audio& audio) {
    if (m_currentCam != cam) {
        m_currentCam = cam;
        m_switchStaticTimer = 0.25f;
        audio.PlayCameraSwitch();
    }
}

void Cameras::ToggleMonitor(Audio& audio) {
    m_monitorUp = !m_monitorUp;
    m_switchStaticTimer = 0.15f;
    audio.PlayCameraFlip();
}

void Cameras::ForceMonitorDown() {
    m_monitorUp = false;
    m_monitorProgress = 0.0f;
}

void Cameras::HandleInput(Vector2 mousePos, bool powerDepleted, Audio& audio) {
    if (powerDepleted) {
        ForceMonitorDown();
        return;
    }

    if (IsKeyPressed(KEY_SPACE)) {
        ToggleMonitor(audio);
        m_flipCooldown = 0.3f;
    }

    bool inFlipBar = CheckCollisionPointRec(mousePos, m_monitorFlipBar);

    // Hover entry toggle (authentic FNAF 1): when mouse crosses into bottom bar with cooldown
    if (inFlipBar && !m_mouseWasInFlipBar && m_flipCooldown <= 0.0f) {
        ToggleMonitor(audio);
        m_flipCooldown = 0.35f;
    }

    // Click toggle on bottom bar
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && inFlipBar && m_flipCooldown <= 0.0f) {
        ToggleMonitor(audio);
        m_flipCooldown = 0.35f;
    }

    m_mouseWasInFlipBar = inFlipBar;

    // Camera switching via map buttons or number keys when monitor is up
    if (m_monitorUp && m_monitorProgress >= 0.9f) {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            for (const auto& btn : m_mapButtons) {
                // Generous hitbox (+4px padding around button) for crisp responsive clicking
                Rectangle hitRect{ btn.rect.x - 4.0f, btn.rect.y - 4.0f, btn.rect.width + 8.0f, btn.rect.height + 8.0f };
                if (CheckCollisionPointRec(mousePos, hitRect)) {
                    SetCurrentCamera(btn.id, audio);
                    break;
                }
            }
        }

        // Hotkeys (Top row numbers and numpad)
        if (IsKeyPressed(KEY_ONE)   || IsKeyPressed(KEY_KP_1)) SetCurrentCamera(CameraID::CAM_1A, audio);
        if (IsKeyPressed(KEY_TWO)   || IsKeyPressed(KEY_KP_2)) SetCurrentCamera(CameraID::CAM_1B, audio);
        if (IsKeyPressed(KEY_THREE) || IsKeyPressed(KEY_KP_3)) SetCurrentCamera(CameraID::CAM_1C, audio);
        if (IsKeyPressed(KEY_FOUR)  || IsKeyPressed(KEY_KP_4)) SetCurrentCamera(CameraID::CAM_2A, audio);
        if (IsKeyPressed(KEY_FIVE)  || IsKeyPressed(KEY_KP_5)) SetCurrentCamera(CameraID::CAM_2B, audio);
        if (IsKeyPressed(KEY_SIX)   || IsKeyPressed(KEY_KP_6)) SetCurrentCamera(CameraID::CAM_3,  audio);
        if (IsKeyPressed(KEY_SEVEN) || IsKeyPressed(KEY_KP_7)) SetCurrentCamera(CameraID::CAM_4A, audio);
        if (IsKeyPressed(KEY_EIGHT) || IsKeyPressed(KEY_KP_8)) SetCurrentCamera(CameraID::CAM_4B, audio);
        if (IsKeyPressed(KEY_NINE)  || IsKeyPressed(KEY_KP_9)) SetCurrentCamera(CameraID::CAM_5,  audio);
        if (IsKeyPressed(KEY_ZERO)  || IsKeyPressed(KEY_KP_0)) SetCurrentCamera(CameraID::CAM_6,  audio);
        if (IsKeyPressed(KEY_MINUS) || IsKeyPressed(KEY_EQUAL)) SetCurrentCamera(CameraID::CAM_7, audio);
    }
}

void Cameras::Update(float dt, bool powerDepleted, Audio& /*audio*/) {
    if (powerDepleted) {
        ForceMonitorDown();
        return;
    }

    if (m_flipCooldown > 0.0f) {
        m_flipCooldown -= dt;
    }

    const float flipSpeed = 4.5f; // Fast, responsive 0.22s flip identical to original FNAF 1
    if (m_monitorUp) {
        m_monitorProgress = std::min(1.0f, m_monitorProgress + dt * flipSpeed);
    } else {
        m_monitorProgress = std::max(0.0f, m_monitorProgress - dt * flipSpeed);
    }

    m_camSweepTimer += dt * 0.4f;
    m_staticTimer += dt;
    if (m_switchStaticTimer > 0.0f) {
        m_switchStaticTimer -= dt;
    }

    if (!m_glitchFrames.empty()) {
        m_glitchFrame = (static_cast<int>(m_staticTimer * 15.0f)) % m_glitchFrames.size();
    }
}

void Cameras::Render(float /*officeCameraX*/,
                     LocationID bearLoc,
                     LocationID rabbitLoc,
                     LocationID chickenLoc,
                     FoxState foxState,
                     bool crtEnabled) {
    if (m_monitorProgress <= 0.0f) {
        Vector2 mousePos = GetVirtualMousePosition();
        bool hovering = CheckCollisionPointRec(mousePos, m_monitorFlipBar);
        Color barColor = hovering ? Color{ 80, 160, 220, 160 } : Color{ 40, 40, 50, 140 };
        DrawRectangleRec(m_monitorFlipBar, barColor);
        DrawRectangleLinesEx(m_monitorFlipBar, 2, hovering ? SKYBLUE : GRAY);
        DrawText("[ SPACE / HOVER / CLICK ] TO OPEN CAMERA MONITOR", m_monitorFlipBar.x + 60, m_monitorFlipBar.y + 12, 18, WHITE);
        return;
    }

    // Monitor Flip Animation Frames if transitioning
    if (m_monitorProgress < 1.0f && !m_flipFrames.empty()) {
        int totalFrames = static_cast<int>(m_flipFrames.size());
        int frameIdx = std::clamp(static_cast<int>(m_monitorProgress * totalFrames), 0, totalFrames - 1);
        Texture2D flipTex = m_flipFrames[frameIdx];
        if (flipTex.id != 0) {
            Rectangle src{ 0, 0, (float)flipTex.width, (float)flipTex.height };
            Rectangle dst{ 0, 0, (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT };
            DrawTexturePro(flipTex, src, dst, Vector2{ 0, 0 }, 0.0f, WHITE);
            return;
        }
    }

    // When monitor is fully open, render active camera feed
    if (m_currentCam == CameraID::CAM_6) {
        // Kitchen: Audio Only
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, BLACK);
        DrawText("- CAMERA DISABLED -", SCREEN_WIDTH / 2 - 160, 280, 28, Color{ 200, 200, 60, 255 });
        DrawText("AUDIO ONLY", SCREEN_WIDTH / 2 - 80, 330, 22, Color{ 180, 180, 180, 255 });
        float pulse = std::sin(m_staticTimer * 8.0f) * 15.0f;
        DrawCircle(SCREEN_WIDTH / 2, 390, 25 + pulse, Color{ 100, 100, 120, 100 });
        DrawCircle(SCREEN_WIDTH / 2, 390, 12, Color{ 220, 220, 240, 255 });
    } else {
        std::string feedPath = DetermineCameraFeedPath(bearLoc, rabbitLoc, chickenLoc, foxState);
        Texture2D camBg = feedPath.empty() ? Texture2D{} : GetCachedCameraTexture(feedPath);

        if (camBg.id != 0) {
            float panT = (std::sin(m_camSweepTimer) + 1.0f) * 0.5f;
            float viewRatio = (camBg.width > SCREEN_WIDTH) ? (static_cast<float>(SCREEN_WIDTH) / static_cast<float>(camBg.width)) : 0.8f;
            float viewWidth = static_cast<float>(camBg.width) * viewRatio;
            float maxPan = std::max(0.0f, static_cast<float>(camBg.width) - viewWidth);
            float camPan = panT * maxPan;

            Rectangle src{ camPan, 0.0f, viewWidth, static_cast<float>(camBg.height) };
            Rectangle dst{ 0.0f, 0.0f, static_cast<float>(SCREEN_WIDTH), static_cast<float>(SCREEN_HEIGHT) };
            // Bilinear upscaled 8-bit grayscale with authentic cold surveillance tint
            DrawTexturePro(camBg, src, dst, Vector2{ 0, 0 }, 0.0f, Color{ 215, 230, 240, 255 });
        } else {
            DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Color{ 20, 22, 28, 255 });
        }
    }

    // 2. Glitch / Static Overlay
    if (!m_glitchFrames.empty()) {
        Texture2D gTex = m_glitchFrames[m_glitchFrame];
        if (gTex.id != 0) {
            float alpha = (m_switchStaticTimer > 0.0f) ? 0.85f : 0.25f;
            Rectangle src{ 0, 0, (float)gTex.width, (float)gTex.height };
            Rectangle dst{ 0, 0, (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT };
            DrawTexturePro(gTex, src, dst, Vector2{ 0, 0 }, 0.0f, ColorAlpha(WHITE, alpha));
        }
    }

    // 3. CRT Scanlines
    if (crtEnabled) {
        for (int y = 0; y < SCREEN_HEIGHT; y += 4) {
            DrawLine(0, y, SCREEN_WIDTH, y, Color{ 0, 0, 0, 45 });
        }
    }

    // 4. Camera Name & REC indicator
    DrawText(GetCameraName(m_currentCam), 60, 50, 24, Color{ 220, 220, 220, 255 });
    bool recBlink = (std::fmod(m_staticTimer, 1.0f) < 0.5f);
    if (recBlink) {
        DrawCircle(SCREEN_WIDTH - 120, 60, 10, RED);
        DrawText("REC", SCREEN_WIDTH - 100, 50, 22, RED);
    }

    // 5. Interactive Surveillance Minimap in Bottom-Right
    float mapX = 900.0f;
    float mapY = 340.0f;

    if (m_mapTex.id != 0) {
        DrawTexture(m_mapTex, mapX - 20, mapY - 20, WHITE);
    } else {
        DrawRectangle(mapX - 20, mapY - 20, 360, 360, Color{ 10, 12, 16, 220 });
        DrawRectangleLinesEx(Rectangle{ mapX - 20, mapY - 20, 360, 360 }, 2, Color{ 50, 60, 75, 255 });
    }

    Vector2 mousePos = GetVirtualMousePosition();
    for (const auto& btn : m_mapButtons) {
        bool isCurrent = (btn.id == m_currentCam);
        bool hovering = CheckCollisionPointRec(mousePos, btn.rect);

        Color btnBg = isCurrent ? Color{ 50, 160, 70, 255 } : (hovering ? Color{ 80, 90, 110, 255 } : Color{ 35, 40, 50, 255 });
        DrawRectangleRec(btn.rect, btnBg);
        DrawRectangleLinesEx(btn.rect, isCurrent ? 2 : 1, isCurrent ? GREEN : GRAY);

        Color textCol = isCurrent ? WHITE : (hovering ? YELLOW : LIGHTGRAY);
        DrawText(btn.label, btn.rect.x + 12, btn.rect.y + 7, 16, textCol);
    }

    // Bottom close bar
    bool hoveringClose = CheckCollisionPointRec(mousePos, m_monitorFlipBar);
    DrawRectangleRec(m_monitorFlipBar, hoveringClose ? Color{ 200, 70, 70, 160 } : Color{ 40, 40, 50, 140 });
    DrawRectangleLinesEx(m_monitorFlipBar, 2, hoveringClose ? RED : GRAY);
    DrawText("[ SPACE / HOVER / CLICK ] TO CLOSE CAMERA MONITOR", m_monitorFlipBar.x + 55, m_monitorFlipBar.y + 12, 18, WHITE);
}

} // namespace FNAF
