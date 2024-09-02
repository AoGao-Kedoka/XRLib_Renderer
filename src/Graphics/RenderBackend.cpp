#include "RenderBackend.h"
#include "Utils/Util.h"

namespace XRLib {
namespace Graphics {
RenderBackend::RenderBackend(std::shared_ptr<Info> info,
                             std::shared_ptr<VkCore> vkCore,
                             std::shared_ptr<XRLib::XR::XrCore> xrCore,
                             std::shared_ptr<XRLib::Scene> scene)
    : info{info}, vkCore{vkCore}, xrCore{xrCore}, scene{scene} {
    if (info->validationLayer) {
        for (const char* layer : validataionLayers) {
            bool res = VkUtil::VkCheckLayerSupport(layer);

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
    auto windowExtensions =
        WindowHandler::VkGetWindowExtensions(&requiredExtensionCount);

    for (uint32_t i = 0; i < requiredExtensionCount; ++i) {
        vulkanInstanceExtensions.push_back(windowExtensions[i]);
    }

    if (info->validationLayer) {
        vulkanInstanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    if (xrCore->IsXRValid()) {
        auto xrGetVulkanInstanceExtensionsKHR =
            XRLib::XR::XrUtil::XrGetXRFunction<
                PFN_xrGetVulkanInstanceExtensionsKHR>(
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

        vulkanInstanceExtensions.push_back(
            VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
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
        debugCreateInfo.pfnUserCallback = VkUtil::VkDebugCallback;

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
        auto xrGetVulkanGraphicsDeviceKHR = XRLib::XR::XrUtil::XrGetXRFunction<
            PFN_xrGetVulkanGraphicsDeviceKHR>(xrCore->GetXRInstance(),
                                              "xrGetVulkanGraphicsDeviceKHR");

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
    VkPhysicalDeviceMultiviewFeaturesKHR physicalDeviceMultiviewFeatures{};
    if (xrCore->IsXRValid()) {
        auto xrGetVulkanDeviceExtensionsKHR =
            XRLib::XR::XrUtil::XrGetXRFunction<
                PFN_xrGetVulkanDeviceExtensionsKHR>(
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

        deviceExtensions.push_back(VK_KHR_MULTIVIEW_EXTENSION_NAME);
        physicalDeviceMultiviewFeatures.sType =
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_FEATURES_KHR;
        physicalDeviceMultiviewFeatures.multiview = VK_TRUE;
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
    if (xrCore->IsXRValid()) {
        deviceCreateInfo.pNext = &physicalDeviceMultiviewFeatures;
    }

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
        // create image view
        vkCore->GetStereoSwapchainImages()[i] =
            xrCore->GetSwapchainImages()[i].image;
        VkImageViewCreateInfo imageViewCreateInfo{};
        imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.image = vkCore->GetStereoSwapchainImages()[i];
        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
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
        imageViewCreateInfo.subresourceRange.layerCount = 2;
        if (vkCreateImageView(
                vkCore->GetRenderDevice(), &imageViewCreateInfo, nullptr,
                &vkCore->GetStereoSwapchainImageViews()[i]) != VK_SUCCESS) {
            Util::ErrorPopup("Failed to create image view");
        }
    }

    vkCore->SetStereoSwapchainExtent2D(
        {xrCore->GetXRViewConfigurationView()[0].recommendedImageRectWidth,
         xrCore->GetXRViewConfigurationView()[0].recommendedImageRectHeight});
}

void RenderBackend::InitVertexIndexBuffers() {
    // init vertex buffer and index buffer
    vertexBuffers.resize(scene->Meshes().size());
    indexBuffers.resize(scene->Meshes().size());

    for (int i = 0; i < scene->Meshes().size(); ++i) {
        auto mesh = scene->Meshes()[i];
        void* verticesData = static_cast<void*>(mesh.vertices.data());
        void* indicesData = static_cast<void*>(mesh.indices.data());
        vertexBuffers.push_back(std::make_unique<Buffer>(
            vkCore, sizeof(mesh.vertices[0]) * mesh.vertices.size(),
            VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, verticesData));

        indexBuffers.push_back(std::make_unique<Buffer>(
            vkCore, sizeof(mesh.indices[0]) * mesh.indices.size(),
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VK_MEMORY_HEAP_DEVICE_LOCAL_BIT, indicesData));
    }
}

void RenderBackend::InitFrameBuffer() {
    // create frame buffer
    if (xrCore->IsXRValid()) {
        vkCore->GetSwapchainFrameBuffer().resize(
            vkCore->GetStereoSwapchainImageViews().size());
    } else {
        vkCore->GetSwapchainFrameBuffer().resize(
            vkCore->GetSwapchainImageViewsFlat().size());
    }

    for (size_t i = 0; i < vkCore->GetSwapchainFrameBuffer().size(); i++) {
        std::vector<VkImageView> attachments;
        if (xrCore->IsXRValid()) {
            attachments.push_back(vkCore->GetStereoSwapchainImageViews()[i]);
        } else {
            attachments.push_back(vkCore->GetSwapchainImageViewsFlat()[i]);
        }

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPasses[renderPasses.size() - 1]
                                         ->GetRenderPass()
                                         .GetVkRenderPass();
        framebufferInfo.attachmentCount = attachments.size();
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width =
            vkCore->GetSwapchainExtent(xrCore->IsXRValid()).width;
        framebufferInfo.height =
            vkCore->GetSwapchainExtent(xrCore->IsXRValid()).height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(
                vkCore->GetRenderDevice(), &framebufferInfo, nullptr,
                &vkCore->GetSwapchainFrameBuffer()[i]) != VK_SUCCESS) {
            Util::ErrorPopup("Failed to create frame buffer");
        }
    }
}

void RenderBackend::Run(uint32_t& imageIndex) {
    vkWaitForFences(vkCore->GetRenderDevice(), 1, &vkCore->GetInFlightFence(),
                    VK_TRUE, UINT64_MAX);
    vkResetCommandBuffer(vkCore->GetCommandBuffer(), 0);

    if (!xrCore->IsXRValid()) {
        auto result = vkAcquireNextImageKHR(
            vkCore->GetRenderDevice(), vkCore->GetFlatSwapchain(), UINT64_MAX,
            vkCore->GetImageAvailableSemaphore(), VK_NULL_HANDLE, &imageIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            int width, height;
            std::pair<int&, int&>(width, height) =
                WindowHandler::GetFrameBufferSize();
            EventSystem::TriggerEvent(WindowHandler::XRLIB_EVENT_WINDOW_RESIZED,
                                      width, height);
            return;
        } else if (result != VK_SUCCESS) {
            Util::ErrorPopup("Failed to acquire next image");
        }
    }

    vkResetFences(vkCore->GetRenderDevice(), 1, &vkCore->GetInFlightFence());

    CommandBuffer::Record(vkCore, renderPasses[0], imageIndex);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {vkCore->GetImageAvailableSemaphore()};
    VkPipelineStageFlags waitStages[] = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 0;
    if (!xrCore->IsXRValid()) {
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
    }

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &vkCore->GetCommandBuffer();

    VkSemaphore signalSemaphores[] = {vkCore->GetRenderFinishedSemaphore()};
    if (!xrCore->IsXRValid()) {
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;
    }

    if (vkQueueSubmit(vkCore->GetGraphicsQueue(), 1, &submitInfo,
                      vkCore->GetInFlightFence()) != VK_SUCCESS) {
        Util::ErrorPopup("Failed to submit draw command buffer");
    }

    if (!xrCore->IsXRValid()) {
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
}
}    // namespace Graphics
}    // namespace XRLib
