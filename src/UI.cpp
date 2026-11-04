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

void UI::LoadAssets(AssetManager& assets) {
    m_bearPortrait    = assets.GetTexture("assets/images/custom_night/527.png");
    m_rabbitPortrait  = assets.GetTexture("assets/images/custom_night/528.png");
    m_chickenPortrait = assets.GetTexture("assets/images/custom_night/529.png");
    m_foxPortrait     = assets.GetTexture("assets/images/custom_night/536.png");

    m_newspaperTex    = assets.GetTexture("assets/images/newspaper.png");
    m_gameFont        = assets.GetFont("assets/fonts/Pixeboy.ttf", 32);

    // Menu Freddy Twitch Frames
    m_menuFreddyFrames.clear();
    const char* mFrames[] = {
        "freddy.png", "12.png", "13.png", "14.png", "15.png",
        "16.png", "17.png", "18.png", "20.png"
    };
    for (const char* mf : mFrames) {
        m_menuFreddyFrames.push_back(assets.GetTexture(std::string("assets/images/menu/") + mf));
    }

    // Bonnie Jumpscare Frames
    m_jumpscareBonnie.clear();
    const char* bJumps[] = {
        "291.png", "293.png", "294.png", "295.png", "296.png",
        "297.png", "298.png", "299.png", "300.png", "301.png", "303.png"
    };
    for (const char* bf : bJumps) {
        m_jumpscareBonnie.push_back(assets.GetTexture(std::string("assets/images/jumpscares/Bonnie/") + bf));
    }

    // Chica Jumpscare Frames
    m_jumpscareChica.clear();
    const char* cJumps[] = {
        "65.png", "69.png", "228.png", "229.png", "230.png", "231.png",
        "232.png", "233.png", "234.png", "235.png", "236.png", "237.png", "279.png", "281.png"
    };
    for (const char* cf : cJumps) {
        m_jumpscareChica.push_back(assets.GetTexture(std::string("assets/images/jumpscares/Chica/") + cf));
    }

    // Foxy Jumpscare Frames
    m_jumpscareFoxy.clear();
    const char* fJumps[] = {
        "242.png", "243.png", "396.png", "397.png", "398.png", "399.png", "400.png",
        "401.png", "402.png", "403.png", "404.png", "405.png", "406.png", "407.png",
        "408.png", "409.png", "410.png", "411.png", "412.png"
    };
    for (const char* ff : fJumps) {
        m_jumpscareFoxy.push_back(assets.GetTexture(std::string("assets/images/jumpscares/Foxy/Jumpscare/") + ff));
    }

    // Freddy Jumpscare Frames
    m_jumpscareFreddy.clear();
    const char* frJumps[] = {
        "307.png", "308.png", "309.png", "310.png", "311.png", "312.png", "313.png",
        "314.png", "315.png", "316.png", "317.png", "318.png", "319.png", "320.png",
        "321.png", "322.png", "323.png", "324.png", "325.png"
    };
    for (const char* frf : frJumps) {
        m_jumpscareFreddy.push_back(assets.GetTexture(std::string("assets/images/jumpscares/Freddy/1/") + frf));
    }

    // Usage Bars
    m_usageTex.clear();
    for (int i = 1; i <= 5; ++i) {
        std::string upath = "assets/images/usage/" + std::to_string(i) + ".png";
        m_usageTex.push_back(assets.GetTexture(upath));
    }
}

void UI::Reset() {
    m_menuTimer = 0.0f;
    m_jumpscareTimer = 0.0f;
    m_sixAmTimer = 0.0f;
    m_customBear = 0;
    m_customRabbit = 0;
    m_customChicken = 0;
    m_customFox = 0;
}

void UI::StartJumpscare() {
    m_jumpscareTimer = 0.0f;
}

void UI::RenderMainMenu(int currentNight, bool customNightUnlocked, Settings& /*settings*/, Audio& audio) {
    m_menuTimer += GetFrameTime();
    audio.PlayMenuMusic(true);

    ClearBackground(Color{ 6, 6, 8, 255 });

    // Animated Freddy face twitch on the right
    Texture2D freddyTex{};
    if (!m_menuFreddyFrames.empty()) {
        bool twitch = (GetRandomValue(0, 100) > 94);
        int frameIdx = twitch ? GetRandomValue(1, static_cast<int>(m_menuFreddyFrames.size() - 1)) : 0;
        freddyTex = m_menuFreddyFrames[frameIdx];
    }

    if (freddyTex.id != 0) {
        float fScale = 0.85f;
        float fw = freddyTex.width * fScale;
        float fh = freddyTex.height * fScale;
        float fx = SCREEN_WIDTH - fw - 20;
        float fy = (SCREEN_HEIGHT - fh) * 0.5f;
        DrawTexturePro(freddyTex,
                       Rectangle{ 0, 0, (float)freddyTex.width, (float)freddyTex.height },
                       Rectangle{ fx, fy, fw, fh },
                       Vector2{ 0, 0 }, 0.0f, WHITE);
    }

    // Static Noise
    for (int i = 0; i < 400; ++i) {
        int x = GetRandomValue(0, SCREEN_WIDTH);
        int y = GetRandomValue(0, SCREEN_HEIGHT);
        int val = GetRandomValue(30, 90);
        DrawPixel(x, y, Color{ static_cast<unsigned char>(val), static_cast<unsigned char>(val), static_cast<unsigned char>(val), 180 });
    }

    // Title
    DrawText("FIVE NIGHTS", 90, 80, 56, WHITE);
    DrawText("AT FREDDY'S", 90, 145, 56, Color{ 220, 50, 40, 255 });
    DrawText("1987 EDITION", 92, 215, 20, Color{ 140, 140, 150, 255 });

    // Menu Options
    Vector2 mousePos = GetVirtualMousePosition();
    float startY = 320.0f;
    float spacing = 55.0f;

    struct Option {
        const char* label;
        Rectangle rect;
    };

    std::vector<Option> options = {
        { "New Game", Rectangle{ 90, startY, 260, 45 } },
        { "Continue", Rectangle{ 90, startY + spacing, 260, 45 } },
        { "Settings", Rectangle{ 90, startY + spacing * 2, 260, 45 } }
    };

    if (customNightUnlocked) {
        options.push_back({ "Custom Night", Rectangle{ 90, startY + spacing * 3, 260, 45 } });
        options.push_back({ "Exit Game",    Rectangle{ 90, startY + spacing * 4, 260, 45 } });
    } else {
        options.push_back({ "Exit Game",    Rectangle{ 90, startY + spacing * 3, 260, 45 } });
    }

    for (const auto& opt : options) {
        bool hover = CheckCollisionPointRec(mousePos, opt.rect);
        Color col = hover ? Color{ 255, 230, 90, 255 } : WHITE;

        if (hover) {
            DrawText(">>", opt.rect.x - 35, opt.rect.y + 4, 28, Color{ 220, 50, 40, 255 });
        }
        DrawText(opt.label, opt.rect.x, opt.rect.y + 4, 30, col);

        if (std::string(opt.label) == "Continue") {
            std::string nText = "(Night " + std::to_string(currentNight) + ")";
            DrawText(nText.c_str(), opt.rect.x + 160, opt.rect.y + 8, 22, Color{ 160, 160, 170, 255 });
        }
    }

    DrawText("Press F1 for Debug Overlay | Press ESC to Exit", 90, SCREEN_HEIGHT - 45, 18, Color{ 100, 100, 110, 255 });
}

bool UI::HandleMainMenuInput(GameState& state, NightManager& nightMgr, Settings& /*settings*/, Audio& audio, Vector2 mousePos) {
    float startY = 320.0f;
    float spacing = 55.0f;

    Rectangle newGameRect{ 90, startY, 260, 45 };
    Rectangle continueRect{ 90, startY + spacing, 260, 45 };
    Rectangle settingsRect{ 90, startY + spacing * 2, 260, 45 };

    bool customUnlocked = (nightMgr.GetHighestUnlockedNight() >= 5);
    Rectangle customRect{ 90, startY + spacing * 3, 260, 45 };
    Rectangle exitRect = customUnlocked ? Rectangle{ 90, startY + spacing * 4, 260, 45 }
                                        : Rectangle{ 90, startY + spacing * 3, 260, 45 };

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
        if (CheckCollisionPointRec(mousePos, settingsRect)) {
            audio.PlayBlip();
            state = GameState::SettingsMenu;
            return true;
        }
        if (customUnlocked && CheckCollisionPointRec(mousePos, customRect)) {
            audio.PlayBlip();
            state = GameState::CustomNightSelect;
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
    // Top Right: Time & Night
    int hour = nightMgr.GetCurrentHour();
    std::string timeStr = (hour == 0) ? "12 AM" : (std::to_string(hour) + " AM");
    DrawText(timeStr.c_str(), SCREEN_WIDTH - 150, 30, 36, WHITE);

    std::string nightStr = "Night " + std::to_string(nightMgr.GetCurrentNight());
    DrawText(nightStr.c_str(), SCREEN_WIDTH - 150, 75, 24, Color{ 200, 200, 200, 255 });

    // Bottom Left: Power & Usage
    int pVal = static_cast<int>(power.GetPower());
    std::string powerStr = "Power left: " + std::to_string(pVal) + "%";
    Color pCol = (pVal > 20) ? WHITE : ((pVal > 10) ? YELLOW : RED);
    DrawText(powerStr.c_str(), 45, SCREEN_HEIGHT - 95, 24, pCol);

    DrawText("Usage:", 45, SCREEN_HEIGHT - 60, 20, WHITE);
    int usage = std::clamp(power.GetUsageLevel(), 1, 5);
    if (usage - 1 < static_cast<int>(m_usageTex.size()) && m_usageTex[usage - 1].id > 0) {
        DrawTexturePro(m_usageTex[usage - 1],
                       Rectangle{ 0, 0, static_cast<float>(m_usageTex[usage - 1].width), static_cast<float>(m_usageTex[usage - 1].height) },
                       Rectangle{ 120.0f, static_cast<float>(SCREEN_HEIGHT - 65), static_cast<float>(m_usageTex[usage - 1].width) * 1.3f, static_cast<float>(m_usageTex[usage - 1].height) * 1.3f },
                       Vector2{ 0, 0 }, 0.0f, WHITE);
    } else {
        for (int i = 0; i < 5; ++i) {
            Color barCol = (i == 0) ? GREEN : ((i == 1) ? LIME : ((i == 2) ? YELLOW : ((i == 3) ? ORANGE : RED)));
            if (i < usage) {
                DrawRectangle(125 + i * 22, SCREEN_HEIGHT - 60, 16, 20, barCol);
            } else {
                DrawRectangleLines(125 + i * 22, SCREEN_HEIGHT - 60, 16, 20, Color{ 80, 80, 90, 160 });
            }
        }
    }
}

void UI::RenderPhoneSubtitles(const std::string& text) {
    if (text.empty()) return;
    DrawRectangle(140, 20, SCREEN_WIDTH - 280, 75, Color{ 10, 10, 15, 220 });
    DrawRectangleLinesEx(Rectangle{ 140, 20, SCREEN_WIDTH - 280, 75 }, 2, Color{ 50, 50, 70, 255 });
    DrawText("[ PHONE CALL ]  (Press M or ENTER to Mute)", 160, 28, 14, YELLOW);
    DrawText(text.c_str(), 160, 50, 16, WHITE);
}

void UI::RenderJumpscare(AnimatronicID attacker, float dt, Audio& audio) {
    if (m_jumpscareTimer == 0.0f) {
        audio.PlayJumpscare();
    }
    m_jumpscareTimer += dt;

    ClearBackground(BLACK);

    std::vector<Texture2D>* frames = nullptr;
    switch (attacker) {
        case AnimatronicID::Rabbit:      frames = &m_jumpscareBonnie; break;
        case AnimatronicID::Chicken:     frames = &m_jumpscareChica; break;
        case AnimatronicID::Fox:         frames = &m_jumpscareFoxy; break;
        case AnimatronicID::RareVisitor: frames = &m_jumpscareFreddy; break;
        default:                         frames = &m_jumpscareFreddy; break;
    }

    if (frames && !frames->empty()) {
        int idx = std::clamp(static_cast<int>(m_jumpscareTimer * 22.0f), 0, static_cast<int>(frames->size() - 1));
        Texture2D fTex = (*frames)[idx];
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

void UI::RenderGameOver(float /*dt*/) {
    ClearBackground(BLACK);
    for (int i = 0; i < 600; ++i) {
        int x = GetRandomValue(0, SCREEN_WIDTH);
        int y = GetRandomValue(0, SCREEN_HEIGHT);
        int v = GetRandomValue(30, 180);
        DrawPixel(x, y, Color{ static_cast<unsigned char>(v), static_cast<unsigned char>(v), static_cast<unsigned char>(v), 200 });
    }
    DrawText("GAME OVER", SCREEN_WIDTH / 2 - 190, 280, 64, Color{ 210, 30, 30, 255 });
    DrawText("Press [ ENTER ] to Retry Night", SCREEN_WIDTH / 2 - 160, 400, 22, WHITE);
    DrawText("Press [ ESC ] for Main Menu", SCREEN_WIDTH / 2 - 150, 440, 22, LIGHTGRAY);
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

    ClearBackground(Color{ 10, 12, 16, 255 });
    if (m_sixAmTimer < 2.0f) {
        DrawText("5 AM", SCREEN_WIDTH / 2 - 70, 300, 56, Color{ 180, 180, 190, 255 });
    } else {
        DrawText("6 AM", SCREEN_WIDTH / 2 - 70, 300, 68, Color{ 240, 230, 90, 255 });
        DrawText("GREAT JOB!", SCREEN_WIDTH / 2 - 80, 400, 28, GREEN);
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
    ClearBackground(Color{ 12, 14, 18, 255 });
    DrawText("SETTINGS", SCREEN_WIDTH / 2 - 90, 80, 44, WHITE);

    float sx = 400.0f;
    float sy = 200.0f;
    float rowH = 65.0f;

    DrawText("Master Volume:", sx, sy, 22, WHITE);
    DrawRectangle(sx + 240, sy + 4, 200, 20, Color{ 40, 45, 55, 255 });
    DrawRectangle(sx + 240, sy + 4, static_cast<int>(settings.masterVolume * 200.0f), 20, GREEN);

    DrawText("SFX Volume:", sx, sy + rowH, 22, WHITE);
    DrawRectangle(sx + 240, sy + rowH + 4, 200, 20, Color{ 40, 45, 55, 255 });
    DrawRectangle(sx + 240, sy + rowH + 4, static_cast<int>(settings.sfxVolume * 200.0f), 20, GREEN);

    DrawText("Fullscreen:", sx, sy + rowH * 2, 22, WHITE);
    DrawRectangle(sx + 240, sy + rowH * 2, 26, 26, settings.fullscreen ? GREEN : GRAY);

    DrawText("CRT Effects:", sx, sy + rowH * 3, 22, WHITE);
    DrawRectangle(sx + 240, sy + rowH * 3, 26, 26, settings.crtEffects ? GREEN : GRAY);

    Rectangle backBtn{ SCREEN_WIDTH / 2.0f - 80.0f, sy + rowH * 4.5f, 160.0f, 45.0f };
    Vector2 mPos = GetVirtualMousePosition();
    bool hoverBack = CheckCollisionPointRec(mPos, backBtn);
    DrawRectangleRec(backBtn, hoverBack ? Color{ 80, 100, 140, 255 } : Color{ 40, 50, 70, 255 });
    DrawRectangleLinesEx(backBtn, 2, hoverBack ? SKYBLUE : GRAY);
    DrawText("BACK", backBtn.x + 48, backBtn.y + 12, 22, WHITE);
}

bool UI::HandleSettingsInput(GameState& state, Settings& settings, Audio& audio, Vector2 mPos) {
    float sx = 400.0f;
    float sy = 200.0f;
    float rowH = 65.0f;

    Rectangle masterSlider{ sx + 240, sy + 4, 200, 20 };
    Rectangle sfxSlider{ sx + 240, sy + rowH + 4, 200, 20 };
    Rectangle fsToggle{ sx + 240, sy + rowH * 2, 26, 26 };
    Rectangle crtToggle{ sx + 240, sy + rowH * 3, 26, 26 };
    Rectangle backBtn{ SCREEN_WIDTH / 2.0f - 80.0f, sy + rowH * 4.5f, 160.0f, 45.0f };

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
    ClearBackground(Color{ 10, 10, 14, 255 });
    DrawText("CUSTOM NIGHT", SCREEN_WIDTH / 2 - 160, 60, 46, WHITE);
    DrawText("Select AI Levels (0 - 20)", SCREEN_WIDTH / 2 - 130, 115, 22, Color{ 170, 170, 180, 255 });

    struct CharCard {
        const char* name;
        int level;
        Color col;
        Texture2D tex;
    };

    CharCard cards[4] = {
        { "Freddy",  m_customBear,    Color{ 90, 55, 30, 255 },  m_bearPortrait },
        { "Bonnie",  m_customRabbit,  Color{ 70, 55, 105, 255 }, m_rabbitPortrait },
        { "Chica",   m_customChicken, Color{ 100, 90, 35, 255 }, m_chickenPortrait },
        { "Foxy",    m_customFox,     Color{ 120, 45, 25, 255 }, m_foxPortrait }
    };

    float cardW = 180.0f;
    float startX = (SCREEN_WIDTH - (4 * cardW + 3 * 30.0f)) * 0.5f;
    float cardY = 180.0f;

    for (int i = 0; i < 4; ++i) {
        float cx = startX + i * (cardW + 30.0f);
        DrawRectangle(cx, cardY, cardW, 260, Color{ 22, 24, 30, 255 });
        DrawRectangleLinesEx(Rectangle{ cx, cardY, cardW, 260 }, 2, cards[i].col);

        if (cards[i].tex.id != 0) {
            Rectangle src{ 0, 0, (float)cards[i].tex.width, (float)cards[i].tex.height };
            Rectangle dst{ cx + 15, cardY + 15, cardW - 30, 110 };
            DrawTexturePro(cards[i].tex, src, dst, Vector2{ 0, 0 }, 0.0f, WHITE);
            DrawRectangleLinesEx(dst, 1, cards[i].col);
        } else {
            DrawRectangle(cx + 15, cardY + 15, cardW - 30, 110, cards[i].col);
            DrawText(cards[i].name, cx + 30, cardY + 55, 24, WHITE);
        }

        std::string lvlStr = std::to_string(cards[i].level);
        DrawText(lvlStr.c_str(), cx + cardW * 0.5f - 12, cardY + 150, 32, WHITE);

        DrawRectangle(cx + 25, cardY + 200, 40, 35, Color{ 40, 45, 55, 255 });
        DrawText("<", cx + 39, cardY + 204, 26, WHITE);

        DrawRectangle(cx + cardW - 65, cardY + 200, 40, 35, Color{ 40, 45, 55, 255 });
        DrawText(">", cx + cardW - 53, cardY + 204, 26, WHITE);
    }

    Rectangle readyBtn{ SCREEN_WIDTH / 2.0f - 100.0f, 500.0f, 200.0f, 55.0f };
    Vector2 mPos = GetVirtualMousePosition();
    bool hoverReady = CheckCollisionPointRec(mPos, readyBtn);
    DrawRectangleRec(readyBtn, hoverReady ? Color{ 40, 160, 60, 255 } : Color{ 30, 110, 45, 255 });
    DrawRectangleLinesEx(readyBtn, 2, hoverReady ? GREEN : LIGHTGRAY);
    DrawText("READY", readyBtn.x + 55, readyBtn.y + 14, 28, WHITE);

    DrawText("Press [ ESC ] to Return", SCREEN_WIDTH / 2 - 110, 580, 18, GRAY);
}

bool UI::HandleCustomNightInput(GameState& state, NightManager& nightMgr, Audio& audio, Vector2 mPos) {
    float cardW = 180.0f;
    float startX = (SCREEN_WIDTH - (4 * cardW + 3 * 30.0f)) * 0.5f;
    float cardY = 180.0f;

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        for (int i = 0; i < 4; ++i) {
            float cx = startX + i * (cardW + 30.0f);
            Rectangle minusBtn{ cx + 25, cardY + 200, 40, 35 };
            Rectangle plusBtn{ cx + cardW - 65, cardY + 200, 40, 35 };

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
                else if (i == 1) m_customRabbit = std::min(20, m_customRabbit - 1);
                else if (i == 2) m_customChicken = std::min(20, m_customChicken - 1);
                else if (i == 3) m_customFox = std::min(20, m_customFox + 1);
            }
        }

        Rectangle readyBtn{ SCREEN_WIDTH / 2.0f - 100.0f, 500.0f, 200.0f, 55.0f };
        if (CheckCollisionPointRec(mPos, readyBtn)) {
            audio.PlayBlip();
            nightMgr.SetCustomNightAI(m_customBear, m_customRabbit, m_customChicken, m_customFox);
            nightMgr.StartNight(7);
            state = GameState::NightIntro;
        }
    }

    if (IsKeyPressed(KEY_ESCAPE)) {
        audio.PlayBlip();
        state = GameState::MainMenu;
    }
    return true;
}

} // namespace FNAF
