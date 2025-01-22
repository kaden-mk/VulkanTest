#include "Application.hpp"

using namespace Engine;

int main() {
    Application app(800, 600, "Vulkan");

    try {
        app.run();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}