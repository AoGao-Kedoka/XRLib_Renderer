#include "core.h"
#include "Util.h"

Core::~Core() {
    for (auto framebuffer : swapChainFrameBuffers) {
        Util::VkSafeClean(vkDestroyFramebuffer, vkDevice, framebuffer, nullptr);
    }
    Util::VkSafeClean(vkDestroyCommandPool, vkDevice, commandPool, nullptr);

    Util::VkSafeClean(vkDestroyDevice, vkDevice, nullptr);
    Util::VkSafeClean(vkDestroyInstance, vkInstance, nullptr);
}
