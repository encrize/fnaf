# Five Nights at Freddy's — C++ Fan-Made Recreation

A complete, faithful, playable fan recreation of **Five Nights at Freddy's 1**, written from scratch in modern **C++17** using **raylib 5.x**.

This project recreates the full core FNAF 1 gameplay loop: realistic animatronic AI with authentic movement opportunities and camera interactions, interactive security office with independent blast doors and hallway lights, a multi-camera surveillance system with 11 distinct rooms, delta-timed power management, 5 progressive nights plus an unlocked Custom Night, original tutorial subtitles, procedural sound synthesis, atmospheric visual effects, and violent jumpscares.

---

## Table of Contents

1. [Features](#features)
2. [Technology Stack](#technology-stack)
3. [System Requirements](#system-requirements)
4. [Build & Installation](#build--installation)
   - [Linux](#linux)
   - [Windows](#windows)
5. [Controls](#controls)
6. [Project Structure](#project-structure)
7. [Architecture Overview](#architecture-overview)
8. [Animatronic AI Systems](#animatronic-ai-systems)
9. [Configuration & Customization](#configuration--customization)
   - [Night Difficulty (`config/nights.json`)](#night-difficulty)
   - [Power Consumption](#power-consumption)
   - [Replacing Assets & Adding Audio](#replacing-assets--adding-audio)
10. [Debug Mode](#debug-mode)

---

## Features

- **Authoritative Gameplay Loop**: Survive from 12 AM to 6 AM across 5 increasingly tense nights, leading to an authentic paycheck victory screen.
- **Interactive Security Office**:
  - Horizontal panning with smooth interpolation via mouse edge navigation or `A`/`D` keys.
  - Left and right security blast doors with motorized sliding animations.
  - Left and right hallway blindspot lights to spot animatronics lurking outside.
  - Animated desk fan with rotating blades and continuous hum.
  - Office desk props, posters, party banners, and CRT equipment.
- **11-Camera Surveillance System**:
  - Toggleable monitor interface via `SPACE` or bottom screen bar with smooth flip animations.
  - Clickable restaurant mini-map and numerical hotkeys (`1`-`9`).
  - Authoritative room layouts with unique perspective geometry and props.
  - CAM 6 Kitchen: Faithful audio-only feed with metallic pots/pans clattering cues.
  - Procedural static noise, CRT scanlines, and camera switching static bursts.
- **Authentic Animatronic AI**:
  - **Rabbit (Bonnie)**: Aggressive left-side roamer with ~4.97s interval; lurks at the left door; retreats if door is closed or attacks if ignored.
  - **Chicken (Chica)**: Persistent right-side roamer with ~4.98s interval; clatters in the kitchen; peers in through the right doorway window.
  - **Bear (Freddy)**: Stealth stalker with ~3.02s interval; freezes while directly observed on cameras; sneaks in through the East Hall Corner into the office.
  - **Fox (Foxy)**: Pirate Cove state machine (Hidden → Watching → Preparing → Running); stalled by camera surveillance; sprints down West Hall; knocks on closed door draining power or triggers a fatal jumpscare.
  - **Rare Visitor (Golden Bear)**: Rare office hallucination with a quick reaction window—flip your monitor up to dispel it before it attacks.
- **Realistic Power Management**:
  - 100% electricity pool drained based on active systems (Base, Doors, Lights, Monitor).
  - Dynamic 1–4+ usage bars.
  - Power blackout sequence: complete electrical shutdown, pitch dark office, Bear's glowing eyes in the left doorway with the iconic music box lullaby, and power-out jumpscare.
- **Procedural Sound Synthesis & Custom Asset Support**:
  - Built-in real-time 16-bit PCM waveform generator that synthesizes all sounds (doors, lights, footsteps, camera static, desk fan, jumpscare screech, Toreador music box, and 6 AM victory chime) without requiring external copyrighted assets.
  - Automatically loads custom `.wav` and `.ogg` files placed in `assets/audio/` if present.
- **Custom Night & Progression**:
  - Save game persistence (`save.dat`) recording highest unlocked night.
  - Unlocked Custom Night menu allowing AI difficulty configuration from 0 to 20 for all 4 animatronics.
- **Settings & Audio Options**:
  - In-game settings for Master Volume, SFX Volume, Music Volume, Fullscreen, and CRT Scanlines.

---

## Technology Stack

- **Language**: C++17
- **Graphics, Input & Windowing**: [raylib](https://www.raylib.com/) 5.x / 6.x
- **Audio**: raylib audio module (miniaudio backend) with real-time procedural waveform synthesizer
- **Build System**: CMake (>= 3.20)
- **Dependency Management**: CMake FetchContent (automatic raylib download) / system raylib fallback
- **Compiler Flags**: `-Wall -Wextra -Wpedantic` (GCC/Clang) or `/W4 /permissive-` (MSVC)
- **Platforms Supported**: Linux, Windows, macOS

---

## System Requirements

- **CPU**: Modern dual-core x86_64 or ARM64 processor.
- **RAM**: 512 MB.
- **GPU**: OpenGL 3.3+ compatible graphics adapter.
- **OS**: Linux (X11 / Wayland) or Windows 10/11 (64-bit).

---

## Build & Installation

### Linux

Install build tools and development libraries:
```bash
# Arch Linux / Manjaro
sudo pacman -S base-devel cmake raylib

# Ubuntu / Debian
sudo apt-get update
sudo apt-get install -y build-essential cmake libasound2-dev libx11-dev libxrandr-dev libxi-dev libgl1-mesa-dev libglu1-mesa-dev libxcursor-dev libxinerama-dev
```

Build the release executable:
```bash
git clone https://github.com/your-username/fnaf-cpp.git
cd fnaf-cpp
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```

Run the game:
```bash
./build/fnaf
```

Run the automated gameplay test suite:
```bash
./build/test_gameplay
```

### Windows

Using Visual Studio 2019/2022 with C++ Desktop Development tools:
```powershell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

The game executable will be generated at `build\Release\fnaf.exe`.

---

## Controls

| Action | Primary Input | Alternative Input |
|---|---|---|
| **Office Panning** | Move mouse to screen edges | `A` (Look Left) / `D` (Look Right) |
| **Left Door Toggle** | Click Left Wall `DOOR` button | `Q` key |
| **Left Light (Hold)** | Hold Left Wall `LIGHT` button | Hold `Z` key |
| **Right Door Toggle** | Click Right Wall `DOOR` button | `E` key |
| **Right Light (Hold)** | Hold Right Wall `LIGHT` button | Hold `C` key |
| **Toggle Camera Monitor** | Press `SPACE` | Click bottom screen flip bar |
| **Switch Cameras** | Click camera on minimap | Number keys `1`–`9` |
| **Mute / Skip Phone Call**| Press `M` | Press `ENTER` |
| **Debug Mode Overlay** | Press `F1` | — |
| **Return to Menu** | Press `ESC` | — |

---

## Project Structure

```text
FNAF-CPP/
├── CMakeLists.txt              # CMake configuration with FetchContent
├── README.md                   # Comprehensive documentation
├── .gitignore
│
├── assets/                     # Replaceable asset directory
│   ├── images/                 # Custom textures & sprites
│   ├── audio/                  # Custom .wav/.ogg sound effects
│   └── fonts/                  # Custom TTF/OTF fonts
│
├── config/                     # Game configurations
│   ├── nights.json             # AI difficulty and night duration
│   └── settings.json           # Audio volume & display settings
│
├── src/                        # C++17 Source code
│   ├── main.cpp                # Application entry point
│   ├── Common.h                # Game enums, camera IDs, locations
│   ├── Game.h / .cpp           # Master coordinator & state machine
│   ├── Office.h / .cpp         # Office rendering, doors, lights, desk fan
│   ├── Cameras.h / .cpp        # 11 cameras, surveillance HUD, room layouts
│   ├── Animatronic.h / .cpp    # AI state machines, movement intervals, attacks
│   ├── Power.h / .cpp          # Delta-timed electricity & blackout sequence
│   ├── UI.h / .cpp             # Menus, HUD, jumpscares, 6 AM & win screens
│   ├── Audio.h / .cpp          # Raylib audio & procedural wave synthesizer
│   ├── AssetManager.h / .cpp   # Centralized asset loader & fallback caching
│   ├── NightManager.h / .cpp   # Night clock, phone call subtitles, save/load
│   └── Settings.h / .cpp       # Settings persistence & raylib configuration
│
├── tests/
│   └── test_gameplay.cpp       # Automated gameplay & AI test harness
│
└── build/                      # Build output directory
```

---

## Architecture Overview

The software follows a modular design adhering to strict separation of gameplay logic and presentation:

```
                  ┌──────────────┐
                  │  main.cpp    │
                  └──────┬───────┘
                         │
                  ┌──────▼───────┐
                  │    Game      │ ◄──────────┐
                  └──────┬───────┘            │
         ┌───────────────┼───────────────┐    │
         ▼               ▼               ▼    │
   ┌───────────┐   ┌───────────┐   ┌──────────┼────────┐
   │  Office   │   │  Cameras  │   │   Power  │   UI   │
   └─────┬─────┘   └─────┬─────┘   └─────┬────┴────────┘
         │               │               │
         └───────────────┼───────────────┘
                         ▼
             ┌───────────────────────┐
             │ AnimatronicController │
             └───────────┬───────────┘
                         ▼
        ┌─────────────────────────────────┐
        │ AssetManager  •  Audio  • Clock │
        └─────────────────────────────────┘
```

- **`Game`**: Central state machine (`MainMenu`, `NightIntro`, `Playing`, `Jumpscare`, `GameOver`, `NightComplete`, `Win`, `SettingsMenu`, `CustomNightSelect`). Routes input and orchestrates subsystems.
- **`AnimatronicController`**: Authoritative AI state machine. Calculates independent timer intervals, movement rolls, door defenses, and camera observation freezes.
- **`Cameras`**: Queries authoritative animatronic positions from `AnimatronicController` to render each camera accurately. Manages the CRT noise and monitor animations.
- **`Office`**: Handles horizontal camera interpolation, door physics progress, hallway gradient lighting, and desk fan rotation.
- **`Power`**: Calculates mathematically precise power consumption based on active systems and delta time. Manages the 3-phase blackout sequence.
- **`Audio`**: Procedural audio synthesizer that generates clean 16-bit PCM buffers for missing audio assets, ensuring the game never crashes and sounds complete out of the box.

---

## Animatronic AI Systems

### 1. Rabbit (Bonnie)
- **Starting Location**: CAM 1A (Show Stage)
- **Route**: Show Stage → Backstage (CAM 5) → Dining Area (CAM 1B) → Supply Closet (CAM 3) → West Hall (CAM 2A) → West Hall Corner (CAM 2B) → Outside Left Door.
- **Movement Interval**: 4.97 seconds.
- **Behavior**: Fast and erratic. When outside the Left Door, turning on the left light reveals Rabbit staring through the window. If the door is closed, Rabbit retreats back to the Dining Area after ~2.5 seconds. If the door remains open for > 4.5 seconds, Rabbit attacks.

### 2. Chicken (Chica)
- **Starting Location**: CAM 1A (Show Stage)
- **Route**: Show Stage → Dining Area (CAM 1B) → Restrooms (CAM 7) → Kitchen (CAM 6) → East Hall (CAM 4A) → East Hall Corner (CAM 4B) → Outside Right Door.
- **Movement Interval**: 4.98 seconds.
- **Behavior**: Lingers in the Kitchen (triggering clattering audio cues). When outside the Right Door, activating the right light reveals Chicken peering in through the right window. Retreats if the door is closed; attacks if left open.

### 3. Bear (Freddy)
- **Starting Location**: CAM 1A (Show Stage)
- **Route**: Show Stage → Dining Area (CAM 1B) → Restrooms (CAM 7) → Kitchen (CAM 6) → East Hall (CAM 4A) → East Hall Corner (CAM 4B) → Inside Office.
- **Movement Interval**: 3.02 seconds.
- **Behavior**: Leaves the stage only after Rabbit and Chicken have vacated. **Camera Observation Freeze**: Does not advance if the player is actively looking at his current camera feed. If he reaches East Hall Corner and the Right Door is open, Bear sneaks into the office and triggers a fatal jumpscare.

### 4. Fox (Foxy)
- **Starting Location**: CAM 1C (Pirate Cove)
- **Stages**:
  1. *Stage 0 (Hidden)*: Completely behind the starry curtains.
  2. *Stage 1 (Watching)*: Peeking out through the parted curtains.
  3. *Stage 2 (Preparing)*: Standing outside the curtains, poised to sprint.
  4. *Stage 3 (Running)*: Pirate Cove is empty ("IT'S ME"). Foxy is sprinting down CAM 2A!
- **Behavior**: Viewing Pirate Cove stalls Foxy's progression. When Running, the player has ~3.5 seconds to close the Left Door.
  - If closed: Foxy knocks heavily on the door (`THUD THUD THUD`), drains power (`1% + 5% * knocks`), and resets to Stage 0.
  - If open: Foxy bursts into the office for an instant jumpscare.

### 5. Rare Visitor (Golden Bear)
- **Behavior**: Rare hallucination appearing slumped on the office floor when lowering the monitor. If triggered, the player must quickly raise the camera monitor within 3.0 seconds to dispel it, or suffer a reality-distorting jumpscare.

---

## Configuration & Customization

### Night Difficulty

All 5 nights can be balanced directly in `config/nights.json`:

```json
{
  "hour_duration_seconds": 60.0,
  "nights": [
    { "night": 1, "bear_ai": 0, "rabbit_ai": 3, "chicken_ai": 1, "fox_ai": 0 },
    { "night": 2, "bear_ai": 0, "rabbit_ai": 5, "chicken_ai": 3, "fox_ai": 1 },
    { "night": 3, "bear_ai": 1, "rabbit_ai": 7, "chicken_ai": 5, "fox_ai": 2 },
    { "night": 4, "bear_ai": 2, "rabbit_ai": 10, "chicken_ai": 7, "fox_ai": 5 },
    { "night": 5, "bear_ai": 3, "rabbit_ai": 12, "chicken_ai": 10, "fox_ai": 7 }
  ]
}
```

- Change `hour_duration_seconds` to shorten or lengthen the night (default is `60.0`s per hour = 360s total per night).
- AI levels range from `0` (completely dormant) to `20` (hyper-aggressive).

### Power Consumption

Power drain logic is located in `src/Power.cpp`:
- Base usage: ~0.105% / second (at 1 bar, lasts ~16 minutes of continuous operation).
- Each active system (Left Door, Right Door, Left Light, Right Light, Camera Monitor) adds +1 usage level (+0.105%/s).
- Foxy door knock drain: `1.0f + 5.0f * knocks`.

### Replacing Assets & Adding Audio

To replace procedural placeholder graphics or sounds, place files directly into the `assets/` directory:

- **Custom Audio** (`assets/audio/`):
  - `door.wav`: Door sliding and thud.
  - `light_buzz.wav`: Hallway light hum.
  - `footsteps.wav`: Hallway footsteps.
  - `jumpscare.wav`: Jumpscare screech.
  - `ambient.wav`: Night ambient drone.
  - `camera_switch.wav`: Camera switching blip.
  - `fox_knock.wav`: Foxy door impact.
  - `power_out.wav`: Power shutdown turbine wind-down.
  - `power_music.wav`: Toreador music box melody.
  - `kitchen.wav`: Pots and pans clattering.
  - `chime_6am.wav`: 6 AM celebration chime.

The asset manager caches textures and sounds automatically, falling back gracefully if any file is missing or invalid.

---

## Debug Mode

Press **`F1`** at any point during gameplay to toggle the comprehensive Debug Overlay:

```text
=== DEBUG MODE (F1 to Toggle) ===
Power: 98% | Usage: 2
Bear AI: 2 | Loc: Show Stage
Rabbit AI: 7 | Loc: West Hall Corner
Chicken AI: 5 | Loc: Restrooms
Fox AI: 2 | State: 1
Keys: [1] Move Rabbit  [2] Move Chicken
      [3] Move Bear    [4] Advance Fox
      [P] Drain Power  [G] Golden Bear
      [N] Skip to 6AM  [J] Test Jumpscare
      [Q/E] Doors      [Z/C] Lights
```

- **`1`**: Force Rabbit to advance to next room.
- **`2`**: Force Chicken to advance to next room.
- **`3`**: Force Bear to advance to next room.
- **`4`**: Advance Fox to next Pirate Cove stage.
- **`P`**: Immediately drain power to 0% to test the blackout sequence.
- **`G`**: Force spawn Golden Bear hallucination.
- **`N`**: Fast-forward night clock to 6 AM (trigger victory).
- **`J`**: Trigger immediate jumpscare sequence.
