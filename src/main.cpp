#include "Game.h"
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    std::cout << "[FNAF-CPP] Starting Five Nights at Freddy's...\n";

    FNAF::Game game;
    if (!game.Initialize()) {
        std::cerr << "[FNAF-CPP] Critical error: Failed to initialize game engine.\n";
        return 1;
    }

    if (argc > 1 && (std::string(argv[1]) == "--measure" || std::string(argv[1]) == "-m")) {
        game.RunMeasurement();
    } else {
        game.Run();
    }

    std::cout << "[FNAF-CPP] Game exited normally. Goodbye!\n";
    return 0;
}
