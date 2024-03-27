#include "RenderBackend.h"

RenderBackend::RenderBackend(Info& info) : info{&info} {
    CreateVulkanInstance();
}

uint32_t RenderBackend::GetQueueFamilyIndex() {
    return 0;
}

void RenderBackend::Cleanup() {
}

void RenderBackend::CreateVulkanInstance() {
    VkApplicationInfo applicationInfo{};
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.pApplicationName = info->applicationName.c_str();
    applicationInfo.applicationVersion = VK_MAKE_VERSION(
        info->majorVersion, info->minorVersion, info->patchVersion);
    applicationInfo.pEngineName = info->applicationName.c_str();
    applicationInfo.engineVersion = VK_MAKE_VERSION(
        info->majorVersion, info->minorVersion, info->patchVersion);
    applicationInfo.apiVersion = VK_API_VERSION_1_2;

    std::vector<const char*> vulkanInstanceExtensions;
    uint32_t requiredExtensionCount;
    const char** glfwExtensions =
        glfwGetRequiredInstanceExtensions(&requiredExtensionCount);
    if (!glfwExtensions) {
        LOGGER(LOGGER::ERR) << "Error getting glfw instance extension";
        exit(-1);
    }

    std::vector<const char*> extensions(
        glfwExtensions, glfwExtensions + requiredExtensionCount);

    if (info->validationLayer) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
    
    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    createInfo.pApplicationInfo = &applicationInfo;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();
    if (info->validationLayer) {
        createInfo.enabledLayerCount =
            static_cast<uint32_t>(validataionLayers.size());
        createInfo.ppEnabledLayerNames = validataionLayers.data();
        //TODO populate debug messenger
    } else {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;
    }

    if (vkCreateInstance(&createInfo, nullptr, &vkInstance) != VK_SUCCESS) {
        LOGGER(LOGGER::ERR) << "Failed to create vulkan instance";
        exit(-1);
    }
}

