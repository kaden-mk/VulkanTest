#include "VulkanRenderer.hpp"

using namespace VkRenderer;

int main() {
    VulkanRenderer renderer;

    try {
        renderer.run();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}