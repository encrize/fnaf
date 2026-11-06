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
    for (auto& tex : m_noiseTextures) {
        if (tex.id != 0) UnloadTexture(tex);
    }
    m_noiseTextures.clear();
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
    m_font = assets.GetFont("assets/fonts/Pixeboy.ttf", 26);
    m_mapTex = assets.GetTexture("assets/images/cams/map.png");

    m_flipFrames.clear();
    for (int i = 1; i <= 10; ++i) {
        m_flipFrames.push_back(assets.GetTexture("assets/images/flip/" + std::to_string(i) + ".png"));
    }

    // Procedural TV snow noise frames for cameras (silky 30-60 FPS CCTV grain, zero flat rectangles)
    m_noiseTextures.clear();
    for (int i = 0; i < 6; ++i) {
        Image img = GenImageWhiteNoise(480, 270, 0.45f);
        Texture2D tex = LoadTextureFromImage(img);
        SetTextureFilter(tex, TEXTURE_FILTER_BILINEAR);
        UnloadImage(img);
        m_noiseTextures.push_back(tex);
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
    constexpr float mx = 880.0f;
    constexpr float my = 320.0f;

    // Aligned directly with the camera room rectangles on map.png
    // 1A - Show Stage (map.png room: x=126..231, y=43..76)
    m_mapButtons.push_back({ CameraID::CAM_1A, Rectangle{ mx + 152.0f, my + 47.0f, 52.0f, 26.0f }, Rectangle{ mx + 126.0f, my + 43.0f, 105.0f, 34.0f }, "1A" });

    // 1B - Dining Area (upper-central dining hall below stage)
    m_mapButtons.push_back({ CameraID::CAM_1B, Rectangle{ mx + 152.0f, my + 101.0f, 52.0f, 28.0f }, Rectangle{ mx + 142.0f, my + 91.0f, 72.0f, 48.0f }, "1B" });

    // 5 - Backstage (map.png room: x=10..48, y=77..147)
    m_mapButtons.push_back({ CameraID::CAM_5,  Rectangle{ mx + 11.0f,  my + 98.0f,  36.0f, 28.0f }, Rectangle{ mx + 8.0f,   my + 76.0f, 44.0f, 72.0f }, "5" });

    // 1C - Pirate Cove (map.png room: x=23..61, y=162..210)
    m_mapButtons.push_back({ CameraID::CAM_1C, Rectangle{ mx + 24.0f,  my + 172.0f, 36.0f, 28.0f }, Rectangle{ mx + 20.0f,  my + 158.0f, 46.0f, 54.0f }, "1C" });

    // 3 - Supply Closet (map.png room: x=56..93, y=246..316)
    m_mapButtons.push_back({ CameraID::CAM_3,  Rectangle{ mx + 57.0f,  my + 267.0f, 36.0f, 28.0f }, Rectangle{ mx + 54.0f,  my + 244.0f, 44.0f, 74.0f }, "3" });

    // 2A - West Hall Upper (map.png corridor: x=105..147, y=229..290)
    m_mapButtons.push_back({ CameraID::CAM_2A, Rectangle{ mx + 106.0f, my + 244.0f, 40.0f, 28.0f }, Rectangle{ mx + 102.0f, my + 225.0f, 48.0f, 68.0f }, "2A" });

    // 2B - West Hall Lower / Corner (map.png corridor: x=105..147, y=295..363)
    m_mapButtons.push_back({ CameraID::CAM_2B, Rectangle{ mx + 106.0f, my + 314.0f, 40.0f, 28.0f }, Rectangle{ mx + 102.0f, my + 294.0f, 48.0f, 72.0f }, "2B" });

    // 4A - East Hall Upper (map.png corridor: x=210..251, y=229..290)
    m_mapButtons.push_back({ CameraID::CAM_4A, Rectangle{ mx + 211.0f, my + 244.0f, 40.0f, 28.0f }, Rectangle{ mx + 207.0f, my + 225.0f, 48.0f, 68.0f }, "4A" });

    // 4B - East Hall Lower / Corner (map.png corridor: x=210..251, y=295..363)
    m_mapButtons.push_back({ CameraID::CAM_4B, Rectangle{ mx + 211.0f, my + 314.0f, 40.0f, 28.0f }, Rectangle{ mx + 207.0f, my + 294.0f, 48.0f, 72.0f }, "4B" });

    // 6 - Kitchen (map.png room: x=265..347, y=229..298)
    m_mapButtons.push_back({ CameraID::CAM_6,  Rectangle{ mx + 280.0f, my + 250.0f, 52.0f, 28.0f }, Rectangle{ mx + 263.0f, my + 227.0f, 86.0f, 73.0f }, "6" });

    // 7 - Restrooms (map.png room: x=297..344, y=98..216)
    m_mapButtons.push_back({ CameraID::CAM_7,  Rectangle{ mx + 297.0f, my + 143.0f, 42.0f, 28.0f }, Rectangle{ mx + 295.0f, my + 96.0f,  88.0f, 122.0f }, "7" });

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
                // Responsive clicking on camera room rectangle or button area
                if (CheckCollisionPointRec(mousePos, btn.hitRect) || CheckCollisionPointRec(mousePos, btn.rect)) {
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
        Color barColor = hovering ? Color{ 140, 160, 190, 120 } : Color{ 40, 45, 55, 90 };
        DrawRectangleRec(m_monitorFlipBar, barColor);
        DrawRectangleLinesEx(m_monitorFlipBar, 2.0f, hovering ? WHITE : Color{ 160, 160, 170, 180 });

        // Sleek authentic camera chevron icon pointing up (▲)
        float cx = m_monitorFlipBar.x + m_monitorFlipBar.width * 0.5f;
        float cy = m_monitorFlipBar.y + m_monitorFlipBar.height * 0.5f;
        DrawTriangle(Vector2{ cx, cy - 8.0f },
                     Vector2{ cx - 14.0f, cy + 6.0f },
                     Vector2{ cx + 14.0f, cy + 6.0f },
                     hovering ? WHITE : Color{ 200, 200, 210, 200 });
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

    // 2. High-speed Animated TV Snow / CCTV Static Noise (Zero grey rectangles!)
    if (!m_noiseTextures.empty()) {
        int nIdx = static_cast<int>(m_staticTimer * 30.0f) % m_noiseTextures.size();
        Texture2D nTex = m_noiseTextures[nIdx];
        if (nTex.id != 0) {
            if (m_switchStaticTimer > 0.0f) {
                // Intense authentic static burst when switching cameras
                DrawTexturePro(nTex,
                               Rectangle{ 0, 0, static_cast<float>(nTex.width), static_cast<float>(nTex.height) },
                               Rectangle{ 0, 0, static_cast<float>(SCREEN_WIDTH), static_cast<float>(SCREEN_HEIGHT) },
                               Vector2{ 0, 0 }, 0.0f, ColorAlpha(WHITE, 0.75f));
            } else {
                // Subtle authentic CCTV static noise in additive mode
                BeginBlendMode(BLEND_ADDITIVE);
                DrawTexturePro(nTex,
                               Rectangle{ 0, 0, static_cast<float>(nTex.width), static_cast<float>(nTex.height) },
                               Rectangle{ 0, 0, static_cast<float>(SCREEN_WIDTH), static_cast<float>(SCREEN_HEIGHT) },
                               Vector2{ 0, 0 }, 0.0f, Color{ 255, 255, 255, 32 });
                EndBlendMode();
            }
        }
    }

    // 3. Crisp CRT Scanlines
    if (crtEnabled) {
        for (int y = 0; y < SCREEN_HEIGHT; y += 3) {
            DrawRectangle(0, y, SCREEN_WIDTH, 1, Color{ 0, 0, 0, 55 });
        }
    }

    // 4. Camera Name & REC indicator (Authentic top-left layout, no overlap with HUD)
    bool recBlink = (std::fmod(m_staticTimer, 1.0f) < 0.6f);
    if (recBlink) {
        DrawCircle(58, 48, 7, RED);
        if (m_font.texture.id != 0) {
            DrawTextEx(m_font, "REC", Vector2{ 72.0f, 37.0f }, 24.0f, 1.5f, RED);
        } else {
            DrawText("REC", 72, 37, 20, RED);
        }
    }

    if (m_font.texture.id != 0) {
        DrawTextEx(m_font, GetCameraName(m_currentCam), Vector2{ 50.0f, 72.0f }, 28.0f, 1.5f, Color{ 235, 235, 235, 255 });
    } else {
        DrawText(GetCameraName(m_currentCam), 50, 72, 24, Color{ 220, 220, 220, 255 });
    }

    // 5. Interactive Surveillance Minimap in Bottom-Right
    constexpr float mapOriginX = 880.0f;
    constexpr float mapOriginY = 320.0f;

    if (m_mapTex.id != 0) {
        DrawTexture(m_mapTex, static_cast<int>(mapOriginX), static_cast<int>(mapOriginY), WHITE);
    } else {
        DrawRectangle(static_cast<int>(mapOriginX), static_cast<int>(mapOriginY), 400, 400, Color{ 10, 12, 16, 220 });
        DrawRectangleLinesEx(Rectangle{ mapOriginX, mapOriginY, 400.0f, 400.0f }, 2, Color{ 50, 60, 75, 255 });
    }

    Vector2 mousePos = GetVirtualMousePosition();
    for (const auto& btn : m_mapButtons) {
        bool isCurrent = (btn.id == m_currentCam);
        bool hovering = CheckCollisionPointRec(mousePos, btn.hitRect) || CheckCollisionPointRec(mousePos, btn.rect);

        Color btnBg = isCurrent ? Color{ 50, 160, 70, 255 } : (hovering ? Color{ 80, 90, 110, 255 } : Color{ 35, 40, 50, 255 });
        DrawRectangleRec(btn.rect, btnBg);
        DrawRectangleLinesEx(btn.rect, isCurrent ? 2 : 1, isCurrent ? GREEN : GRAY);

        Color textCol = isCurrent ? WHITE : (hovering ? YELLOW : LIGHTGRAY);
        if (m_font.texture.id != 0) {
            Vector2 sz = MeasureTextEx(m_font, btn.label, 18.0f, 1.0f);
            DrawTextEx(m_font, btn.label,
                       Vector2{ btn.rect.x + (btn.rect.width - sz.x) * 0.5f,
                                btn.rect.y + (btn.rect.height - sz.y) * 0.5f },
                       18.0f, 1.0f, textCol);
        } else {
            int textW = MeasureText(btn.label, 16);
            DrawText(btn.label,
                     static_cast<int>(btn.rect.x + (btn.rect.width - static_cast<float>(textW)) * 0.5f),
                     static_cast<int>(btn.rect.y + (btn.rect.height - 16.0f) * 0.5f),
                     16,
                     textCol);
        }
    }

    // Bottom close bar (authentic FNAF 1 minimal flip chevron)
    bool hoveringClose = CheckCollisionPointRec(mousePos, m_monitorFlipBar);
    Color barColor = hoveringClose ? Color{ 140, 160, 190, 120 } : Color{ 40, 45, 55, 90 };
    DrawRectangleRec(m_monitorFlipBar, barColor);
    DrawRectangleLinesEx(m_monitorFlipBar, 2.0f, hoveringClose ? WHITE : Color{ 160, 160, 170, 180 });

    float cx = m_monitorFlipBar.x + m_monitorFlipBar.width * 0.5f;
    float cy = m_monitorFlipBar.y + m_monitorFlipBar.height * 0.5f;
    DrawTriangle(Vector2{ cx - 14.0f, cy - 6.0f },
                 Vector2{ cx, cy + 8.0f },
                 Vector2{ cx + 14.0f, cy - 6.0f },
                 hoveringClose ? WHITE : Color{ 200, 200, 210, 200 });
}

} // namespace FNAF
