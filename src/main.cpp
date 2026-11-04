#include "Game.h"
#include <iostream>

int main() {
    std::cout << "[FNAF-CPP] Starting Five Nights at Freddy's...\n";

    FNAF::Game game;
    if (!game.Initialize()) {
        std::cerr << "[FNAF-CPP] Critical error: Failed to initialize game engine.\n";
        return 1;
    }

    game.Run();

    std::cout << "[FNAF-CPP] Game exited normally. Goodbye!\n";
    return 0;
}
