#include "RenderBackend.h"
#include "Util.h"
#include <stdint.h>
#include <vulkan/vulkan_core.h>

RenderBackend::RenderBackend(Info& info, VkCore& vkCore, XrCore& xrCore)
    : info{&info}, vkCore{&vkCore}, xrCore{&xrCore} {
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
    InitVulkan();
}

RenderBackend::~RenderBackend() {
    if (!vkCore || !info)
        return;
    if (vkDebugMessenger != VK_NULL_HANDLE) {
        PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT =
            reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
                vkGetInstanceProcAddr(vkCore->GetRenderInstance(),
                                      "vkDestroyDebugUtilsMessengerEXT"));
        vkDestroyDebugUtilsMessengerEXT(vkCore->GetRenderInstance(),
                                        vkDebugMessenger, nullptr);
    }
}

void RenderBackend::InitVulkan() {

    // create vulkan instance
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

    if (xrCore->IsXRValid()) {
        auto xrGetVulkanInstanceExtensionsKHR =
            Util::XrGetXRFunction<PFN_xrGetVulkanInstanceExtensionsKHR>(
                xrCore->GetXRInstance(), "xrGetVulkanInstanceExtensionsKHR");

        uint32_t xrVulkanInstanceExtensionsCount;
        if (xrGetVulkanInstanceExtensionsKHR(
                xrCore->GetXRInstance(), xrCore->GetSystemID(), 0,
                &xrVulkanInstanceExtensionsCount, nullptr) != XR_SUCCESS) {
            LOGGER(LOGGER::ERR)
                << "Failed to get vulkan instance extension count";
            exit(-1);
        }

        std::string buffer(xrVulkanInstanceExtensionsCount, ' ');
        if (xrGetVulkanInstanceExtensionsKHR(xrCore->GetXRInstance(),
                                             xrCore->GetSystemID(),
                                             xrVulkanInstanceExtensionsCount,
                                             &xrVulkanInstanceExtensionsCount,
                                             buffer.data()) != XR_SUCCESS) {
            LOGGER(LOGGER::ERR) << "Failed to get vulkan instance extension";
            exit(-1);
        }

        std::vector<const char*> xrInstanceExtensions =
            Util::SplitStringToCharPtr(buffer);
        Util::SplitStringToCharPtr(buffer);
        for (auto extension : xrInstanceExtensions) {
            vulkanInstanceExtensions.push_back(extension);
        }
    }

    VkInstanceCreateInfo instanceCreateInfo{};
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pApplicationInfo = &applicationInfo;
    instanceCreateInfo.enabledExtensionCount =
        static_cast<uint32_t>(vulkanInstanceExtensions.size());
    instanceCreateInfo.ppEnabledExtensionNames =
        vulkanInstanceExtensions.data();

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

        instanceCreateInfo.enabledLayerCount =
            static_cast<uint32_t>(validataionLayers.size());
        instanceCreateInfo.ppEnabledLayerNames = validataionLayers.data();
        instanceCreateInfo.pNext =
            (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
    } else {
        instanceCreateInfo.enabledLayerCount = 0;
        instanceCreateInfo.pNext = nullptr;
    }

    if (vkCreateInstance(&instanceCreateInfo, nullptr,
                         &vkCore->GetRenderInstance()) != VK_SUCCESS) {
        LOGGER(LOGGER::ERR) << "Failed to create vulkan instance";
        exit(-1);
    }

    // select physical device
    if (info->validationLayer) {
        vkCreateDebugUtilsMessengerEXT =
            reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
                vkGetInstanceProcAddr(this->vkCore->GetRenderInstance(),
                                      "vkCreateDebugUtilsMessengerEXT"));
        if (vkCreateDebugUtilsMessengerEXT(this->vkCore->GetRenderInstance(),
                                           &debugCreateInfo, nullptr,
                                           &vkDebugMessenger) != VK_SUCCESS) {
            LOGGER(LOGGER::WARNING) << "Failed to create debug utils messenger";
        }
    }
    if (xrCore->IsXRValid()) {
        auto xrGetVulkanGraphicsDeviceKHR =
            Util::XrGetXRFunction<PFN_xrGetVulkanGraphicsDeviceKHR>(
                xrCore->GetXRInstance(), "xrGetVulkanGraphicsDeviceKHR");

        if (xrGetVulkanGraphicsDeviceKHR(
                xrCore->GetXRInstance(), xrCore->GetSystemID(),
                vkCore->GetRenderInstance(),
                &vkCore->GetRenderPhysicalDevice()) != XR_SUCCESS) {
            LOGGER(LOGGER::ERR) << "Failed to get vulkan graphics device";
            exit(-1);
        }
    } else {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(vkCore->GetRenderInstance(), &deviceCount,
                                   0);
        if (deviceCount == 0) {
            LOGGER(LOGGER::ERR) << "Failed to find GPUs with Vulkan support";
            exit(-1);
        }
        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(vkCore->GetRenderInstance(), &deviceCount,
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
        vkCore->GetRenderPhysicalDevice() = device.first;

        if (vkCore->GetRenderPhysicalDevice() == VK_NULL_HANDLE) {
            LOGGER(LOGGER::ERR) << "Failed to find suitable GPU";
            exit(-1);
        }
    }

    // create logical device
    std::vector<const char*> deviceExtensions(0);
    if (xrCore->IsXRValid()) {
        auto xrGetVulkanDeviceExtensionsKHR =
            Util::XrGetXRFunction<PFN_xrGetVulkanDeviceExtensionsKHR>(
                xrCore->GetXRInstance(), "xrGetVulkanDeviceExtensionsKHR");

        uint32_t deviceExtensionsCount;
        if (xrGetVulkanDeviceExtensionsKHR(
                xrCore->GetXRInstance(), xrCore->GetSystemID(), 0,
                &deviceExtensionsCount, nullptr) != XR_SUCCESS) {
            LOGGER(LOGGER::ERR) << "Failed to get vulkan device extensions";
            exit(-1);
        }
        std::string buffer(deviceExtensionsCount, ' ');
        if (xrGetVulkanDeviceExtensionsKHR(
                xrCore->GetXRInstance(), xrCore->GetSystemID(),
                deviceExtensionsCount, &deviceExtensionsCount, buffer.data())) {
            LOGGER(LOGGER::ERR) << "Failed to get vulkan device extensions";
            exit(-1);
        }
        deviceExtensions = Util::SplitStringToCharPtr(buffer);
    }

    deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    float queuePriority = 1;

    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = vkCore->GetGraphicsQueueFamilyIndex();
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    VkDeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
    deviceCreateInfo.enabledExtensionCount =
        static_cast<uint32_t>(deviceExtensions.size());
    deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();

    if (vkCreateDevice(vkCore->GetRenderPhysicalDevice(), &deviceCreateInfo,
                       nullptr, &vkCore->GetRenderDevice()) != VK_SUCCESS) {
        LOGGER(LOGGER::ERR) << "Failed to create vulkan device.";
    }

    vkGetDeviceQueue(vkCore->GetRenderDevice(),
                     vkCore->GetGraphicsQueueFamilyIndex(), 0,
                     &vkCore->GetGraphicsQueue());
}

void RenderBackend::Run() {
    glfwPollEvents();
    vkWaitForFences(vkCore->GetRenderDevice(), 1, &vkCore->GetInFlightFence(),
                    VK_TRUE, UINT64_MAX);
    vkResetFences(vkCore->GetRenderDevice(), 1, &vkCore->GetInFlightFence());
    vkResetCommandBuffer(vkCore->GetCommandBuffer(), 0);

    uint32_t imageIndex;
    auto err = vkAcquireNextImageKHR(
        vkCore->GetRenderDevice(), vkCore->GetFlatSwapchain(), UINT64_MAX,
        vkCore->GetImageAvailableSemaphore(), VK_NULL_HANDLE, &imageIndex);
    if (err !=VK_SUCCESS) {
        LOGGER(LOGGER::ERR) << "Failed to acquire next image";
        exit(-1);
    }

    renderPasses.at(0)->renderPass.Record(imageIndex);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {vkCore->GetImageAvailableSemaphore()};
    VkPipelineStageFlags waitStages[] = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &vkCore->GetCommandBuffer();

    VkSemaphore signalSemaphores[] = {vkCore->GetRenderFinishedSemaphore()};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(vkCore->GetGraphicsQueue(), 1, &submitInfo,
                      vkCore->GetInFlightFence()) != VK_SUCCESS) {
        LOGGER(LOGGER::ERR) << "Failed to submit draw command buffer";
    }
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {vkCore->GetFlatSwapchain()};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;

    vkQueuePresentKHR(vkCore->GetGraphicsQueue(), &presentInfo);
}
