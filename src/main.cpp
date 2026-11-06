#include "Game.h"
#include "BundleManager.h"
#include <iostream>
#include <string>
#include <cstdlib>
#include <exception>

#if defined(_WIN32)
// Force dedicated high-performance GPU on hybrid laptops (Nvidia & AMD)
extern "C" {
    __declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

// Forward declare Win32 API functions to avoid raylib / windows.h header collisions
extern "C" __declspec(dllimport) int __stdcall MessageBoxA(void* hWnd, const char* lpText, const char* lpCaption, unsigned int uType);
#endif

static int RunApp(int argc, char* argv[]) {
    try {
        FNAF::LogDebug("=== Five Nights at Freddy's Starting ===");

#if defined(__linux__)
        // Force the X11 (XWayland) backend: raylib's GLFW Wayland backend reports a wrong
        // window size on fractionally-scaled monitors, which breaks letterboxing and mouse mapping.
        // Must run BEFORE InitWindow(). Set FNAF_WAYLAND=1 to opt back into native Wayland.
        if (!std::getenv("FNAF_WAYLAND")) {
            unsetenv("WAYLAND_DISPLAY");
        }
#endif

        if (!FNAF::EnsureAssetsReady()) {
            FNAF::LogDebug("Error: Failed to ensure assets are ready.");
#if defined(_WIN32)
            MessageBoxA(nullptr,
                "Failed to unpack game assets.\nPlease check fnaf_log.txt or %TEMP%\\fnaf_log.txt for details.",
                "FNAF Asset Error", 0x00000010L /* MB_ICONERROR */);
#endif
            return 1;
        }

        FNAF::LogDebug("Initializing game engine...");
        FNAF::Game game;
        if (!game.Initialize()) {
            FNAF::LogDebug("Error: Failed to initialize game engine (OpenGL context / window).");
#if defined(_WIN32)
            MessageBoxA(nullptr,
                "Failed to initialize FNAF game engine.\n\n"
                "Please make sure your graphics drivers support OpenGL 3.3.\n"
                "Check fnaf_log.txt or %TEMP%\\fnaf_log.txt for diagnostic details.",
                "FNAF Graphics Error", 0x00000010L /* MB_ICONERROR */);
#endif
            return 1;
        }

        FNAF::LogDebug("Game initialized successfully! Starting main loop.");
        if (argc > 1 && (std::string(argv[1]) == "--measure" || std::string(argv[1]) == "-m")) {
            game.RunMeasurement();
        } else {
            game.Run();
        }

        FNAF::LogDebug("Game loop exited normally. Goodbye!");
        return 0;
    } catch (const std::exception& ex) {
        std::string err = std::string("Fatal exception: ") + ex.what();
        FNAF::LogDebug(err);
#if defined(_WIN32)
        MessageBoxA(nullptr, err.c_str(), "FNAF Crash", 0x00000010L);
#endif
        return 1;
    } catch (...) {
        FNAF::LogDebug("Fatal unknown exception!");
#if defined(_WIN32)
        MessageBoxA(nullptr, "Fatal unknown exception occurred during startup.", "FNAF Crash", 0x00000010L);
#endif
        return 1;
    }
}

int main(int argc, char* argv[]) {
    return RunApp(argc, argv);
}

#if defined(_WIN32)
// In case linker selects WinMain entry point
extern "C" int __stdcall WinMain(void* /*hInstance*/, void* /*hPrevInstance*/, char* /*lpCmdLine*/, int /*nCmdShow*/) {
    return RunApp(__argc, __argv);
}
#endif
