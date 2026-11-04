#include "UI.h"
#include "Audio.h"
#include "Settings.h"
#include "NightManager.h"
#include "Power.h"
#include <cmath>
#include <algorithm>

namespace FNAF {

UI::UI() {
    Reset();
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

void UI::RenderMainMenu(int currentNight, bool customNightUnlocked, Settings& /*settings*/, Audio& /*audio*/) {
    m_menuTimer += GetFrameTime();

    // Dark background with flickering CRT noise
    ClearBackground(Color{ 8, 8, 12, 255 });

    // Procedural TV static noise in background
    for (int i = 0; i < 400; ++i) {
        int x = GetRandomValue(0, SCREEN_WIDTH);
        int y = GetRandomValue(0, SCREEN_HEIGHT);
        int val = GetRandomValue(30, 90);
        DrawPixel(x, y, Color{ static_cast<unsigned char>(val), static_cast<unsigned char>(val), static_cast<unsigned char>(val), 180 });
    }

    // Occasional Bear face twitching on the right
    bool twitch = (GetRandomValue(0, 100) > 96);
    float bx = SCREEN_WIDTH * 0.65f + (twitch ? GetRandomValue(-10, 10) : 0);
    float by = SCREEN_HEIGHT * 0.35f + (twitch ? GetRandomValue(-5, 5) : 0);

    // Menacing Bear silhouette
    DrawCircle(bx, by, 120, Color{ 35, 22, 14, 255 });
    DrawCircle(bx - 90, by - 70, 35, Color{ 35, 22, 14, 255 });
    DrawCircle(bx + 90, by - 70, 35, Color{ 35, 22, 14, 255 });
    // Top hat
    DrawRectangle(bx - 40, by - 165, 80, 60, BLACK);
    DrawRectangle(bx - 60, by - 110, 120, 12, BLACK);
    // Glowing eyes
    Color eyeColor = twitch ? Color{ 240, 240, 255, 255 } : Color{ 160, 190, 220, 255 };
    DrawCircle(bx - 40, by - 15, 12, eyeColor);
    DrawCircle(bx + 40, by - 15, 12, eyeColor);
    DrawCircle(bx - 40, by - 15, 5, WHITE);
    DrawCircle(bx + 40, by - 15, 5, WHITE);

    // Title
    DrawText("FIVE NIGHTS", 90, 90, 54, WHITE);
    DrawText("AT FREDDY'S", 90, 150, 54, Color{ 220, 50, 40, 255 });
    DrawText("C++17 RAYLIB EDITION", 92, 215, 20, Color{ 140, 140, 150, 255 });

    // Menu Options
    Vector2 mousePos = GetMousePosition();
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

    // Version label
    DrawText("v1.0.0 | Press F1 for Debug Mode", 90, SCREEN_HEIGHT - 45, 18, Color{ 100, 100, 110, 255 });
}

bool UI::HandleMainMenuInput(GameState& state, NightManager& nightMgr, Settings& /*settings*/, Audio& audio) {
    Vector2 mousePos = GetMousePosition();
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
            nightMgr.StartNight(1);
            state = GameState::NightIntro;
            return true;
        }
        if (CheckCollisionPointRec(mousePos, continueRect)) {
            audio.PlayBlip();
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
            return false; // Request exit
        }
    }
    return true;
}

void UI::RenderHUD(const NightManager& nightMgr, const Power& power, bool /*monitorUp*/) {
    // 1. Top Right: Time & Night
    int hour = nightMgr.GetCurrentHour();
    std::string timeStr = (hour == 0) ? "12 AM" : (std::to_string(hour) + " AM");
    DrawText(timeStr.c_str(), SCREEN_WIDTH - 150, 30, 36, WHITE);

    std::string nightStr = "Night " + std::to_string(nightMgr.GetCurrentNight());
    DrawText(nightStr.c_str(), SCREEN_WIDTH - 150, 75, 24, Color{ 200, 200, 200, 255 });

    // 2. Bottom Left: Power & Usage
    int pVal = static_cast<int>(power.GetPower());
    std::string powerStr = "Power left: " + std::to_string(pVal) + "%";
    Color pCol = (pVal > 20) ? WHITE : ((pVal > 10) ? YELLOW : RED);
    DrawText(powerStr.c_str(), 45, SCREEN_HEIGHT - 95, 24, pCol);

    DrawText("Usage:", 45, SCREEN_HEIGHT - 60, 20, WHITE);
    int usage = power.GetUsageLevel(); // 1 to 5
    for (int i = 0; i < 4; ++i) {
        Color barCol;
        if (i == 0) barCol = GREEN;
        else if (i == 1) barCol = LIME;
        else if (i == 2) barCol = ORANGE;
        else barCol = RED;

        if (i < usage) {
            DrawRectangle(125 + i * 22, SCREEN_HEIGHT - 60, 16, 20, barCol);
        } else {
            DrawRectangleLines(125 + i * 22, SCREEN_HEIGHT - 60, 16, 20, Color{ 80, 80, 90, 160 });
        }
    }
}

void UI::RenderPhoneSubtitles(const std::string& text) {
    if (text.empty()) return;

    // Subtitle banner at top
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

    // Violent screen shake
    float shakeX = (GetRandomValue(-25, 25));
    float shakeY = (GetRandomValue(-20, 20));

    // Strobe / blood-red flash
    bool redFlash = (GetRandomValue(0, 10) > 6);
    ClearBackground(redFlash ? Color{ 80, 10, 10, 255 } : Color{ 10, 10, 12, 255 });

    // Attacker silhouette scaling rapidly into player face
    float scale = 1.0f + m_jumpscareTimer * 1.8f;
    float cx = SCREEN_WIDTH * 0.5f + shakeX;
    float cy = SCREEN_HEIGHT * 0.55f + shakeY;

    switch (attacker) {
        case AnimatronicID::Rabbit: {
            DrawCircle(cx, cy, 140 * scale, Color{ 70, 55, 100, 255 });
            DrawRectangle(cx - 50 * scale, cy - 250 * scale, 30 * scale, 130 * scale, Color{ 70, 55, 100, 255 });
            DrawRectangle(cx + 20 * scale, cy - 250 * scale, 30 * scale, 130 * scale, Color{ 70, 55, 100, 255 });
            // Wide terrifying red pupils
            DrawCircle(cx - 40 * scale, cy - 20 * scale, 18 * scale, Color{ 220, 30, 50, 255 });
            DrawCircle(cx + 40 * scale, cy - 20 * scale, 18 * scale, Color{ 220, 30, 50, 255 });
            // Open metallic jaw with sharp teeth
            DrawRectangle(cx - 70 * scale, cy + 40 * scale, 140 * scale, 70 * scale, BLACK);
            for (int t = 0; t < 6; ++t) {
                DrawRectangle(cx - 60 * scale + t * 20 * scale, cy + 42 * scale, 12 * scale, 18 * scale, WHITE);
            }
            break;
        }
        case AnimatronicID::Chicken: {
            DrawCircle(cx, cy, 145 * scale, Color{ 95, 85, 30, 255 });
            // Orange beak agape
            DrawTriangle(Vector2{ cx, cy + 80 * scale }, Vector2{ cx - 60 * scale, cy + 15 * scale }, Vector2{ cx + 60 * scale, cy + 15 * scale }, Color{ 230, 120, 20, 255 });
            DrawCircle(cx - 45 * scale, cy - 25 * scale, 20 * scale, Color{ 240, 40, 30, 255 });
            DrawCircle(cx + 45 * scale, cy - 25 * scale, 20 * scale, Color{ 240, 40, 30, 255 });
            break;
        }
        case AnimatronicID::Fox: {
            DrawCircle(cx, cy, 135 * scale, Color{ 120, 45, 25, 255 });
            DrawTriangle(Vector2{ cx - 70 * scale, cy - 140 * scale }, Vector2{ cx - 110 * scale, cy - 50 * scale }, Vector2{ cx - 30 * scale, cy - 50 * scale }, Color{ 120, 45, 25, 255 });
            DrawTriangle(Vector2{ cx + 70 * scale, cy - 140 * scale }, Vector2{ cx + 30 * scale, cy - 50 * scale }, Vector2{ cx + 110 * scale, cy - 50 * scale }, Color{ 120, 45, 25, 255 });
            DrawCircle(cx - 40 * scale, cy - 20 * scale, 16 * scale, Color{ 255, 230, 80, 255 });
            // Eyepatch on right eye
            DrawRectangle(cx + 25 * scale, cy - 35 * scale, 35 * scale, 30 * scale, BLACK);
            DrawRectangle(cx - 60 * scale, cy + 30 * scale, 120 * scale, 80 * scale, BLACK);
            break;
        }
        case AnimatronicID::RareVisitor: {
            DrawCircle(cx, cy, 150 * scale, Color{ 160, 135, 25, 255 });
            DrawCircle(cx - 45 * scale, cy - 20 * scale, 24 * scale, BLACK);
            DrawCircle(cx + 45 * scale, cy - 20 * scale, 24 * scale, BLACK);
            DrawCircle(cx - 45 * scale, cy - 20 * scale, 4 * scale, WHITE);
            DrawCircle(cx + 45 * scale, cy - 20 * scale, 4 * scale, WHITE);
            DrawText("IT'S ME", cx - 80 * scale, cy + 80 * scale, static_cast<int>(32 * scale), Color{ 200, 30, 30, 255 });
            break;
        }
        default: { // Bear
            DrawCircle(cx, cy, 150 * scale, Color{ 80, 48, 25, 255 });
            DrawCircle(cx - 100 * scale, cy - 90 * scale, 40 * scale, Color{ 80, 48, 25, 255 });
            DrawCircle(cx + 100 * scale, cy - 90 * scale, 40 * scale, Color{ 80, 48, 25, 255 });
            DrawRectangle(cx - 50 * scale, cy - 200 * scale, 100 * scale, 70 * scale, BLACK);
            DrawCircle(cx - 45 * scale, cy - 20 * scale, 16 * scale, Color{ 200, 220, 255, 255 });
            DrawCircle(cx + 45 * scale, cy - 20 * scale, 16 * scale, Color{ 200, 220, 255, 255 });
            DrawRectangle(cx - 70 * scale, cy + 35 * scale, 140 * scale, 65 * scale, BLACK);
            break;
        }
    }
}

void UI::RenderGameOver(float /*dt*/) {
    ClearBackground(BLACK);

    // Static overlay
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

bool UI::HandleGameOverInput(GameState& state, NightManager& nightMgr, Audio& audio) {
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
    m_sixAmTimer += dt;

    ClearBackground(Color{ 10, 12, 16, 255 });

    // Transition from 5 AM to 6 AM
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
    ClearBackground(Color{ 20, 22, 28, 255 });

    // Vintage Paycheck certificate
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

bool UI::HandleWinScreenInput(GameState& state, Audio& audio) {
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

    // Master Volume
    DrawText("Master Volume:", sx, sy, 22, WHITE);
    DrawRectangle(sx + 240, sy + 4, 200, 20, Color{ 40, 45, 55, 255 });
    DrawRectangle(sx + 240, sy + 4, static_cast<int>(settings.masterVolume * 200.0f), 20, GREEN);

    // SFX Volume
    DrawText("SFX Volume:", sx, sy + rowH, 22, WHITE);
    DrawRectangle(sx + 240, sy + rowH + 4, 200, 20, Color{ 40, 45, 55, 255 });
    DrawRectangle(sx + 240, sy + rowH + 4, static_cast<int>(settings.sfxVolume * 200.0f), 20, GREEN);

    // Fullscreen
    DrawText("Fullscreen:", sx, sy + rowH * 2, 22, WHITE);
    DrawRectangle(sx + 240, sy + rowH * 2, 26, 26, settings.fullscreen ? GREEN : GRAY);

    // CRT Scanlines
    DrawText("CRT Effects:", sx, sy + rowH * 3, 22, WHITE);
    DrawRectangle(sx + 240, sy + rowH * 3, 26, 26, settings.crtEffects ? GREEN : GRAY);

    // Back Button
    Rectangle backBtn{ SCREEN_WIDTH / 2.0f - 80.0f, sy + rowH * 4.5f, 160.0f, 45.0f };
    Vector2 mPos = GetMousePosition();
    bool hoverBack = CheckCollisionPointRec(mPos, backBtn);
    DrawRectangleRec(backBtn, hoverBack ? Color{ 80, 100, 140, 255 } : Color{ 40, 50, 70, 255 });
    DrawRectangleLinesEx(backBtn, 2, hoverBack ? SKYBLUE : GRAY);
    DrawText("BACK", backBtn.x + 48, backBtn.y + 12, 22, WHITE);
}

bool UI::HandleSettingsInput(GameState& state, Settings& settings, Audio& audio) {
    Vector2 mPos = GetMousePosition();
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

    // 4 Character Columns
    struct CharCard {
        const char* name;
        int level;
        Color col;
    };

    CharCard cards[4] = {
        { "Bear",    m_customBear,    Color{ 90, 55, 30, 255 } },
        { "Rabbit",  m_customRabbit,  Color{ 70, 55, 105, 255 } },
        { "Chicken", m_customChicken, Color{ 100, 90, 35, 255 } },
        { "Fox",     m_customFox,     Color{ 120, 45, 25, 255 } }
    };

    float cardW = 180.0f;
    float startX = (SCREEN_WIDTH - (4 * cardW + 3 * 30.0f)) * 0.5f;
    float cardY = 180.0f;

    for (int i = 0; i < 4; ++i) {
        float cx = startX + i * (cardW + 30.0f);
        DrawRectangle(cx, cardY, cardW, 260, Color{ 22, 24, 30, 255 });
        DrawRectangleLinesEx(Rectangle{ cx, cardY, cardW, 260 }, 2, cards[i].col);

        // Portrait
        DrawRectangle(cx + 15, cardY + 15, cardW - 30, 110, cards[i].col);
        DrawText(cards[i].name, cx + 30, cardY + 55, 24, WHITE);

        // Level display
        std::string lvlStr = std::to_string(cards[i].level);
        DrawText(lvlStr.c_str(), cx + cardW * 0.5f - 12, cardY + 150, 32, WHITE);

        // Minus button
        DrawRectangle(cx + 25, cardY + 200, 40, 35, Color{ 40, 45, 55, 255 });
        DrawText("-", cx + 39, cardY + 204, 26, WHITE);

        // Plus button
        DrawRectangle(cx + cardW - 65, cardY + 200, 40, 35, Color{ 40, 45, 55, 255 });
        DrawText("+", cx + cardW - 53, cardY + 204, 26, WHITE);
    }

    // Ready Button
    Rectangle readyBtn{ SCREEN_WIDTH / 2.0f - 100.0f, 500.0f, 200.0f, 55.0f };
    Vector2 mPos = GetMousePosition();
    bool hoverReady = CheckCollisionPointRec(mPos, readyBtn);
    DrawRectangleRec(readyBtn, hoverReady ? Color{ 40, 160, 60, 255 } : Color{ 30, 110, 45, 255 });
    DrawRectangleLinesEx(readyBtn, 2, hoverReady ? GREEN : LIGHTGRAY);
    DrawText("READY", readyBtn.x + 55, readyBtn.y + 14, 28, WHITE);

    // Back to Menu
    DrawText("Press [ ESC ] to Return", SCREEN_WIDTH / 2 - 110, 580, 18, GRAY);
}

bool UI::HandleCustomNightInput(GameState& state, NightManager& nightMgr, Audio& audio) {
    Vector2 mPos = GetMousePosition();
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
                else if (i == 3) m_customFox = std::min(20, m_customFox - 1);
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
