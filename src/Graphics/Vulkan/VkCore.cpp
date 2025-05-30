#include "VkCore.h"

namespace XRLib {
namespace Graphics {
VkCore::~VkCore() {
    vkDeviceWaitIdle(GetRenderDevice());
    if (vkDebugMessenger != VK_NULL_HANDLE) {
        PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT =
            reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
                vkGetInstanceProcAddr(GetRenderInstance(), "vkDestroyDebugUtilsMessengerEXT"));
        vkDestroyDebugUtilsMessengerEXT(GetRenderInstance(), vkDebugMessenger, nullptr);
    }

    VkUtil::VkSafeClean(vkDestroyCommandPool, vkDevice, commandPool, nullptr);
    VkUtil::VkSafeClean(vkDestroyDescriptorPool, vkDevice, descriptorPool, nullptr);

    VkUtil::VkSafeClean(vkDestroySemaphore, vkDevice, imageAvailableSemaphore, nullptr);
    VkUtil::VkSafeClean(vkDestroySemaphore, vkDevice, renderFinishedSemaphore, nullptr);
    VkUtil::VkSafeClean(vkDestroyFence, vkDevice, inFlightFence, nullptr);

    VkUtil::VkSafeClean(vkDestroyDevice, vkDevice, nullptr);
    VkUtil::VkSafeClean(vkDestroySurfaceKHR, vkInstance, surfaceFlat, nullptr);
    VkUtil::VkSafeClean(vkDestroyInstance, vkInstance, nullptr);
}

void VkCore::CreateVkInstance(Config& config, const std::vector<const char*>& additionalInstanceExts) {
    if (config.validationLayer) {
        for (const char* layer : validataionLayers) {
            bool res = VkUtil::VkCheckLayerSupport(layer);

            // disable validataion if validation layer is not supported
            if (!res) {
                config.validationLayer = false;
                LOGGER(LOGGER::WARNING) << "Validation layer not available, disabling validataion layer";
            }
        }
    }

    VkApplicationInfo applicationInfo{};
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.pApplicationName = config.applicationName.c_str();
    applicationInfo.applicationVersion = VK_MAKE_VERSION(config.majorVersion, config.minorVersion, config.patchVersion);
    applicationInfo.pEngineName = config.applicationName.c_str();
    applicationInfo.engineVersion = VK_MAKE_VERSION(config.majorVersion, config.minorVersion, config.patchVersion);
    applicationInfo.apiVersion = VK_API_VERSION_1_3;

    std::vector<const char*> vulkanInstanceExtensions;
    uint32_t requiredExtensionCount;
    auto windowExtensions = WindowHandler::VkGetWindowExtensions(&requiredExtensionCount);

    // compute vulkan instance extensions
    for (uint32_t i = 0; i < requiredExtensionCount; ++i) {
        vulkanInstanceExtensions.push_back(windowExtensions[i]);
    }

    if (config.validationLayer) {
        vulkanInstanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    for (const auto& exts : additionalInstanceExts) {
        vulkanInstanceExtensions.push_back(exts);
    }
#ifdef __APPLE__
    vulkanInstanceExtensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    vulkanInstanceExtensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
#endif

    VkInstanceCreateInfo instanceCreateInfo{};
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    std::vector<VkValidationFeatureEnableEXT> enabledValidationFeatures;

    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pApplicationInfo = &applicationInfo;

    vulkanInstanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    vulkanInstanceExtensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

    instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(vulkanInstanceExtensions.size());
    instanceCreateInfo.ppEnabledExtensionNames = vulkanInstanceExtensions.data();
#ifdef __APPLE__
    instanceCreateInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif

    if (config.validationLayer) {
        debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                          VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                          VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                      VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                      VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        debugCreateInfo.pfnUserCallback = VkUtil::VkDebugCallback;

        instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validataionLayers.size());
        instanceCreateInfo.ppEnabledLayerNames = validataionLayers.data();
    } else {
        instanceCreateInfo.enabledLayerCount = 0;
        instanceCreateInfo.pNext = nullptr;
    }

    if (vkCreateInstance(&instanceCreateInfo, nullptr, &vkInstance) != VK_SUCCESS) {
        Util::ErrorPopup("Failed to create vulkan instance");
    }

    if (config.validationLayer) {
        vkCreateDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
            vkGetInstanceProcAddr(GetRenderInstance(), "vkCreateDebugUtilsMessengerEXT"));
        if (vkCreateDebugUtilsMessengerEXT(GetRenderInstance(), &debugCreateInfo, nullptr, &vkDebugMessenger) !=
            VK_SUCCESS) {
            LOGGER(LOGGER::WARNING) << "Failed to create debug utils messenger";
        }
    }
}

void VkCore::SelectPhysicalDevice() {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(GetRenderInstance(), &deviceCount, 0);
    if (deviceCount == 0) {
        Util::ErrorPopup("Failed to find GPUs with Vulkan support");
    }
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(GetRenderInstance(), &deviceCount, devices.data());

    std::vector<std::pair<VkPhysicalDevice, uint8_t>> suitableDevices;

    auto isDeviceSuitable = [](VkPhysicalDevice device) -> bool {
        VkPhysicalDeviceProperties deviceProperties;
        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);
        vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
        return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && deviceFeatures.geometryShader ||
               VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU;
    };

    for (const auto& device : devices) {
        if (isDeviceSuitable(device)) {
            suitableDevices.push_back(std::make_pair(device, 0));
        }
    }

    for (auto& device : suitableDevices) {
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(device.first, &deviceProperties);
        if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            device.second += 1000;
        }
        if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
            device.second += 100;
        }
        if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU) {
            device.second += 10;
        }
    }

    auto device = *std::max_element(suitableDevices.begin(), suitableDevices.end(),
                                    [](const auto& lhs, const auto& rhs) { return lhs.second < rhs.second; });
    vkPhysicalDevice = device.first;

    if (GetRenderPhysicalDevice() == VK_NULL_HANDLE) {
        Util::ErrorPopup("Failed to find suitable GPU");
    }
}

void VkCore::CreateVkDevice(Config& config, const std::vector<const char*>& additionalDeviceExts, bool xr) {
    std::vector<const char*> deviceExtensions(0);

    deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    if (config.validationLayer) {
        deviceExtensions.push_back(VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME);
    }

    for (const auto& exts : additionalDeviceExts) {
        deviceExtensions.push_back(exts);
    }

#ifdef __APPLE__
    deviceExtensions.push_back("VK_KHR_portability_subset");
#endif

    float queuePriority = 1;

    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = GetGraphicsQueueFamilyIndex();
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    VkPhysicalDeviceDescriptorIndexingFeaturesEXT indexingFeatures{};
    indexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT;
    indexingFeatures.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
    indexingFeatures.runtimeDescriptorArray = VK_TRUE;
    indexingFeatures.descriptorBindingPartiallyBound = VK_TRUE;
    indexingFeatures.descriptorBindingVariableDescriptorCount = VK_TRUE;

    // TODO: remove after switching to XR_KHR_vulkan_enable2
    VkPhysicalDeviceTimelineSemaphoreFeatures timelineSemaphoreFeatures{};
    timelineSemaphoreFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES;
    timelineSemaphoreFeatures.timelineSemaphore = true;
    indexingFeatures.pNext = &timelineSemaphoreFeatures;

#ifdef __APPLE__
    VkPhysicalDevicePortabilitySubsetFeaturesKHR portabilitySubsetFeatures{};
    portabilitySubsetFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PORTABILITY_SUBSET_FEATURES_KHR;
    portabilitySubsetFeatures.mutableComparisonSamplers = VK_TRUE;
    timelineSemaphoreFeatures.pNext = &portabilitySubsetFeatures;
#endif

    VkDeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
    deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
    deviceCreateInfo.pNext = &indexingFeatures;

    VkPhysicalDeviceMultiviewFeaturesKHR physicalDeviceMultiviewFeatures{};
    if (xr) {
        physicalDeviceMultiviewFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_FEATURES_KHR;
        physicalDeviceMultiviewFeatures.multiview = VK_TRUE;
        deviceCreateInfo.pNext = &physicalDeviceMultiviewFeatures;
    }

    if (vkCreateDevice(GetRenderPhysicalDevice(), &deviceCreateInfo, nullptr, &vkDevice) != VK_SUCCESS) {
        Util::ErrorPopup("Failed to create vulkan device.");
    }

    vkGetDeviceQueue(GetRenderDevice(), GetGraphicsQueueFamilyIndex(), 0, &graphicsQueue);
}

uint32_t VkCore::GetMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(GetRenderPhysicalDevice(), &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    Util::ErrorPopup("Failed to find suitable memory type!");
    return -1;
}

void VkCore::CreateSyncSemaphore(VkSemaphore& semaphore) {
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    if (vkCreateSemaphore(GetRenderDevice(), &semaphoreInfo, nullptr, &semaphore) != VK_SUCCESS) {

        Util::ErrorPopup("Failed to create semaphore");
    }
}

void VkCore::CreateFence(VkFence& fence) {
    VkFenceCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    if (vkCreateFence(GetRenderDevice(), &info, nullptr, &fence)) {
        Util::ErrorPopup("Failed to create fence!");
    }
}

void VkCore::CreateCommandPool() {
    auto graphicsFamilyIndex = GetGraphicsQueueFamilyIndex();
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = graphicsFamilyIndex;
    if (vkCreateCommandPool(GetRenderDevice(), &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
        Util::ErrorPopup("failed to create command pool!");
    }
}
void VkCore::CreateDescriptorPool() {
    VkDescriptorPoolSize poolSizes[] = {{VK_DESCRIPTOR_TYPE_SAMPLER, 20},
                                        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 20},
                                        {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 20},
                                        {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 20},
                                        {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 20},
                                        {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 20},
                                        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 20},
                                        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 20},
                                        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 20},
                                        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 20},
                                        {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 20}};
    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = std::size(poolSizes);
    poolInfo.pPoolSizes = poolSizes;
    poolInfo.maxSets = 100;
    if (vkCreateDescriptorPool(GetRenderDevice(), &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {}
}
}    // namespace Graphics
}    // namespace XRLib
