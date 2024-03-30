#include "RenderBackend.h"

RenderBackend::RenderBackend(Info& info, Core& core)
    : info{&info}, core{&core} {
    glfwInit();
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

    std::vector<const char*> vulkanInstanceExtensions;
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

    if (core->IsXRValid()) {
        uint32_t xrVulkanInstanceExtensionsCount;
        if (xrGetVulkanInstanceExtensionsKHR(
                core->GetXRInstance(), core->GetSystemID(), 0,
                &xrVulkanInstanceExtensionsCount, nullptr) != XR_SUCCESS) {
            LOGGER(LOGGER::ERR)
                << "Failed to get vulkan instance extension count";
            Cleanup();
            exit(-1);
        }

        std::string buffer(xrVulkanInstanceExtensionsCount, ' ');
        if (xrGetVulkanInstanceExtensionsKHR(core->GetXRInstance(),
                                             core->GetSystemID(), 0,
                                             &xrVulkanInstanceExtensionsCount,
                                             buffer.data()) != XR_SUCCESS) {
            LOGGER(LOGGER::ERR) << "Failed to get vulkan instance extension";
            Cleanup();
            exit(-1);
        }

        vulkanInstanceExtensions.push_back(buffer.c_str());
    }

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    createInfo.pApplicationInfo = &applicationInfo;
    createInfo.enabledExtensionCount =
        static_cast<uint32_t>(vulkanInstanceExtensions.size());
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

    if (vkCreateInstance(&createInfo, nullptr, &core->GetRenderInstance()) !=
        VK_SUCCESS) {
        LOGGER(LOGGER::ERR) << "Failed to create vulkan instance";
        exit(-1);
    }
}

void RenderBackend::CreatePhysicalDevice() {
    if (core->IsXRValid()) {
        if (xrGetVulkanGraphicsDeviceKHR(
                core->GetXRInstance(), core->GetSystemID(),
                core->GetRenderInstance(),
                &core->GetRenderPhysicalDevice()) != XR_SUCCESS) {
            LOGGER(LOGGER::ERR) << "Failed to get vulkan graphics device";
            exit(-1);
        }
    } else {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(core->GetRenderInstance(), &deviceCount, 0);
        if (deviceCount == 0) {
            LOGGER(LOGGER::ERR) << "Failed to find GPUs with Vulkan support";
            exit(-1);
        }
        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(core->GetRenderInstance(), &deviceCount,
                                   devices.data());
        std::vector<std::pair<VkPhysicalDevice, uint8_t>> suitableDevices;

        auto isDeviceSuitable = [](VkPhysicalDevice device) -> bool {
            VkPhysicalDeviceProperties deviceProperties;
            VkPhysicalDeviceFeatures deviceFeatures;
            vkGetPhysicalDeviceProperties(device, &deviceProperties);
            vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
            return deviceProperties.deviceType ==
                       VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
                   deviceFeatures.geometryShader;
        };

        for (const auto& device : devices) {
            if (isDeviceSuitable(device)) {
                suitableDevices.push_back(std::make_pair(device, 0));
            }
        }

        for (auto& device : suitableDevices) {
            VkPhysicalDeviceProperties deviceProperties;
            vkGetPhysicalDeviceProperties(device.first, &deviceProperties);
            if (deviceProperties.deviceType ==
                VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
                device.second += 1000;
            }
            if (deviceProperties.deviceType ==
                VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
                device.second += 100;
            }
            if (deviceProperties.deviceType ==
                VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU) {
                device.second += 10;
            }
        }

        auto device =
            *std::max_element(suitableDevices.begin(), suitableDevices.end(),
                              [](const auto& lhs, const auto& rhs) {
                                  return lhs.second < rhs.second;
                              });
        core->GetRenderPhysicalDevice() = device.first;

        if (core->GetRenderPhysicalDevice() == VK_NULL_HANDLE) {
            LOGGER(LOGGER::ERR) << "Failed to find suitable GPU";
            exit(-1);
        }
    }
}

void RenderBackend::LoadXRExtensionFunctions(XrInstance xrInstance) const {
    const std::vector<std::pair<PFN_xrVoidFunction*, std::string>>
        functionPairs{
            {reinterpret_cast<PFN_xrVoidFunction*>(
                 xrGetVulkanInstanceExtensionsKHR),
             "xrGetVulkanInstanceExtensionsKHR"},
            {reinterpret_cast<PFN_xrVoidFunction*>(
                 xrGetVulkanGraphicsDeviceKHR),
             "xrGetVulkanGraphicsDeviceKHR"},
            {reinterpret_cast<PFN_xrVoidFunction*>(
                 xrGetVulkanDeviceExtensionsKHR),
             "xrGetVulkanDeviceExtensionsKHR"},
            {reinterpret_cast<PFN_xrVoidFunction*>(
                 xrGetVulkanGraphicsRequirementsKHR),
             "xrGetVulkanGraphicsRequirementsKHR"},
        };

    for (const auto& [fst, snd] : functionPairs) {
        if (xrGetInstanceProcAddr(xrInstance, snd.c_str(), fst) != XR_SUCCESS) {
            LOGGER(LOGGER::WARNING)
                << "OpenXR extension" << snd << "not supported";
        }
    }
}
