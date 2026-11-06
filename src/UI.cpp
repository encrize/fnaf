#include "UI.h"
#include "Audio.h"
#include "AssetManager.h"
#include "Settings.h"
#include "NightManager.h"
#include "Power.h"
#include <cmath>
#include <algorithm>

namespace FNAF {

UI::UI() {
    Reset();
}

void UI::DrawPixelText(const char* text, float x, float y, float size, Color color, float spacing) {
    if (m_gameFont.texture.id != 0) {
        DrawTextEx(m_gameFont, text, Vector2{ x, y }, size, spacing, color);
    } else {
        DrawText(text, static_cast<int>(x), static_cast<int>(y), static_cast<int>(size), color);
    }
}

Vector2 UI::MeasurePixelText(const char* text, float size, float spacing) {
    if (!text || text[0] == '\0') return Vector2{ 0.0f, 0.0f };
    if (m_gameFont.texture.id != 0) {
        return MeasureTextEx(m_gameFont, text, size, spacing);
    }
    return Vector2{ static_cast<float>(MeasureText(text, static_cast<int>(size))), size };
}

void UI::DrawCenteredPixelText(const char* text, const Rectangle& bounds, float size, Color color, float spacing) {
    Vector2 sz = MeasurePixelText(text, size, spacing);
    float tx = bounds.x + (bounds.width - sz.x) * 0.5f;
    float ty = bounds.y + (bounds.height - sz.y) * 0.5f;
    DrawPixelText(text, tx, ty, size, color, spacing);
}

void UI::LoadAssets(AssetManager& assets) {
    m_assets = &assets;
    m_bearPortrait    = assets.GetTexture("assets/images/custom_night/527.png");
    m_rabbitPortrait  = assets.GetTexture("assets/images/custom_night/528.png");
    m_chickenPortrait = assets.GetTexture("assets/images/custom_night/529.png");
    m_foxPortrait     = assets.GetTexture("assets/images/custom_night/536.png");

    m_newspaperTex    = assets.GetTexture("assets/images/newspaper.png");
    m_gameFont        = assets.GetFont("assets/fonts/Pixeboy.ttf", 36);

    // Menu Freddy Artwork
    m_menuFreddyTex = assets.GetTexture("assets/images/menu/freddy.png");

    // Procedural TV Snow Noise Textures (6 frames for fluid 30-60 FPS analog grain, zero flat rectangles)
    m_noiseTextures.clear();
    for (int i = 0; i < 6; ++i) {
        Image img = GenImageWhiteNoise(480, 270, 0.45f);
        Texture2D tex = LoadTextureFromImage(img);
        SetTextureFilter(tex, TEXTURE_FILTER_BILINEAR);
        UnloadImage(img);
        m_noiseTextures.push_back(tex);
    }

    // Usage Bars
    m_usageTex.clear();
    for (int i = 1; i <= 5; ++i) {
        std::string upath = "assets/images/usage/" + std::to_string(i) + ".png";
        m_usageTex.push_back(assets.GetTexture(upath));
    }
}

UI::~UI() {
    UnloadJumpscareFrames();
    for (auto& tex : m_noiseTextures) {
        if (tex.id != 0) UnloadTexture(tex);
    }
    m_noiseTextures.clear();
}

void UI::UnloadJumpscareFrames() {
    if (m_assets) {
        for (const auto& path : m_activeJumpscarePaths) {
            m_assets->UnloadTextureByPath(path);
        }
    } else {
        for (auto& tex : m_activeJumpscareFrames) {
            if (tex.id != 0) {
                UnloadTexture(tex);
            }
        }
    }
    m_activeJumpscareFrames.clear();
    m_activeJumpscarePaths.clear();
}

void UI::LoadJumpscareFrames(AnimatronicID attacker) {
    UnloadJumpscareFrames();
    m_activeJumpscareAttacker = attacker;

    std::vector<std::string> fileList;
    std::string folder;

    switch (attacker) {
        case AnimatronicID::Rabbit: {
            folder = "assets/images/jumpscares/Bonnie/";
            const char* bJumps[] = { "291.png", "293.png", "294.png", "295.png", "296.png", "297.png", "298.png", "299.png", "300.png", "301.png", "303.png" };
            for (const char* f : bJumps) fileList.push_back(folder + f);
            break;
        }
        case AnimatronicID::Chicken: {
            folder = "assets/images/jumpscares/Chica/";
            const char* cJumps[] = { "65.png", "69.png", "228.png", "229.png", "230.png", "231.png", "232.png", "233.png", "234.png", "235.png", "236.png", "237.png", "279.png", "281.png" };
            for (const char* f : cJumps) fileList.push_back(folder + f);
            break;
        }
        case AnimatronicID::Fox: {
            folder = "assets/images/jumpscares/Foxy/Jumpscare/";
            const char* fJumps[] = { "242.png", "243.png", "396.png", "397.png", "398.png", "399.png", "400.png", "401.png", "402.png", "403.png", "404.png", "405.png", "406.png", "407.png", "408.png", "409.png", "410.png", "411.png", "412.png" };
            for (const char* f : fJumps) fileList.push_back(folder + f);
            break;
        }
        case AnimatronicID::Bear:
        case AnimatronicID::RareVisitor:
        default: {
            folder = "assets/images/jumpscares/Freddy/1/";
            const char* frJumps[] = { "307.png", "308.png", "309.png", "310.png", "311.png", "312.png", "313.png", "314.png", "315.png", "316.png", "317.png", "318.png", "319.png", "320.png", "321.png", "322.png", "323.png", "324.png", "325.png" };
            for (const char* f : frJumps) fileList.push_back(folder + f);
            break;
        }
    }

    for (const auto& path : fileList) {
        if (FileExists(path.c_str())) {
            Texture2D tex{};
            if (m_assets) {
                tex = m_assets->GetTexture(path);
            } else {
                Image img = LoadImage(path.c_str());
                if (img.data != nullptr) {
                    if (img.format == PIXELFORMAT_UNCOMPRESSED_R8G8B8A8) {
                        ImageFormat(&img, PIXELFORMAT_UNCOMPRESSED_R8G8B8);
                    }
                    tex = LoadTextureFromImage(img);
                    SetTextureFilter(tex, TEXTURE_FILTER_BILINEAR);
                    UnloadImage(img);
                }
            }
            if (tex.id != 0) {
                m_activeJumpscareFrames.push_back(tex);
                m_activeJumpscarePaths.push_back(path);
            }
        }
    }
}

void UI::Reset() {
    m_menuTimer = 0.0f;
    m_jumpscareTimer = 0.0f;
    UnloadJumpscareFrames();
    m_sixAmTimer = 0.0f;
    m_customBear = 0;
    m_customRabbit = 0;
    m_customChicken = 0;
    m_customFox = 0;
}

void UI::StartJumpscare() {
    m_jumpscareTimer = 0.0f;
}

void UI::RenderMainMenu(int currentNight, bool /*customNightUnlocked*/, Settings& settings, Audio& audio) {
    float dt = GetFrameTime();
    m_menuTimer += dt;
    audio.PlayMenuMusic(true);

    ClearBackground(BLACK);

    // 1. Freddy Background & Authentic Twitch Animation
    m_menuTwitchTimer += dt;
    bool isTwitching = false;
    float jitterX = 0.0f;
    float jitterY = 0.0f;

    if (m_menuTwitchTimer >= m_menuNextTwitchTime) {
        if (m_menuTwitchTimer < m_menuNextTwitchTime + 0.14f) {
            isTwitching = true;
            jitterX = static_cast<float>(GetRandomValue(-12, 12));
            jitterY = static_cast<float>(GetRandomValue(-6, 6));
        } else {
            m_menuTwitchTimer = 0.0f;
            m_menuNextTwitchTime = 2.2f + (static_cast<float>(GetRandomValue(0, 320)) / 100.0f);
        }
    }

    if (m_menuFreddyTex.id != 0) {
        float fH = static_cast<float>(SCREEN_HEIGHT) + 20.0f;
        float fW = fH * (static_cast<float>(m_menuFreddyTex.width) / static_cast<float>(m_menuFreddyTex.height));
        float fX = (static_cast<float>(SCREEN_WIDTH) - fW + 40.0f) + jitterX;
        float fY = jitterY;

        Color fTint = isTwitching ? Color{ 255, 220, 220, 255 } : Color{ 200, 200, 205, 255 };
        DrawTexturePro(m_menuFreddyTex,
                       Rectangle{ 0, 0, static_cast<float>(m_menuFreddyTex.width), static_cast<float>(m_menuFreddyTex.height) },
                       Rectangle{ fX, fY, fW, fH },
                       Vector2{ 0, 0 }, 0.0f, fTint);
    }

    // 2. High-speed Animated TV Snow (Additive Mode - Authentic retro phosphor TV grain, zero flat grey boxes)
    m_noiseTimer += dt;
    if (m_noiseTimer >= 0.033f) {
        m_noiseTimer = 0.0f;
        if (!m_noiseTextures.empty()) {
            m_noiseFrame = (m_noiseFrame + 1) % m_noiseTextures.size();
        }
    }
    if (settings.crtEffects && !m_noiseTextures.empty()) {
        BeginBlendMode(BLEND_ADDITIVE);
        Texture2D nTex = m_noiseTextures[m_noiseFrame];
        unsigned char nAlpha = isTwitching ? 85 : 24;
        DrawTexturePro(nTex,
                       Rectangle{ 0, 0, static_cast<float>(nTex.width), static_cast<float>(nTex.height) },
                       Rectangle{ 0, 0, static_cast<float>(SCREEN_WIDTH), static_cast<float>(SCREEN_HEIGHT) },
                       Vector2{ 0, 0 }, 0.0f, Color{ 255, 255, 255, nAlpha });
        EndBlendMode();
    }

    // 3. Crisp Authentic CRT Phosphor Scanlines
    if (settings.crtEffects) {
        for (int y = 0; y < SCREEN_HEIGHT; y += 3) {
            DrawRectangle(0, y, SCREEN_WIDTH, 1, Color{ 0, 0, 0, 55 });
        }
    }

    // 4. Iconic Title: "Five Nights at Freddy's"
    float titleX = 80.0f;
    float titleY = 60.0f;
    DrawPixelText("Five", titleX, titleY, 50.0f, WHITE, 2.0f);
    DrawPixelText("Nights", titleX, titleY + 46.0f, 50.0f, WHITE, 2.0f);
    DrawPixelText("at", titleX, titleY + 92.0f, 50.0f, WHITE, 2.0f);
    DrawPixelText("Freddy's", titleX, titleY + 138.0f, 50.0f, WHITE, 2.0f);

    // 5. Menu Options with zero overlap and uniform authentic spacing
    Vector2 mousePos = GetVirtualMousePosition();
    constexpr float menuX = 80.0f;
    constexpr float menuW = 340.0f;
    constexpr float yNewGame   = 310.0f;
    constexpr float yContinue  = 358.0f;
    constexpr float yNight6    = 440.0f;
    constexpr float yCustom    = 490.0f;
    constexpr float ySettings  = 540.0f;
    constexpr float yExit      = 590.0f;

    struct MenuEntry {
        int id; // 0=NewGame, 1=Continue, 2=Night6, 3=CustomNight, 4=Settings, 5=Exit
        const char* label;
        Rectangle hitRect;
        float textY;
    };

    std::vector<MenuEntry> menuEntries = {
        { 0, "New Game",     Rectangle{ menuX, yNewGame - 5.0f,  menuW, 40.0f }, yNewGame },
        { 1, "Continue",     Rectangle{ menuX, yContinue - 5.0f, menuW, 72.0f }, yContinue },
        { 2, "6th Night",    Rectangle{ menuX, yNight6 - 5.0f,   menuW, 40.0f }, yNight6 },
        { 3, "Custom Night", Rectangle{ menuX, yCustom - 5.0f,   menuW, 40.0f }, yCustom },
        { 4, "Settings",     Rectangle{ menuX, ySettings - 5.0f, menuW, 40.0f }, ySettings },
        { 5, "Exit",         Rectangle{ menuX, yExit - 5.0f,     menuW, 40.0f }, yExit }
    };

    for (const auto& entry : menuEntries) {
        bool hover = CheckCollisionPointRec(mousePos, entry.hitRect);
        Color col = hover ? WHITE : Color{ 200, 200, 205, 230 };

        if (hover) {
            DrawPixelText(">>", menuX - 34.0f, entry.textY + 4.0f, 30.0f, WHITE, 1.0f);
        }
        DrawPixelText(entry.label, menuX, entry.textY + 4.0f, 30.0f, col, 1.5f);

        // Display current night cleanly indented underneath Continue - strictly non-overlapping
        if (entry.id == 1) {
            std::string nText = "Night " + std::to_string(currentNight);
            Color nCol = hover ? Color{ 240, 230, 110, 255 } : Color{ 150, 150, 165, 255 };
            DrawPixelText(nText.c_str(), menuX + 35.0f, entry.textY + 42.0f, 20.0f, nCol, 1.2f);
        }
    }

    DrawPixelText("v 1.13", 30.0f, SCREEN_HEIGHT - 35.0f, 20.0f, Color{ 120, 120, 130, 180 }, 1.0f);
}

bool UI::HandleMainMenuInput(GameState& state, NightManager& nightMgr, Settings& /*settings*/, Audio& audio, Vector2 mousePos) {
    constexpr float menuX = 80.0f;
    constexpr float menuW = 340.0f;
    constexpr float yNewGame   = 310.0f;
    constexpr float yContinue  = 358.0f;
    constexpr float yNight6    = 440.0f;
    constexpr float yCustom    = 490.0f;
    constexpr float ySettings  = 540.0f;
    constexpr float yExit      = 590.0f;

    Rectangle newGameRect{ menuX, yNewGame - 5.0f,  menuW, 40.0f };
    Rectangle continueRect{ menuX, yContinue - 5.0f, menuW, 72.0f };
    Rectangle night6Rect{ menuX, yNight6 - 5.0f,   menuW, 40.0f };
    Rectangle customRect{ menuX, yCustom - 5.0f,   menuW, 40.0f };
    Rectangle settingsRect{ menuX, ySettings - 5.0f, menuW, 40.0f };
    Rectangle exitRect{ menuX, yExit - 5.0f,     menuW, 40.0f };

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        if (CheckCollisionPointRec(mousePos, newGameRect)) {
            audio.PlayBlip();
            audio.PlayMenuMusic(false);
            nightMgr.StartNight(1);
            state = GameState::NightIntro;
            return true;
        }
        if (CheckCollisionPointRec(mousePos, continueRect)) {
            audio.PlayBlip();
            audio.PlayMenuMusic(false);
            nightMgr.StartNight(nightMgr.GetHighestUnlockedNight());
            state = GameState::NightIntro;
            return true;
        }
        if (CheckCollisionPointRec(mousePos, night6Rect)) {
            audio.PlayBlip();
            audio.PlayMenuMusic(false);
            nightMgr.StartNight(6);
            state = GameState::NightIntro;
            return true;
        }
        if (CheckCollisionPointRec(mousePos, customRect)) {
            audio.PlayBlip();
            state = GameState::CustomNightSelect;
            return true;
        }
        if (CheckCollisionPointRec(mousePos, settingsRect)) {
            audio.PlayBlip();
            state = GameState::SettingsMenu;
            return true;
        }
        if (CheckCollisionPointRec(mousePos, exitRect)) {
            audio.PlayBlip();
            return false;
        }
    }
    return true;
}

void UI::RenderHUD(const NightManager& nightMgr, const Power& power, bool /*monitorUp*/) {
    // Top Right: Time & Night (Authentic FNAF 1 pixel presentation, right-aligned with ample vertical gap)
    int hour = nightMgr.GetCurrentHour();
    std::string timeStr = (hour == 0) ? "12 AM" : (std::to_string(hour) + " AM");
    Vector2 timeSz = MeasurePixelText(timeStr.c_str(), 46.0f, 2.0f);
    DrawPixelText(timeStr.c_str(), SCREEN_WIDTH - 45.0f - timeSz.x, 22.0f, 46.0f, WHITE, 2.0f);

    std::string nightStr = "Night " + std::to_string(nightMgr.GetCurrentNight());
    Vector2 nightSz = MeasurePixelText(nightStr.c_str(), 24.0f, 1.5f);
    DrawPixelText(nightStr.c_str(), SCREEN_WIDTH - 45.0f - nightSz.x, 76.0f, 24.0f, Color{ 200, 200, 200, 255 }, 1.5f);

    // Bottom Left: Power & Usage (Authentic FNAF 1 pixel presentation, non-overlapping spacing)
    int pVal = static_cast<int>(power.GetPower());
    std::string powerStr = "Power left: " + std::to_string(pVal) + "%";
    Color pCol = (pVal > 20) ? WHITE : ((pVal > 10) ? YELLOW : RED);
    DrawPixelText(powerStr.c_str(), 45.0f, SCREEN_HEIGHT - 98.0f, 26.0f, pCol, 1.5f);

    DrawPixelText("Usage:", 45.0f, SCREEN_HEIGHT - 62.0f, 22.0f, WHITE, 1.5f);
    int usage = std::clamp(power.GetUsageLevel(), 1, 5);
    float usageBarX = 155.0f;
    float usageBarY = SCREEN_HEIGHT - 60.0f;
    constexpr float USAGE_SCALE = 1.0f;

    if (usage - 1 < static_cast<int>(m_usageTex.size()) && m_usageTex[usage - 1].id > 0) {
        Texture2D uTex = m_usageTex[usage - 1];

        DrawTexturePro(
            uTex,
            Rectangle{ 0, 0, static_cast<float>(uTex.width), static_cast<float>(uTex.height) },
            Rectangle{
                usageBarX,
                usageBarY - 2.0f,
                static_cast<float>(uTex.width) * USAGE_SCALE,
                static_cast<float>(uTex.height) * 0.75f
            },
            Vector2{ 0, 0 },
            0.0f,
            WHITE
        );
    } else {
        for (int i = 0; i < 5; ++i) {
            Color barCol = (i == 0) ? GREEN : ((i == 1) ? LIME : ((i == 2) ? YELLOW : ((i == 3) ? ORANGE : RED)));
            if (i < usage) {
                DrawRectangle(static_cast<int>(usageBarX) + i * 20, static_cast<int>(usageBarY), 15, 20, barCol);
            } else {
                DrawRectangleLines(static_cast<int>(usageBarX) + i * 20, static_cast<int>(usageBarY), 15, 20, Color{ 80, 80, 90, 160 });
            }
        }
    }
}

void UI::RenderPhoneMuteButton(Vector2 mousePos) {
    Rectangle muteBtn{ 40.0f, 40.0f, 140.0f, 34.0f };
    bool hover = CheckCollisionPointRec(mousePos, muteBtn);
    DrawRectangleRec(muteBtn, hover ? Color{ 170, 40, 40, 200 } : Color{ 50, 50, 60, 160 });
    DrawRectangleLinesEx(muteBtn, 1.5f, hover ? RED : Color{ 160, 160, 170, 200 });
    DrawCenteredPixelText("MUTE CALL", muteBtn, 20.0f, WHITE, 1.0f);
}

void UI::RenderJumpscare(AnimatronicID attacker, float dt, Audio& audio) {
    if (m_jumpscareTimer == 0.0f) {
        audio.PlayJumpscare();
    }
    m_jumpscareTimer += dt;

    if (m_activeJumpscareFrames.empty() || m_activeJumpscareAttacker != attacker) {
        LoadJumpscareFrames(attacker);
    }

    ClearBackground(BLACK);

    if (!m_activeJumpscareFrames.empty()) {
        int idx = std::clamp(static_cast<int>(m_jumpscareTimer * 22.0f), 0, static_cast<int>(m_activeJumpscareFrames.size() - 1));
        Texture2D fTex = m_activeJumpscareFrames[idx];
        if (fTex.id != 0) {
            // Camera shake
            float shakeX = (GetRandomValue(-15, 15));
            float shakeY = (GetRandomValue(-10, 10));
            Rectangle src{ 0, 0, (float)fTex.width, (float)fTex.height };
            Rectangle dst{ shakeX, shakeY, (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT };
            DrawTexturePro(fTex, src, dst, Vector2{ 0, 0 }, 0.0f, WHITE);
            return;
        }
    }

    // Fallback if texture empty
    float shakeX = (GetRandomValue(-25, 25));
    float shakeY = (GetRandomValue(-20, 20));
    DrawCircle(SCREEN_WIDTH * 0.5f + shakeX, SCREEN_HEIGHT * 0.55f + shakeY, 150, RED);
}

void UI::RenderNightIntro(int currentNight, float timer, AssetManager& assets) {
    ClearBackground(BLACK);

    if (currentNight == 1 && timer < 3.5f) {
        Texture2D news = assets.GetTexture("assets/images/newspaper.png");
        if (news.id > 0) {
            DrawTexturePro(news,
                           Rectangle{ 0, 0, static_cast<float>(news.width), static_cast<float>(news.height) },
                           Rectangle{ 0, 0, static_cast<float>(SCREEN_WIDTH), static_cast<float>(SCREEN_HEIGHT) },
                           Vector2{ 0, 0 }, 0.0f, WHITE);
            return;
        }
    }

    // Authentic FNAF 1 Night Intro
    bool blip = (std::fmod(timer, 0.8f) < 0.4f);
    std::string timeStr = "12:00 AM";
    Vector2 tSz = MeasurePixelText(timeStr.c_str(), 54.0f, 2.0f);
    float tX = (SCREEN_WIDTH - tSz.x) * 0.5f;
    float tY = SCREEN_HEIGHT * 0.5f - 55.0f;
    DrawPixelText(timeStr.c_str(), tX, tY, 54.0f, WHITE, 2.0f);
    if (blip) {
        DrawCircle(static_cast<int>(tX + tSz.x + 18.0f), static_cast<int>(tY + tSz.y * 0.5f), 6.0f, WHITE);
    }

    std::string nStr;
    switch (currentNight) {
        case 1: nStr = "1st Night"; break;
        case 2: nStr = "2nd Night"; break;
        case 3: nStr = "3rd Night"; break;
        case 4: nStr = "4th Night"; break;
        case 5: nStr = "5th Night"; break;
        case 6: nStr = "6th Night"; break;
        case 7: nStr = "Custom Night"; break;
        default: nStr = "Night " + std::to_string(currentNight); break;
    }
    Vector2 nSz = MeasurePixelText(nStr.c_str(), 34.0f, 1.5f);
    DrawPixelText(nStr.c_str(), (SCREEN_WIDTH - nSz.x) * 0.5f, SCREEN_HEIGHT * 0.5f + 30.0f, 34.0f, Color{ 210, 210, 220, 255 }, 1.5f);
}

void UI::RenderGameOver(float /*dt*/) {
    UnloadJumpscareFrames(); // Immediately release jumpscare frames from memory!
    ClearBackground(BLACK);
    for (int i = 0; i < 600; ++i) {
        int x = GetRandomValue(0, SCREEN_WIDTH);
        int y = GetRandomValue(0, SCREEN_HEIGHT);
        int v = GetRandomValue(30, 180);
        DrawPixel(x, y, Color{ static_cast<unsigned char>(v), static_cast<unsigned char>(v), static_cast<unsigned char>(v), 200 });
    }
    Vector2 goSz = MeasurePixelText("GAME OVER", 70.0f, 2.0f);
    DrawPixelText("GAME OVER", (SCREEN_WIDTH - goSz.x) * 0.5f, 270.0f, 70.0f, Color{ 210, 30, 30, 255 }, 2.0f);

    Vector2 r1Sz = MeasurePixelText("Press [ ENTER ] to Retry Night", 24.0f, 1.2f);
    DrawPixelText("Press [ ENTER ] to Retry Night", (SCREEN_WIDTH - r1Sz.x) * 0.5f, 400.0f, 24.0f, WHITE, 1.2f);

    Vector2 r2Sz = MeasurePixelText("Press [ ESC ] for Main Menu", 22.0f, 1.2f);
    DrawPixelText("Press [ ESC ] for Main Menu", (SCREEN_WIDTH - r2Sz.x) * 0.5f, 440.0f, 22.0f, LIGHTGRAY, 1.2f);
}

bool UI::HandleGameOverInput(GameState& state, NightManager& nightMgr, Audio& audio, Vector2 /*mousePos*/) {
    if (IsKeyPressed(KEY_ENTER)) {
        audio.PlayBlip();
        nightMgr.StartNight(nightMgr.GetCurrentNight());
        state = GameState::Playing;
        return true;
    }
    if (IsKeyPressed(KEY_ESCAPE)) {
        audio.PlayBlip();
        state = GameState::MainMenu;
        return true;
    }
    return true;
}

void UI::Render6AM(float dt, Audio& audio) {
    if (m_sixAmTimer == 0.0f) {
        audio.PlayChime6AM();
    }
    float prevTimer = m_sixAmTimer;
    m_sixAmTimer += dt;
    if (prevTimer < 2.0f && m_sixAmTimer >= 2.0f) {
        audio.PlayCheer6AM();
    }

    ClearBackground(Color{ 8, 8, 10, 255 });

    // Subtle CRT scanlines
    for (int y = 0; y < SCREEN_HEIGHT; y += 4) {
        DrawLine(0, y, SCREEN_WIDTH, y, Color{ 0, 0, 0, 50 });
    }

    if (m_sixAmTimer < 2.0f) {
        Vector2 sz5 = MeasurePixelText("5 AM", 80.0f, 2.5f);
        DrawPixelText("5 AM", (SCREEN_WIDTH - sz5.x) * 0.5f, 280.0f, 80.0f, Color{ 200, 200, 210, 255 }, 2.5f);
    } else {
        Vector2 sz6 = MeasurePixelText("6 AM", 84.0f, 2.5f);
        DrawPixelText("6 AM", (SCREEN_WIDTH - sz6.x) * 0.5f, 280.0f, 84.0f, Color{ 245, 235, 120, 255 }, 2.5f);
    }
}

bool UI::Handle6AMInput(GameState& state, NightManager& nightMgr, Audio& audio) {
    if (m_sixAmTimer > 4.5f || IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
        audio.PlayBlip();
        if (nightMgr.GetCurrentNight() >= 5) {
            state = GameState::Win;
        } else {
            nightMgr.UnlockNextNight();
            nightMgr.StartNight(nightMgr.GetCurrentNight() + 1);
            state = GameState::Playing;
        }
    }
    return true;
}

void UI::RenderWinScreen() {
    ClearBackground(Color{ 18, 20, 26, 255 });

    float px = 300.0f;
    float py = 120.0f;
    float pw = 680.0f;
    float ph = 420.0f;

    DrawRectangle(px, py, pw, ph, Color{ 235, 230, 210, 255 });
    DrawRectangleLinesEx(Rectangle{ px, py, pw, ph }, 6, Color{ 40, 60, 45, 255 });
    DrawRectangleLinesEx(Rectangle{ px + 12, py + 12, pw - 24, ph - 24 }, 2, Color{ 90, 110, 85, 255 });

    DrawText("FAZBEAR ENTERTAINMENT", px + 160, py + 35, 26, Color{ 40, 40, 45, 255 });
    DrawText("EMPLOYEE PAYCHECK", px + 220, py + 75, 20, Color{ 80, 80, 90, 255 });

    DrawLine(px + 40, py + 115, px + pw - 40, py + 115, Color{ 140, 140, 130, 255 });

    DrawText("PAY TO THE ORDER OF:", px + 50, py + 150, 18, Color{ 60, 60, 65, 255 });
    DrawText("Mike Schmidt", px + 270, py + 145, 24, Color{ 15, 30, 90, 255 });

    DrawText("HOURS WORKED:  30.00", px + 50, py + 210, 18, Color{ 60, 60, 65, 255 });
    DrawText("RATE:  $4.00 / hr", px + 50, py + 245, 18, Color{ 60, 60, 65, 255 });

    DrawText("TOTAL AMOUNT:", px + 50, py + 300, 22, Color{ 40, 40, 45, 255 });
    DrawText("$120.00", px + 240, py + 295, 30, Color{ 20, 90, 30, 255 });

    DrawText("FAZBEAR INC. MANAGEMENT", px + pw - 280, py + 370, 14, Color{ 90, 90, 100, 255 });
    DrawText("Press [ ENTER ] to Return to Menu", SCREEN_WIDTH / 2 - 160, SCREEN_HEIGHT - 80, 20, WHITE);
}

bool UI::HandleWinScreenInput(GameState& state, Audio& audio, Vector2 /*mousePos*/) {
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_ESCAPE) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        audio.PlayBlip();
        state = GameState::MainMenu;
    }
    return true;
}

void UI::RenderSettingsMenu(Settings& settings, Audio& /*audio*/) {
    ClearBackground(Color{ 10, 12, 16, 255 });
    Vector2 titleSz = MeasurePixelText("SETTINGS", 48.0f, 2.0f);
    DrawPixelText("SETTINGS", (SCREEN_WIDTH - titleSz.x) * 0.5f, 65.0f, 48.0f, WHITE, 2.0f);

    constexpr float labelX = 350.0f;
    constexpr float ctrlX  = 630.0f;
    constexpr float sy     = 180.0f;
    constexpr float rowH   = 65.0f;
    constexpr float sliderW = 220.0f;
    constexpr float sliderH = 22.0f;
    constexpr float toggleSize = 28.0f;

    // Master Volume
    DrawPixelText("Master Volume:", labelX, sy + 3.0f, 24.0f, WHITE, 1.2f);
    DrawRectangle(static_cast<int>(ctrlX), static_cast<int>(sy + 4.0f), static_cast<int>(sliderW), static_cast<int>(sliderH), Color{ 40, 45, 55, 255 });
    DrawRectangle(static_cast<int>(ctrlX), static_cast<int>(sy + 4.0f), static_cast<int>(settings.masterVolume * sliderW), static_cast<int>(sliderH), GREEN);
    DrawRectangleLines(static_cast<int>(ctrlX), static_cast<int>(sy + 4.0f), static_cast<int>(sliderW), static_cast<int>(sliderH), Color{ 80, 90, 110, 200 });

    // SFX Volume
    DrawPixelText("SFX Volume:", labelX, sy + rowH + 3.0f, 24.0f, WHITE, 1.2f);
    DrawRectangle(static_cast<int>(ctrlX), static_cast<int>(sy + rowH + 4.0f), static_cast<int>(sliderW), static_cast<int>(sliderH), Color{ 40, 45, 55, 255 });
    DrawRectangle(static_cast<int>(ctrlX), static_cast<int>(sy + rowH + 4.0f), static_cast<int>(settings.sfxVolume * sliderW), static_cast<int>(sliderH), GREEN);
    DrawRectangleLines(static_cast<int>(ctrlX), static_cast<int>(sy + rowH + 4.0f), static_cast<int>(sliderW), static_cast<int>(sliderH), Color{ 80, 90, 110, 200 });

    // Fullscreen
    DrawPixelText("Fullscreen:", labelX, sy + rowH * 2.0f + 3.0f, 24.0f, WHITE, 1.2f);
    DrawRectangle(static_cast<int>(ctrlX), static_cast<int>(sy + rowH * 2.0f + 2.0f), static_cast<int>(toggleSize), static_cast<int>(toggleSize), settings.fullscreen ? GREEN : GRAY);
    DrawRectangleLines(static_cast<int>(ctrlX), static_cast<int>(sy + rowH * 2.0f + 2.0f), static_cast<int>(toggleSize), static_cast<int>(toggleSize), WHITE);

    // CRT Effects
    DrawPixelText("CRT Effects:", labelX, sy + rowH * 3.0f + 3.0f, 24.0f, WHITE, 1.2f);
    DrawRectangle(static_cast<int>(ctrlX), static_cast<int>(sy + rowH * 3.0f + 2.0f), static_cast<int>(toggleSize), static_cast<int>(toggleSize), settings.crtEffects ? GREEN : GRAY);
    DrawRectangleLines(static_cast<int>(ctrlX), static_cast<int>(sy + rowH * 3.0f + 2.0f), static_cast<int>(toggleSize), static_cast<int>(toggleSize), WHITE);

    // Back Button
    Rectangle backBtn{ SCREEN_WIDTH * 0.5f - 90.0f, sy + rowH * 4.4f, 180.0f, 46.0f };
    Vector2 mPos = GetVirtualMousePosition();
    bool hoverBack = CheckCollisionPointRec(mPos, backBtn);
    DrawRectangleRec(backBtn, hoverBack ? Color{ 80, 100, 140, 255 } : Color{ 40, 50, 70, 255 });
    DrawRectangleLinesEx(backBtn, 2.0f, hoverBack ? SKYBLUE : GRAY);
    DrawCenteredPixelText("BACK", backBtn, 24.0f, WHITE, 1.2f);
}

bool UI::HandleSettingsInput(GameState& state, Settings& settings, Audio& audio, Vector2 mPos) {
    constexpr float ctrlX  = 630.0f;
    constexpr float sy     = 180.0f;
    constexpr float rowH   = 65.0f;
    constexpr float sliderW = 220.0f;
    constexpr float sliderH = 22.0f;
    constexpr float toggleSize = 28.0f;

    Rectangle masterSlider{ ctrlX, sy + 4.0f, sliderW, sliderH };
    Rectangle sfxSlider{ ctrlX, sy + rowH + 4.0f, sliderW, sliderH };
    Rectangle fsToggle{ ctrlX, sy + rowH * 2.0f + 2.0f, toggleSize, toggleSize };
    Rectangle crtToggle{ ctrlX, sy + rowH * 3.0f + 2.0f, toggleSize, toggleSize };
    Rectangle backBtn{ SCREEN_WIDTH * 0.5f - 90.0f, sy + rowH * 4.4f, 180.0f, 46.0f };

    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        if (CheckCollisionPointRec(mPos, masterSlider)) {
            settings.masterVolume = std::clamp((mPos.x - masterSlider.x) / masterSlider.width, 0.0f, 1.0f);
            settings.Apply();
            audio.SetMasterVolume(settings.masterVolume);
        }
        if (CheckCollisionPointRec(mPos, sfxSlider)) {
            settings.sfxVolume = std::clamp((mPos.x - sfxSlider.x) / sfxSlider.width, 0.0f, 1.0f);
            audio.SetSFXVolume(settings.sfxVolume);
        }
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        if (CheckCollisionPointRec(mPos, fsToggle)) {
            audio.PlayBlip();
            settings.fullscreen = !settings.fullscreen;
            settings.Apply();
        }
        if (CheckCollisionPointRec(mPos, crtToggle)) {
            audio.PlayBlip();
            settings.crtEffects = !settings.crtEffects;
        }
        if (CheckCollisionPointRec(mPos, backBtn)) {
            audio.PlayBlip();
            settings.SaveToFile();
            state = GameState::MainMenu;
        }
    }

    if (IsKeyPressed(KEY_ESCAPE)) {
        audio.PlayBlip();
        settings.SaveToFile();
        state = GameState::MainMenu;
    }
    return true;
}

void UI::RenderCustomNightMenu(NightManager& /*nightMgr*/, Audio& /*audio*/) {
    ClearBackground(Color{ 8, 8, 12, 255 });

    // High-speed Animated TV Snow (Additive Mode)
    float dt = GetFrameTime();
    m_noiseTimer += dt;
    if (m_noiseTimer >= 0.033f) {
        m_noiseTimer = 0.0f;
        if (!m_noiseTextures.empty()) {
            m_noiseFrame = (m_noiseFrame + 1) % m_noiseTextures.size();
        }
    }
    if (!m_noiseTextures.empty()) {
        BeginBlendMode(BLEND_ADDITIVE);
        Texture2D nTex = m_noiseTextures[m_noiseFrame];
        DrawTexturePro(nTex,
                       Rectangle{ 0, 0, static_cast<float>(nTex.width), static_cast<float>(nTex.height) },
                       Rectangle{ 0, 0, static_cast<float>(SCREEN_WIDTH), static_cast<float>(SCREEN_HEIGHT) },
                       Vector2{ 0, 0 }, 0.0f, Color{ 255, 255, 255, 20 });
        EndBlendMode();
    }

    // Crisp CRT Scanlines
    for (int y = 0; y < SCREEN_HEIGHT; y += 3) {
        DrawRectangle(0, y, SCREEN_WIDTH, 1, Color{ 0, 0, 0, 50 });
    }

    Vector2 titleSz = MeasurePixelText("Customize Night", 50.0f, 2.0f);
    DrawPixelText("Customize Night", (SCREEN_WIDTH - titleSz.x) * 0.5f, 40.0f, 50.0f, WHITE, 2.0f);

    Vector2 subSz = MeasurePixelText("A.I. LEVEL", 26.0f, 1.5f);
    DrawPixelText("A.I. LEVEL", (SCREEN_WIDTH - subSz.x) * 0.5f, 96.0f, 26.0f, Color{ 170, 170, 180, 255 }, 1.5f);

    struct CharCard {
        const char* name;
        int level;
        Color col;
        Texture2D tex;
    };

    CharCard cards[4] = {
        { "Freddy",  m_customBear,    Color{ 107, 63, 34, 255 },  m_bearPortrait },
        { "Bonnie",  m_customRabbit,  Color{ 111, 60, 78, 255 },  m_rabbitPortrait },
        { "Chica",   m_customChicken, Color{ 124, 108, 54, 255 }, m_chickenPortrait },
        { "Foxy",    m_customFox,     Color{ 110, 45, 50, 255 },  m_foxPortrait }
    };

    constexpr float cardW = 190.0f;
    constexpr float cardGap = 35.0f;
    constexpr float totalW = 4.0f * cardW + 3.0f * cardGap;
    float startX = (SCREEN_WIDTH - totalW) * 0.5f;
    float endX = startX + totalW;
    float cardY = 145.0f;
    Vector2 mPos = GetVirtualMousePosition();

    for (int i = 0; i < 4; ++i) {
        float cx = startX + i * (cardW + cardGap);
        Rectangle cardRect{ cx, cardY, cardW, 280.0f };
        DrawRectangleRec(cardRect, Color{ 16, 18, 24, 230 });
        DrawRectangleLinesEx(cardRect, 2.0f, cards[i].col);

        // Portrait
        if (cards[i].tex.id != 0) {
            Rectangle src{ 0, 0, static_cast<float>(cards[i].tex.width), static_cast<float>(cards[i].tex.height) };
            Rectangle dst{ cx + 15.0f, cardY + 15.0f, cardW - 30.0f, cardW - 30.0f };
            DrawTexturePro(cards[i].tex, src, dst, Vector2{ 0, 0 }, 0.0f, WHITE);
            DrawRectangleLinesEx(dst, 1.5f, cards[i].col);
        }

        // Name (Centered under portrait)
        Vector2 nameSz = MeasurePixelText(cards[i].name, 26.0f, 1.2f);
        DrawPixelText(cards[i].name, cx + (cardW - nameSz.x) * 0.5f, cardY + 185.0f, 26.0f, WHITE, 1.2f);

        // Level controls: <  LEVEL  >
        Rectangle minusBtn{ cx + 16.0f, cardY + 225.0f, 40.0f, 36.0f };
        Rectangle plusBtn{ cx + cardW - 56.0f, cardY + 225.0f, 40.0f, 36.0f };
        bool hMinus = CheckCollisionPointRec(mPos, minusBtn);
        bool hPlus = CheckCollisionPointRec(mPos, plusBtn);

        DrawRectangleRec(minusBtn, hMinus ? Color{ 80, 85, 100, 255 } : Color{ 35, 40, 50, 255 });
        DrawRectangleLinesEx(minusBtn, 1.5f, hMinus ? WHITE : GRAY);
        DrawCenteredPixelText("<", minusBtn, 24.0f, WHITE, 1.0f);

        DrawRectangleRec(plusBtn, hPlus ? Color{ 80, 85, 100, 255 } : Color{ 35, 40, 50, 255 });
        DrawRectangleLinesEx(plusBtn, 1.5f, hPlus ? WHITE : GRAY);
        DrawCenteredPixelText(">", plusBtn, 24.0f, WHITE, 1.0f);

        std::string lvlStr = std::to_string(cards[i].level);
        Rectangle lvlRect{ cx + 56.0f, cardY + 225.0f, cardW - 112.0f, 36.0f };
        DrawCenteredPixelText(lvlStr.c_str(), lvlRect, 30.0f, WHITE, 1.5f);
    }

    // Presets (bottom-left)
    Rectangle mode20Btn{ startX, 465.0f, 195.0f, 48.0f };
    Rectangle mode10Btn{ startX + 210.0f, 465.0f, 115.0f, 48.0f };
    Rectangle mode0Btn{ startX + 340.0f, 465.0f, 105.0f, 48.0f };

    auto drawPresetBtn = [&](const Rectangle& r, const char* label) {
        bool h = CheckCollisionPointRec(mPos, r);
        DrawRectangleRec(r, h ? Color{ 50, 60, 80, 255 } : Color{ 25, 30, 40, 220 });
        DrawRectangleLinesEx(r, 1.5f, h ? WHITE : GRAY);
        DrawCenteredPixelText(label, r, 20.0f, h ? WHITE : LIGHTGRAY, 1.0f);
    };

    drawPresetBtn(mode20Btn, "20/20/20/20");
    drawPresetBtn(mode10Btn, "ALL 10");
    drawPresetBtn(mode0Btn, "ALL 0");

    // Action buttons (bottom-right, strictly inside bounds, perfectly centered)
    Rectangle backBtn{ endX - 130.0f, 465.0f, 130.0f, 48.0f };
    Rectangle readyBtn{ endX - 130.0f - 20.0f - 160.0f, 465.0f, 160.0f, 48.0f };

    bool hoverReady = CheckCollisionPointRec(mPos, readyBtn);
    DrawRectangleRec(readyBtn, hoverReady ? Color{ 45, 160, 65, 255 } : Color{ 30, 110, 45, 255 });
    DrawRectangleLinesEx(readyBtn, 2.0f, hoverReady ? GREEN : LIGHTGRAY);
    DrawCenteredPixelText("READY", readyBtn, 28.0f, WHITE, 1.5f);

    bool hoverBack = CheckCollisionPointRec(mPos, backBtn);
    DrawRectangleRec(backBtn, hoverBack ? Color{ 140, 40, 40, 255 } : Color{ 80, 25, 25, 255 });
    DrawRectangleLinesEx(backBtn, 1.5f, hoverBack ? RED : GRAY);
    DrawCenteredPixelText("BACK", backBtn, 26.0f, WHITE, 1.2f);
}

bool UI::HandleCustomNightInput(GameState& state, NightManager& nightMgr, Audio& audio, Vector2 mPos) {
    constexpr float cardW = 190.0f;
    constexpr float cardGap = 35.0f;
    constexpr float totalW = 4.0f * cardW + 3.0f * cardGap;
    float startX = (SCREEN_WIDTH - totalW) * 0.5f;
    float endX = startX + totalW;
    float cardY = 145.0f;

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        // Character +/- level buttons
        for (int i = 0; i < 4; ++i) {
            float cx = startX + i * (cardW + cardGap);
            Rectangle minusBtn{ cx + 16.0f, cardY + 225.0f, 40.0f, 36.0f };
            Rectangle plusBtn{ cx + cardW - 56.0f, cardY + 225.0f, 40.0f, 36.0f };

            if (CheckCollisionPointRec(mPos, minusBtn)) {
                audio.PlayBlip();
                if (i == 0) m_customBear = std::max(0, m_customBear - 1);
                else if (i == 1) m_customRabbit = std::max(0, m_customRabbit - 1);
                else if (i == 2) m_customChicken = std::max(0, m_customChicken - 1);
                else if (i == 3) m_customFox = std::max(0, m_customFox - 1);
            }
            if (CheckCollisionPointRec(mPos, plusBtn)) {
                audio.PlayBlip();
                if (i == 0) m_customBear = std::min(20, m_customBear + 1);
                else if (i == 1) m_customRabbit = std::min(20, m_customRabbit + 1);
                else if (i == 2) m_customChicken = std::min(20, m_customChicken + 1);
                else if (i == 3) m_customFox = std::min(20, m_customFox + 1);
            }
        }

        // Preset buttons
        Rectangle mode20Btn{ startX, 465.0f, 195.0f, 48.0f };
        Rectangle mode10Btn{ startX + 210.0f, 465.0f, 115.0f, 48.0f };
        Rectangle mode0Btn{ startX + 340.0f, 465.0f, 105.0f, 48.0f };

        if (CheckCollisionPointRec(mPos, mode20Btn)) {
            audio.PlayBlip();
            m_customBear = 20;
            m_customRabbit = 20;
            m_customChicken = 20;
            m_customFox = 20;
        }
        if (CheckCollisionPointRec(mPos, mode10Btn)) {
            audio.PlayBlip();
            m_customBear = 10;
            m_customRabbit = 10;
            m_customChicken = 10;
            m_customFox = 10;
        }
        if (CheckCollisionPointRec(mPos, mode0Btn)) {
            audio.PlayBlip();
            m_customBear = 0;
            m_customRabbit = 0;
            m_customChicken = 0;
            m_customFox = 0;
        }

        // Action buttons
        Rectangle backBtn{ endX - 130.0f, 465.0f, 130.0f, 48.0f };
        Rectangle readyBtn{ endX - 130.0f - 20.0f - 160.0f, 465.0f, 160.0f, 48.0f };

        if (CheckCollisionPointRec(mPos, readyBtn)) {
            audio.PlayBlip();
            audio.PlayMenuMusic(false);
            nightMgr.SetCustomNightAI(m_customBear, m_customRabbit, m_customChicken, m_customFox);
            nightMgr.StartNight(7);
            state = GameState::NightIntro;
            return true;
        }
        if (CheckCollisionPointRec(mPos, backBtn)) {
            audio.PlayBlip();
            state = GameState::MainMenu;
            return true;
        }
    }

    if (IsKeyPressed(KEY_ESCAPE)) {
        audio.PlayBlip();
        state = GameState::MainMenu;
    }
    return true;
}

} // namespace FNAF
