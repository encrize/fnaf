#include <raylib.h>
#include <iostream>
#include <cmath>
#include <vector>

void GenerateOfficePanorama() {
    int w = 1600;
    int h = 720;
    Image img = GenImageColor(w, h, Color{ 26, 28, 34, 255 });

    // 1. Concrete / Metal Wall Panels & Seams
    for (int x = 0; x < w; x += 160) {
        ImageDrawLine(&img, x, 0, x, 560, Color{ 18, 20, 24, 255 });
        ImageDrawLine(&img, x + 1, 0, x + 1, 560, Color{ 40, 44, 52, 255 });
        // Rivets on vertical seams
        for (int y = 40; y < 560; y += 60) {
            ImageDrawCircle(&img, x, y, 2, Color{ 60, 65, 75, 255 });
        }
    }

    // Horizontal hazard stripes along upper wall trim
    for (int x = 0; x < w; x += 30) {
        Color stripeCol = ((x / 30) % 2 == 0) ? Color{ 160, 130, 20, 255 } : Color{ 20, 20, 24, 255 };
        ImageDrawRectangle(&img, x, 75, 30, 10, stripeCol);
    }

    // 2. Checkerboard Floor (y: 560 to 720)
    for (int x = 0; x < w; x += 40) {
        for (int y = 560; y < h; y += 40) {
            bool tileA = ((x / 40) + (y / 40)) % 2 == 0;
            Color tileCol = tileA ? Color{ 48, 50, 58, 255 } : Color{ 16, 17, 22, 255 };
            ImageDrawRectangle(&img, x, y, 40, 40, tileCol);
            ImageDrawRectangleLines(&img, x, y, 40, 40, Color{ 10, 10, 14, 255 });
        }
    }

    // 3. Left Doorway Opening (x: 140, y: 120, w: 160, h: 460)
    ImageDrawRectangle(&img, 140, 120, 160, 460, Color{ 6, 6, 8, 255 });
    ImageDrawRectangleLinesEx(&img, Rectangle{ 134, 114, 172, 468 }, 6, Color{ 45, 48, 55, 255 });
    ImageDrawRectangleLinesEx(&img, Rectangle{ 138, 118, 164, 464 }, 2, Color{ 15, 16, 20, 255 });

    // 4. Right Doorway Opening (x: 1300, y: 120, w: 160, h: 460)
    ImageDrawRectangle(&img, 1300, 120, 160, 460, Color{ 6, 6, 8, 255 });
    ImageDrawRectangleLinesEx(&img, Rectangle{ 1294, 114, 172, 468 }, 6, Color{ 45, 48, 55, 255 });
    ImageDrawRectangleLinesEx(&img, Rectangle{ 1298, 118, 164, 464 }, 2, Color{ 15, 16, 20, 255 });

    // 5. Party Streamers & "CELEBRATE!" Poster (x: 720, y: 120, w: 160, h: 110)
    ImageDrawRectangle(&img, 720, 120, 160, 110, Color{ 18, 18, 22, 255 });
    ImageDrawRectangleLinesEx(&img, Rectangle{ 720, 120, 160, 110 }, 4, Color{ 170, 140, 40, 255 });
    // Three character silhouettes on poster
    ImageDrawCircle(&img, 765, 185, 16, Color{ 75, 55, 105, 255 }); // Bonnie
    ImageDrawCircle(&img, 800, 178, 18, Color{ 90, 60, 35, 255 });  // Freddy
    ImageDrawCircle(&img, 835, 185, 16, Color{ 100, 90, 35, 255 }); // Chica

    // Colorful party pennant flags across ceiling
    for (int p = 0; p < 18; ++p) {
        int px = 260 + p * 60;
        Color flagCol = (p % 3 == 0) ? Color{ 200, 40, 40, 255 } : ((p % 3 == 1) ? Color{ 220, 200, 30, 255 } : Color{ 40, 140, 220, 255 });
        for (int dy = 0; dy < 25; ++dy) {
            int span = (25 - dy);
            ImageDrawLine(&img, px + 30 - span / 2, 70 + dy, px + 30 + span / 2, 70 + dy, flagCol);
        }
    }

    // 6. Overhead and Wall Hanging Conduits / Cables
    for (int x = 200; x < 1400; x += 180) {
        ImageDrawLine(&img, x, 0, x + 20, 80, Color{ 14, 14, 18, 255 });
        ImageDrawLine(&img, x + 1, 0, x + 21, 80, Color{ 20, 20, 26, 255 });
    }

    // 7. Security Desk (x: 400 to 1200, y: 500 to 720)
    ImageDrawRectangle(&img, 400, 500, 800, 220, Color{ 34, 36, 42, 255 });
    ImageDrawRectangle(&img, 400, 500, 800, 18, Color{ 50, 54, 62, 255 });
    ImageDrawRectangleLinesEx(&img, Rectangle{ 400, 500, 800, 220 }, 2, Color{ 20, 22, 26, 255 });

    // Left security mini-monitor on desk
    ImageDrawRectangle(&img, 520, 420, 95, 80, Color{ 24, 25, 28, 255 });
    ImageDrawRectangle(&img, 528, 428, 79, 64, Color{ 12, 24, 16, 255 });
    ImageDrawRectangleLinesEx(&img, Rectangle{ 520, 420, 95, 80 }, 3, Color{ 48, 50, 56, 255 });

    // Right stacked electronic consoles & telephone
    ImageDrawRectangle(&img, 980, 435, 115, 65, Color{ 26, 26, 30, 255 });
    ImageDrawRectangle(&img, 1000, 410, 75, 25, Color{ 36, 36, 42, 255 });
    ImageDrawCircle(&img, 1012, 422, 4, Color{ 210, 30, 30, 255 }); // Telephone LED

    // Crumpled soda cup & paperwork
    ImageDrawRectangle(&img, 460, 472, 24, 28, Color{ 175, 175, 185, 255 });
    ImageDrawRectangle(&img, 880, 486, 40, 14, Color{ 150, 150, 155, 255 });

    ExportImage(img, "assets/images/office_panorama.png");
    UnloadImage(img);
    std::cout << "[Texture] Generated assets/images/office_panorama.png\n";
}

void GenerateDoorTexture() {
    int w = 160;
    int h = 460;
    Image img = GenImageColor(w, h, Color{ 46, 48, 56, 255 });

    // Heavy steel plates and cross-bracing
    ImageDrawRectangle(&img, 10, 10, w - 20, h - 20, Color{ 38, 40, 46, 255 });
    ImageDrawRectangleLinesEx(&img, Rectangle{ 10, 10, (float)(w - 20), (float)(h - 20) }, 4, Color{ 25, 26, 30, 255 });

    // Vertical steel I-beams
    ImageDrawRectangle(&img, 25, 20, 14, h - 40, Color{ 55, 58, 66, 255 });
    ImageDrawRectangle(&img, w - 39, 20, 14, h - 40, Color{ 55, 58, 66, 255 });

    // Steel Rivets
    for (int y = 30; y < h - 40; y += 40) {
        ImageDrawCircle(&img, 32, y, 3, Color{ 75, 80, 92, 255 });
        ImageDrawCircle(&img, w - 32, y, 3, Color{ 75, 80, 92, 255 });
    }

    // Hazard Stripes on bottom (h - 50 to h)
    for (int s = 0; s < 6; ++s) {
        Color sCol = (s % 2 == 0) ? Color{ 200, 165, 20, 255 } : Color{ 20, 20, 24, 255 };
        ImageDrawRectangle(&img, s * 27, h - 35, 27, 35, sCol);
    }

    ExportImage(img, "assets/images/door.png");
    UnloadImage(img);
    std::cout << "[Texture] Generated assets/images/door.png\n";
}

void GenerateButtonPanelTexture() {
    int w = 60;
    int h = 125;
    Image img = GenImageColor(w, h, Color{ 22, 22, 26, 255 });
    ImageDrawRectangleLinesEx(&img, Rectangle{ 0, 0, (float)w, (float)h }, 2, Color{ 50, 50, 60, 255 });

    // Four corner screws
    ImageDrawCircle(&img, 6, 6, 2, Color{ 100, 100, 110, 255 });
    ImageDrawCircle(&img, w - 6, 6, 2, Color{ 100, 100, 110, 255 });
    ImageDrawCircle(&img, 6, h - 6, 2, Color{ 100, 100, 110, 255 });
    ImageDrawCircle(&img, w - 6, h - 6, 2, Color{ 100, 100, 110, 255 });

    ExportImage(img, "assets/images/button_panel.png");
    UnloadImage(img);
    std::cout << "[Texture] Generated assets/images/button_panel.png\n";
}

void GenerateFanBladesTexture() {
    int size = 80;
    Image img = GenImageColor(size, size, Color{ 0, 0, 0, 0 });
    int cx = size / 2;
    int cy = size / 2;

    // 3 curved metallic fan blades
    for (int b = 0; b < 3; ++b) {
        float angle = b * (2.0f * 3.14159f / 3.0f);
        for (int r = 8; r < 36; ++r) {
            int px = cx + static_cast<int>(std::cos(angle + r * 0.02f) * r);
            int py = cy + static_cast<int>(std::sin(angle + r * 0.02f) * r);
            ImageDrawCircle(&img, px, py, 4, Color{ 150, 155, 165, 255 });
        }
    }
    ImageDrawCircle(&img, cx, cy, 10, Color{ 45, 48, 55, 255 });
    ImageDrawCircle(&img, cx, cy, 4, Color{ 80, 85, 95, 255 });

    ExportImage(img, "assets/images/fan_blade.png");
    UnloadImage(img);
    std::cout << "[Texture] Generated assets/images/fan_blade.png\n";
}

void GenerateVignetteTexture() {
    int w = 1280;
    int h = 720;
    Image img = GenImageColor(w, h, Color{ 0, 0, 0, 0 });

    float cx = w * 0.5f;
    float cy = h * 0.5f;
    float maxDist = std::sqrt(cx * cx + cy * cy);

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            float dx = (x - cx);
            float dy = (y - cy);
            float dist = std::sqrt(dx * dx + dy * dy);
            float norm = dist / maxDist;
            if (norm > 0.45f) {
                float alpha = std::min(1.0f, (norm - 0.45f) / 0.55f);
                int a = static_cast<int>(alpha * alpha * 210.0f);
                ImageDrawPixel(&img, x, y, Color{ 0, 0, 0, static_cast<unsigned char>(a) });
            }
        }
    }

    ExportImage(img, "assets/images/vignette.png");
    UnloadImage(img);
    std::cout << "[Texture] Generated assets/images/vignette.png\n";
}

void GenerateCameraRooms() {
    int w = 1280;
    int h = 720;

    // 1. CAM 1A (Show Stage)
    {
        Image img = GenImageColor(w, h, Color{ 20, 16, 26, 255 });
        // Deep purple starry curtain backdrop
        ImageDrawRectangle(&img, 120, 60, w - 240, 360, Color{ 40, 22, 55, 255 });
        ImageDrawRectangleLinesEx(&img, Rectangle{ 120, 60, (float)(w - 240), 360 }, 6, Color{ 70, 45, 90, 255 });
        // Wooden stage floor
        ImageDrawRectangle(&img, 80, 380, w - 160, 200, Color{ 58, 42, 28, 255 });
        for (int p = 80; p < w - 160; p += 60) {
            ImageDrawLine(&img, p, 380, p, 580, Color{ 38, 26, 18, 255 });
        }
        // Footlights
        for (int f = 160; f < w - 160; f += 120) {
            ImageDrawCircle(&img, f, 560, 10, Color{ 220, 200, 120, 200 });
        }
        ExportImage(img, "assets/images/cam_1a.png");
        UnloadImage(img);
    }

    // 2. CAM 1B (Dining Area)
    {
        Image img = GenImageColor(w, h, Color{ 22, 24, 30, 255 });
        // Perspective tables with checkered tablecloths
        for (int t = 0; t < 3; ++t) {
            int tx = 180 + t * 320;
            ImageDrawRectangle(&img, tx, 340, 260, 110, Color{ 45, 46, 52, 255 });
            ImageDrawRectangle(&img, tx + 15, 320, 230, 25, Color{ 190, 190, 200, 255 });
            // Party hats
            ImageDrawCircle(&img, tx + 60, 310, 12, Color{ 220, 40, 40, 255 });
            ImageDrawCircle(&img, tx + 130, 310, 12, Color{ 40, 140, 220, 255 });
            ImageDrawCircle(&img, tx + 190, 310, 12, Color{ 220, 200, 40, 255 });
        }
        ExportImage(img, "assets/images/cam_1b.png");
        UnloadImage(img);
    }

    // 3. CAM 1C (Pirate Cove)
    {
        Image img = GenImageColor(w, h, Color{ 18, 16, 22, 255 });
        int cx = 380;
        int cy = 100;
        // Velvet cove curtains
        ImageDrawRectangle(&img, cx, cy, 420, 360, Color{ 48, 25, 62, 255 });
        ImageDrawRectangleLinesEx(&img, Rectangle{ (float)cx, (float)cy, 420, 360 }, 8, Color{ 80, 50, 100, 255 });
        // Gold stars on curtains
        for (int s = 0; s < 12; ++s) {
            int sx = cx + 40 + (s % 4) * 100;
            int sy = cy + 40 + (s / 4) * 80;
            ImageDrawCircle(&img, sx, sy, 6, Color{ 220, 190, 60, 255 });
        }
        ExportImage(img, "assets/images/cam_1c.png");
        UnloadImage(img);
    }

    // 4. CAM 2A (West Hall) & 4A (East Hall)
    {
        Image img2a = GenImageColor(w, h, Color{ 20, 22, 28, 255 });
        ImageDrawRectangle(&img2a, 320, 80, 540, 440, Color{ 14, 16, 20, 255 });
        ImageDrawRectangleLinesEx(&img2a, Rectangle{ 320, 80, 540, 440 }, 4, Color{ 35, 38, 46, 255 });
        ExportImage(img2a, "assets/images/cam_2a.png");
        UnloadImage(img2a);

        Image img4a = GenImageColor(w, h, Color{ 22, 20, 26, 255 });
        ImageDrawRectangle(&img4a, 320, 80, 540, 440, Color{ 16, 14, 20, 255 });
        ImageDrawRectangleLinesEx(&img4a, Rectangle{ 320, 80, 540, 440 }, 4, Color{ 40, 35, 46, 255 });
        ExportImage(img4a, "assets/images/cam_4a.png");
        UnloadImage(img4a);
    }

    // 5. CAM 3 (Supply Closet)
    {
        Image img = GenImageColor(w, h, Color{ 20, 24, 22, 255 });
        ImageDrawRectangle(&img, 280, 80, 600, 420, Color{ 15, 18, 16, 255 });
        // Shelving with bottles
        ImageDrawRectangle(&img, 320, 160, 520, 16, Color{ 60, 70, 65, 255 });
        ImageDrawRectangle(&img, 320, 260, 520, 16, Color{ 60, 70, 65, 255 });
        for (int b = 0; b < 6; ++b) {
            ImageDrawRectangle(&img, 360 + b * 80, 120, 25, 40, Color{ 80, 120, 100, 255 });
            ImageDrawRectangle(&img, 360 + b * 80, 220, 25, 40, Color{ 100, 90, 130, 255 });
        }
        ExportImage(img, "assets/images/cam_3.png");
        UnloadImage(img);
    }

    // 6. CAM 5 (Backstage)
    {
        Image img = GenImageColor(w, h, Color{ 24, 22, 26, 255 });
        ImageDrawRectangle(&img, 200, 80, 780, 440, Color{ 18, 16, 20, 255 });
        // Storage racks with endoskeleton parts
        ImageDrawRectangle(&img, 650, 180, 90, 180, Color{ 65, 68, 75, 255 });
        ImageDrawCircle(&img, 695, 160, 24, Color{ 90, 95, 105, 255 });
        ExportImage(img, "assets/images/cam_5.png");
        UnloadImage(img);
    }

    // 7. CAM 7 (Restrooms)
    {
        Image img = GenImageColor(w, h, Color{ 24, 22, 28, 255 });
        ImageDrawRectangle(&img, 240, 80, 700, 420, Color{ 18, 16, 22, 255 });
        // Restroom sign
        ImageDrawRectangle(&img, 520, 120, 140, 60, Color{ 35, 30, 45, 255 });
        ImageDrawRectangleLinesEx(&img, Rectangle{ 520, 120, 140, 60 }, 2, Color{ 80, 70, 100, 255 });
        ExportImage(img, "assets/images/cam_7.png");
        UnloadImage(img);
    }

    std::cout << "[Texture] Generated camera room backgrounds\n";
}

void GenerateAnimatronicSprites() {
    // 1. Rabbit at door (120x240)
    {
        Image img = GenImageColor(120, 240, Color{ 0, 0, 0, 0 });
        // Body & Ears
        ImageDrawRectangle(&img, 25, 90, 70, 150, Color{ 55, 42, 85, 255 });
        ImageDrawCircle(&img, 60, 75, 34, Color{ 65, 52, 95, 255 });
        ImageDrawRectangle(&img, 38, 5, 14, 55, Color{ 65, 52, 95, 255 });
        ImageDrawRectangle(&img, 68, 5, 14, 55, Color{ 65, 52, 95, 255 });
        // Red Bowtie
        ImageDrawCircle(&img, 60, 115, 8, Color{ 180, 30, 40, 255 });
        // Menacing glowing eyes
        ImageDrawCircle(&img, 48, 70, 8, Color{ 240, 240, 250, 255 });
        ImageDrawCircle(&img, 72, 70, 8, Color{ 240, 240, 250, 255 });
        ImageDrawCircle(&img, 48, 70, 4, Color{ 200, 30, 70, 255 });
        ImageDrawCircle(&img, 72, 70, 4, Color{ 200, 30, 70, 255 });
        ExportImage(img, "assets/images/rabbit_door.png");
        UnloadImage(img);
    }

    // 2. Chicken at door (120x240)
    {
        Image img = GenImageColor(120, 240, Color{ 0, 0, 0, 0 });
        ImageDrawRectangle(&img, 25, 90, 70, 150, Color{ 80, 70, 25, 255 });
        ImageDrawCircle(&img, 60, 75, 36, Color{ 95, 85, 35, 255 });
        // Feathers
        ImageDrawCircle(&img, 60, 35, 10, Color{ 95, 85, 35, 255 });
        // Bib
        ImageDrawRectangle(&img, 40, 110, 40, 45, Color{ 210, 210, 220, 255 });
        // Orange Beak & Eyes
        for (int dy = 0; dy < 18; ++dy) {
            int span = (18 - dy) * 2;
            ImageDrawLine(&img, 60 - span / 2, 85 + dy, 60 + span / 2, 85 + dy, Color{ 230, 125, 20, 255 });
        }
        ImageDrawCircle(&img, 48, 68, 8, Color{ 240, 240, 250, 255 });
        ImageDrawCircle(&img, 72, 68, 8, Color{ 240, 240, 250, 255 });
        ImageDrawCircle(&img, 48, 68, 4, Color{ 180, 40, 20, 255 });
        ImageDrawCircle(&img, 72, 68, 4, Color{ 180, 40, 20, 255 });
        ExportImage(img, "assets/images/chicken_door.png");
        UnloadImage(img);
    }

    // 3. Golden Bear seated (140x160)
    {
        Image img = GenImageColor(140, 160, Color{ 0, 0, 0, 0 });
        // Slumped Golden Freddy
        ImageDrawRectangle(&img, 30, 60, 80, 95, Color{ 145, 120, 25, 255 });
        ImageDrawCircle(&img, 70, 48, 36, Color{ 165, 138, 28, 255 });
        // Top hat
        ImageDrawRectangle(&img, 52, 10, 36, 26, BLACK);
        ImageDrawRectangle(&img, 44, 34, 52, 6, BLACK);
        // Empty black sockets with white glowing pinpoints
        ImageDrawCircle(&img, 56, 45, 9, BLACK);
        ImageDrawCircle(&img, 84, 45, 9, BLACK);
        ImageDrawCircle(&img, 56, 45, 2, WHITE);
        ImageDrawCircle(&img, 84, 45, 2, WHITE);
        ExportImage(img, "assets/images/golden_bear.png");
        UnloadImage(img);
    }

    // 4. Custom Night Portraits (140x140 each)
    {
        // Bear portrait
        Image bp = GenImageColor(140, 140, Color{ 35, 22, 14, 255 });
        ImageDrawCircle(&bp, 70, 75, 45, Color{ 80, 48, 25, 255 });
        ImageDrawCircle(&bp, 35, 40, 14, Color{ 80, 48, 25, 255 });
        ImageDrawCircle(&bp, 105, 40, 14, Color{ 80, 48, 25, 255 });
        ImageDrawRectangle(&bp, 52, 15, 36, 25, BLACK);
        ImageDrawRectangle(&bp, 42, 38, 56, 6, BLACK);
        ImageDrawCircle(&bp, 56, 70, 7, Color{ 220, 240, 255, 255 });
        ImageDrawCircle(&bp, 84, 70, 7, Color{ 220, 240, 255, 255 });
        ExportImage(bp, "assets/images/bear_portrait.png");
        UnloadImage(bp);

        // Rabbit portrait
        Image rp = GenImageColor(140, 140, Color{ 28, 20, 38, 255 });
        ImageDrawCircle(&rp, 70, 80, 44, Color{ 68, 52, 100, 255 });
        ImageDrawRectangle(&rp, 46, 12, 14, 45, Color{ 68, 52, 100, 255 });
        ImageDrawRectangle(&rp, 80, 12, 14, 45, Color{ 68, 52, 100, 255 });
        ImageDrawCircle(&rp, 56, 75, 7, Color{ 220, 30, 60, 255 });
        ImageDrawCircle(&rp, 84, 75, 7, Color{ 220, 30, 60, 255 });
        ExportImage(rp, "assets/images/rabbit_portrait.png");
        UnloadImage(rp);

        // Chicken portrait
        Image cp = GenImageColor(140, 140, Color{ 38, 34, 18, 255 });
        ImageDrawCircle(&cp, 70, 75, 45, Color{ 95, 85, 35, 255 });
        ImageDrawCircle(&cp, 70, 28, 12, Color{ 95, 85, 35, 255 });
        ImageDrawCircle(&cp, 56, 70, 8, Color{ 240, 40, 30, 255 });
        ImageDrawCircle(&cp, 84, 70, 8, Color{ 240, 40, 30, 255 });
        ExportImage(cp, "assets/images/chicken_portrait.png");
        UnloadImage(cp);

        // Fox portrait
        Image fp = GenImageColor(140, 140, Color{ 40, 18, 15, 255 });
        ImageDrawCircle(&fp, 70, 75, 44, Color{ 120, 45, 25, 255 });
        ImageDrawCircle(&fp, 56, 70, 7, Color{ 255, 230, 80, 255 });
        ImageDrawRectangle(&fp, 80, 62, 16, 16, BLACK); // Eyepatch
        ExportImage(fp, "assets/images/fox_portrait.png");
        UnloadImage(fp);
    }

    std::cout << "[Texture] Generated animatronic sprites and portraits\n";
}

int main() {
    InitWindow(100, 100, "TextureGenerator");
    GenerateOfficePanorama();
    GenerateDoorTexture();
    GenerateButtonPanelTexture();
    GenerateFanBladesTexture();
    GenerateVignetteTexture();
    GenerateCameraRooms();
    GenerateAnimatronicSprites();
    CloseWindow();
    std::cout << "[TextureGenerator] All textures successfully generated!\n";
    return 0;
}
