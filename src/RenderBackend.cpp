#include "RenderBackend.h"
#include "Utils/Util.h"

RenderBackend::RenderBackend(std::shared_ptr<Info> info,
                             std::shared_ptr<VkCore> vkCore,
                             std::shared_ptr<XrCore> xrCore,
                             std::shared_ptr<Scene> scene)
    : info{info}, vkCore{vkCore}, xrCore{xrCore}, scene{scene} {
    glfwInit();

    if (info->validationLayer) {
        for (const char* layer : validataionLayers) {
            bool res = Util::VkCheckLayerSupport(layer);

            // disable validataion if validation layer is not supported
            if (!res) {
                info->validationLayer = false;
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

    vkDeviceWaitIdle(vkCore->GetRenderDevice());
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
        Util::ErrorPopup("Error getting glfw instance extension");
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
            Util::ErrorPopup("Failed to get vulkan instance extension count");
        }

        std::string buffer(xrVulkanInstanceExtensionsCount, ' ');
        if (xrGetVulkanInstanceExtensionsKHR(xrCore->GetXRInstance(),
                                             xrCore->GetSystemID(),
                                             xrVulkanInstanceExtensionsCount,
                                             &xrVulkanInstanceExtensionsCount,
                                             buffer.data()) != XR_SUCCESS) {
            Util::ErrorPopup("Failed to get vulkan instance extension");
        }

        std::vector<const char*> xrInstanceExtensions =
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
        Util::ErrorPopup("Failed to create vulkan instance");
    }

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

    // select physical device
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(vkCore->GetRenderInstance(), &deviceCount, 0);
    if (deviceCount == 0) {
        Util::ErrorPopup("Failed to find GPUs with Vulkan support");
    }
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(vkCore->GetRenderInstance(), &deviceCount,
                               devices.data());

    if (xrCore->IsXRValid()) {
        auto xrGetVulkanGraphicsDeviceKHR =
            Util::XrGetXRFunction<PFN_xrGetVulkanGraphicsDeviceKHR>(
                xrCore->GetXRInstance(), "xrGetVulkanGraphicsDeviceKHR");

        if (xrGetVulkanGraphicsDeviceKHR(
                xrCore->GetXRInstance(), xrCore->GetSystemID(),
                vkCore->GetRenderInstance(),
                &vkCore->GetRenderPhysicalDevice()) != XR_SUCCESS) {
            Util::ErrorPopup("Failed to get vulkan graphics device");
        }
    } else {
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
            Util::ErrorPopup("Failed to find suitable GPU");
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
            Util::ErrorPopup("Failed to get vulkan device extensions");
        }
        std::string buffer(deviceExtensionsCount, ' ');
        if (xrGetVulkanDeviceExtensionsKHR(
                xrCore->GetXRInstance(), xrCore->GetSystemID(),
                deviceExtensionsCount, &deviceExtensionsCount, buffer.data())) {
            Util::ErrorPopup("Failed to get vulkan device extensions");
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
        Util::ErrorPopup("Failed to create vulkan device.");
    }

    vkGetDeviceQueue(vkCore->GetRenderDevice(),
                     vkCore->GetGraphicsQueueFamilyIndex(), 0,
                     &vkCore->GetGraphicsQueue());
}

void RenderBackend::Prepare(
    std::vector<std::pair<const std::string&, const std::string&>>
        passesToAdd) {
    InitVertexIndexBuffers();
    GetSwapchainInfo();
    if (passesToAdd.empty()) {
        auto graphicsRenderPass =
            std::make_unique<GraphicsRenderPass>(vkCore, true);
        renderPasses.push_back(std::move(graphicsRenderPass));
    } else {
        for (auto& pass : passesToAdd) {
            auto graphicsRenderPass = std::make_unique<GraphicsRenderPass>(
                vkCore, true, pass.first, pass.second);
            renderPasses.push_back(std::move(graphicsRenderPass));
        }
    }
    InitFrameBuffer();
}

void RenderBackend::GetSwapchainInfo() {
    uint8_t swapchainImageCount = xrCore->GetSwapchainImages().size();
    vkCore->GetStereoSwapchainImages().resize(swapchainImageCount);
    vkCore->GetStereoSwapchainImageViews().resize(swapchainImageCount);
    for (uint32_t i = 0; i < xrCore->GetSwapchainImages().size(); ++i) {
        vkCore->GetStereoSwapchainImages()[i] =
            xrCore->GetSwapchainImages()[i].image;
        VkImageViewCreateInfo imageViewCreateInfo{};
        imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.image = vkCore->GetStereoSwapchainImages()[i];
        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.format = vkCore->GetStereoSwapchainImageFormat();
        imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.subresourceRange.aspectMask =
            VK_IMAGE_ASPECT_COLOR_BIT;
        imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
        imageViewCreateInfo.subresourceRange.levelCount = 1;
        imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        imageViewCreateInfo.subresourceRange.layerCount = 1;
        if (vkCreateImageView(
                vkCore->GetRenderDevice(), &imageViewCreateInfo, nullptr,
                &vkCore->GetStereoSwapchainImageViews()[i]) != VK_SUCCESS) {
            Util::ErrorPopup("Failed to create image view");
        }
    }
}

void RenderBackend::InitVertexIndexBuffers() {
    // init vertex buffer and index buffer
    vertexBuffers.resize(scene->Meshes().size());
    indexBuffers.resize(scene->Meshes().size());

    for (int i = 0; i < scene->Meshes().size(); ++i) {
        auto mesh = scene->Meshes()[i];
        vertexBuffers.push_back(std::make_unique<Buffer>(
            vkCore, sizeof(mesh.vertices[0]) * mesh.vertices.size(),
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));
        indexBuffers.push_back(std::make_unique<Buffer>(
            vkCore, sizeof(mesh.indices[0]) * mesh.indices.size(),
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));
    }
}

void RenderBackend::InitFrameBuffer() {
    // create frame buffer
    vkCore->GetSwapchainFrameBufferFlat().resize(
        vkCore->GetSwapchainImageViewsFlat().size());

    for (size_t i = 0; i < vkCore->GetSwapchainImageViewsFlat().size(); i++) {
        VkImageView attachments[] = {vkCore->GetSwapchainImageViewsFlat()[i]};

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass =
            renderPasses[renderPasses.size() - 1]->renderPass->GetRenderPass();
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = vkCore->GetFlatSwapchainExtent2D().width;
        framebufferInfo.height = vkCore->GetFlatSwapchainExtent2D().height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(
                vkCore->GetRenderDevice(), &framebufferInfo, nullptr,
                &vkCore->GetSwapchainFrameBufferFlat()[i]) != VK_SUCCESS) {
            Util::ErrorPopup("Failed to create frame buffer");
        }
    }
}

void RenderBackend::Run() {
    glfwPollEvents();
    vkWaitForFences(vkCore->GetRenderDevice(), 1, &vkCore->GetInFlightFence(),
                    VK_TRUE, UINT64_MAX);
    vkResetCommandBuffer(vkCore->GetCommandBuffer(), 0);

    uint32_t imageIndex;
    auto result = vkAcquireNextImageKHR(
        vkCore->GetRenderDevice(), vkCore->GetFlatSwapchain(), UINT64_MAX,
        vkCore->GetImageAvailableSemaphore(), VK_NULL_HANDLE, &imageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        OnWindowResized();
        return;
    } else if (result != VK_SUCCESS) {
        Util::ErrorPopup("Failed to acquire next image");
    }

    vkResetFences(vkCore->GetRenderDevice(), 1, &vkCore->GetInFlightFence());

    renderPasses.at(0)->renderPass->Record(imageIndex);

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
        Util::ErrorPopup("Failed to submit draw command buffer");
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
