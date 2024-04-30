#include "core.h"
#include "Util.h"

Core::~Core() {
    Util::VkSafeClean(vkDestroyDevice, vkDevice, nullptr);
    Util::VkSafeClean(vkDestroyInstance, vkInstance, nullptr);
}
