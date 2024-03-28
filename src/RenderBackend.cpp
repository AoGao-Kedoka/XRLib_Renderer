#include "RenderBackend.h"

RenderBackend::RenderBackend(Info& info) : info{&info} {
    glfwInit();
}

uint32_t RenderBackend::GetQueueFamilyIndex() {
    //TODO
    return 0;
}


void RenderBackend::Cleanup() {
    glfwTerminate();
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

    uint32_t requiredExtensionCount;
    const char** glfwExtensions =
        glfwGetRequiredInstanceExtensions(&requiredExtensionCount);
    if (!glfwExtensions) {
        LOGGER(LOGGER::ERR) << "Error getting glfw instance extension";
        exit(-1);
    }
    
    for (uint32_t i = 0; i < requiredExtensionCount; ++i) {
        vulkanInstanceExtensions.push_back(glfwExtensions[i]);
    }

    if (info->validationLayer) {
        vulkanInstanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
    
    createInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    createInfo.pApplicationInfo = &applicationInfo;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(vulkanInstanceExtensions.size());
    createInfo.ppEnabledExtensionNames = vulkanInstanceExtensions.data();
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

