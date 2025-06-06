#include "main_game.hpp"

using namespace VkRenderer;

int main() {
    Game game{};

    try {
        game.run();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}