#include "RenderBackend.h"
#include "Util.h"

RenderBackend::RenderBackend(Info& info, Core& core)
    : info{&info}, core{&core} {
    glfwInit();

    if (info.validationLayer) {
        for (const char* layer : validataionLayers) {
            bool res = Util::VkCheckLayerSupport(layer);

            // disable validataion if validation layer is not supported
            if (!res) {
                info.validationLayer = false;
                LOGGER(LOGGER::WARNING) << "Validation layer not available, "
                                           "disabling validataion layer";
            }
        }
    }
    CreateVulkanInstance();
    CreatePhysicalDevice();
    CreateLogicalDevice();
}

RenderBackend::~RenderBackend() {
    if (!core || !info)
        return;
    if (vkDebugMessenger != VK_NULL_HANDLE) {
        PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT =
            reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
                vkGetInstanceProcAddr(core->GetRenderInstance(),
                                      "vkDestroyDebugUtilsMessengerEXT"));
        vkDestroyDebugUtilsMessengerEXT(core->GetRenderInstance(),
                                        vkDebugMessenger, nullptr);
    }

    for (auto& renderpass : renderPasses) {
        Util::VkSafeClean(vkDestroyRenderPass, core->GetRenderDevice(),
                          renderpass.renderPass, nullptr);
    }
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
        auto xrGetVulkanInstanceExtensionsKHR =
           Util::XrGetXRFunction<PFN_xrGetVulkanInstanceExtensionsKHR>(
                core->GetXRInstance(), "xrGetVulkanInstanceExtensionsKHR");

        uint32_t xrVulkanInstanceExtensionsCount;
        if (xrGetVulkanInstanceExtensionsKHR(
                core->GetXRInstance(), core->GetSystemID(), 0,
                &xrVulkanInstanceExtensionsCount, nullptr) != XR_SUCCESS) {
            LOGGER(LOGGER::ERR)
                << "Failed to get vulkan instance extension count";
            exit(-1);
        }

        std::string buffer(xrVulkanInstanceExtensionsCount, ' ');
        if (xrGetVulkanInstanceExtensionsKHR(core->GetXRInstance(),
                                             core->GetSystemID(), 0,
                                             &xrVulkanInstanceExtensionsCount,
                                             buffer.data()) != XR_SUCCESS) {
            LOGGER(LOGGER::ERR) << "Failed to get vulkan instance extension";
            exit(-1);
        }

        //vulkanInstanceExtensions.push_back(buffer.c_str());
    }

    VkInstanceCreateInfo createInfo{};
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &applicationInfo;
    createInfo.enabledExtensionCount =
        static_cast<uint32_t>(vulkanInstanceExtensions.size());
    createInfo.ppEnabledExtensionNames = vulkanInstanceExtensions.data();

    if (info->validationLayer) {
        debugCreateInfo.sType =
            VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debugCreateInfo.messageSeverity =
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debugCreateInfo.messageType =
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        debugCreateInfo.pfnUserCallback = Util::VkDebugCallback;

        createInfo.enabledLayerCount =
            static_cast<uint32_t>(validataionLayers.size());
        createInfo.ppEnabledLayerNames = validataionLayers.data();
        createInfo.pNext =
            (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
    } else {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;
    }

    if (vkCreateInstance(&createInfo, nullptr, &core->GetRenderInstance()) !=
        VK_SUCCESS) {
        LOGGER(LOGGER::ERR) << "Failed to create vulkan instance";
        exit(-1);
    }

    if (info->validationLayer) {
        vkCreateDebugUtilsMessengerEXT =
            reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
                vkGetInstanceProcAddr(this->core->GetRenderInstance(),
                                      "vkCreateDebugUtilsMessengerEXT"));
        if (vkCreateDebugUtilsMessengerEXT(this->core->GetRenderInstance(),
                                           &debugCreateInfo, nullptr,
                                           &vkDebugMessenger) != VK_SUCCESS) {
            LOGGER(LOGGER::WARNING) << "Failed to create debug utils messenger";
        }
    }
}

void RenderBackend::CreatePhysicalDevice() {
    if (core->IsXRValid()) {
        auto xrGetVulkanGraphicsDeviceKHR =
            Util::XrGetXRFunction<PFN_xrGetVulkanGraphicsDeviceKHR>(
                core->GetXRInstance(), "xrGetVulkanGraphicsDeviceKHR");

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

void RenderBackend::CreateLogicalDevice() {
    std::vector<const char*> deviceExtensions(0);
    if (core->IsXRValid()) {
        auto xrGetVulkanDeviceExtensionsKHR =
            Util::XrGetXRFunction<PFN_xrGetVulkanDeviceExtensionsKHR>(
                core->GetXRInstance(), "xrGetVulkanDeviceExtensionsKHR");

        uint32_t deviceExtensionsCount;
        if (xrGetVulkanDeviceExtensionsKHR(
                core->GetXRInstance(), core->GetSystemID(), 0,
                &deviceExtensionsCount, nullptr) != XR_SUCCESS) {
            LOGGER(LOGGER::ERR) << "Failed to get vulkan device extensions";
            exit(-1);
        }
        std::string buffer;
        buffer.resize(deviceExtensionsCount);
        if (xrGetVulkanDeviceExtensionsKHR(
                core->GetXRInstance(), core->GetSystemID(),
                deviceExtensionsCount, &deviceExtensionsCount, buffer.data())) {
            LOGGER(LOGGER::ERR) << "Failed to get vulkan device extensions";
            exit(-1);
        }
        deviceExtensions = core->UnpackExtensionString(buffer);
    }

    deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    float queuePriority = 1;

    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = core->GetGraphicsQueueFamilyIndex();
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = 1;
    createInfo.pQueueCreateInfos = &queueCreateInfo;
    createInfo.enabledExtensionCount =
        static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    if (vkCreateDevice(core->GetRenderPhysicalDevice(), &createInfo, nullptr,
                       &core->GetRenderDevice()) != VK_SUCCESS) {
        LOGGER(LOGGER::ERR) << "Failed to create vulkan device.";
        exit(-1);
    }

    vkGetDeviceQueue(core->GetRenderDevice(),
                     core->GetGraphicsQueueFamilyIndex(), 0,
                     &core->GetGraphicsQueue());
}
