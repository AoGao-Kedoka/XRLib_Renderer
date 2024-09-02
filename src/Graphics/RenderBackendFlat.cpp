#include "RenderBackendFlat.h"

RenderBackendFlat::~RenderBackendFlat() {
    if (vkCore == nullptr || info == nullptr)
        return;
    vkDeviceWaitIdle(vkCore->GetRenderDevice());
    for (auto imageView : vkCore->GetSwapchainImageViewsFlat()) {
        VkUtil::VkSafeClean(vkDestroyImageView, vkCore->GetRenderDevice(),
                            imageView, nullptr);
    }
    VkUtil::VkSafeClean(vkDestroySwapchainKHR, vkCore->GetRenderDevice(),
                        vkCore->GetFlatSwapchain(), nullptr);
    VkUtil::VkSafeClean(vkDestroySurfaceKHR, vkCore->GetRenderInstance(),
                        vkCore->GetFlatSurface(), nullptr);
}

void RenderBackendFlat::Prepare(
    std::vector<std::pair<const std::string&, const std::string&>>
        passesToAdd) {
    PrepareFlatWindow();
    CreateFlatSwapChain();
    InitVertexIndexBuffers();

    // prepare shader
    if (passesToAdd.empty()) {
        auto graphicsRenderPass =
            std::make_unique<GraphicsRenderPass>(vkCore, false);
        renderPasses.push_back(std::move(graphicsRenderPass));
    } else {
        for (auto& pass : passesToAdd) {
            auto graphicsRenderPass = std::make_unique<GraphicsRenderPass>(
                vkCore, false, pass.first, pass.second);
            renderPasses.push_back(std::move(graphicsRenderPass));
        }
    }

    InitFrameBuffer();

    // register window resize callback
    EventSystem::Callback<int, int> windowResizeCallback =
        std::bind(&RenderBackendFlat::OnWindowResized, this,
                  std::placeholders::_1, std::placeholders::_2);
    EventSystem::RegisterListener(WindowHandler::XRLIB_EVENT_WINDOW_RESIZED,
                                  windowResizeCallback);
}

void RenderBackendFlat::CreateFlatSwapChain() {
    VkSurfaceFormatKHR swapChainSurfaceFormat;
    VkPresentModeKHR swapChainPresentMode;

    std::vector<VkSurfaceFormatKHR> surfaceFormats;
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(vkCore->GetRenderPhysicalDevice(),
                                         vkCore->GetFlatSurface(), &formatCount,
                                         nullptr);
    surfaceFormats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(vkCore->GetRenderPhysicalDevice(),
                                         vkCore->GetFlatSurface(), &formatCount,
                                         surfaceFormats.data());
    if (surfaceFormats.empty()) {
        Util::ErrorPopup("Failed to get surface formats");
    }
    swapChainSurfaceFormat = surfaceFormats[0];
    for (const auto& availableFormat : surfaceFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
            availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            swapChainSurfaceFormat = availableFormat;
        }
    }
    vkCore->SetFlatSwapchainImageFormat(swapChainSurfaceFormat.format);

    std::vector<VkPresentModeKHR> presentModes;
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(vkCore->GetRenderPhysicalDevice(),
                                              vkCore->GetFlatSurface(),
                                              &presentModeCount, nullptr);
    presentModes.resize(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(
        vkCore->GetRenderPhysicalDevice(), vkCore->GetFlatSurface(),
        &presentModeCount, presentModes.data());
    if (presentModes.empty()) {
        LOGGER(LOGGER::WARNING) << "Cannot create swapchain in flat mode";
    }
    swapChainPresentMode = VK_PRESENT_MODE_FIFO_KHR;
    for (const auto& availablePresentMode : presentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            swapChainPresentMode = availablePresentMode;
        }
    }

    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vkCore->GetRenderPhysicalDevice(),
                                              vkCore->GetFlatSurface(),
                                              &capabilities);
    int width, height;
    std::pair<int&, int&>(width, height) = WindowHandler::GetFrameBufferSize();

    vkCore->SetFlatSwapchainExtent2D(
        {std::clamp(static_cast<uint32_t>(width),
                    capabilities.minImageExtent.width,
                    capabilities.maxImageExtent.width),
         std::clamp(static_cast<uint32_t>(height),
                    capabilities.minImageExtent.height,
                    capabilities.maxImageExtent.height)});

    uint32_t imageCount = capabilities.minImageCount + 1;
    if (imageCount > capabilities.maxImageCount)
        imageCount = capabilities.maxImageCount;

    VkSwapchainCreateInfoKHR swapchainCreateInfo{};
    swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreateInfo.surface = vkCore->GetFlatSurface();
    swapchainCreateInfo.minImageCount = imageCount;
    swapchainCreateInfo.imageFormat = swapChainSurfaceFormat.format;
    swapchainCreateInfo.imageColorSpace = swapChainSurfaceFormat.colorSpace;
    swapchainCreateInfo.imageExtent = vkCore->GetSwapchainExtent(false);
    swapchainCreateInfo.imageArrayLayers = 1;
    swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchainCreateInfo.preTransform = capabilities.currentTransform;
    swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainCreateInfo.presentMode = swapChainPresentMode;
    swapchainCreateInfo.clipped = VK_TRUE;
    swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;
    if (vkCreateSwapchainKHR(vkCore->GetRenderDevice(), &swapchainCreateInfo,
                             nullptr,
                             &vkCore->GetFlatSwapchain()) != VK_SUCCESS) {
        Util::ErrorPopup("Failed to create swapchain");
    }

    vkGetSwapchainImagesKHR(vkCore->GetRenderDevice(),
                            vkCore->GetFlatSwapchain(), &imageCount, nullptr);
    vkCore->GetFlatSwapchainImages().resize(imageCount);
    vkGetSwapchainImagesKHR(vkCore->GetRenderDevice(),
                            vkCore->GetFlatSwapchain(), &imageCount,
                            vkCore->GetFlatSwapchainImages().data());

    vkCore->GetSwapchainImageViewsFlat().resize(
        vkCore->GetFlatSwapchainImages().size());
    for (size_t i = 0; i < vkCore->GetFlatSwapchainImages().size(); ++i) {
        VkImageViewCreateInfo imageViewCreateInfo{};
        imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.image = vkCore->GetFlatSwapchainImages()[i];
        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.format = vkCore->GetFlatSwapchainImageFormat();
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
                &vkCore->GetSwapchainImageViewsFlat()[i]) != VK_SUCCESS) {
            Util::ErrorPopup("Failed to create image view");
        }
    }

    LOGGER(LOGGER::DEBUG) << "Flat Swapchain created";
}

void RenderBackendFlat::OnWindowResized(int width, int height) {
    vkDeviceWaitIdle(vkCore->GetRenderDevice());
    for (auto framebuffer : vkCore->GetSwapchainFrameBuffer()) {
        //TODO: Change to save clean
        vkDestroyFramebuffer(vkCore->GetRenderDevice(), framebuffer, nullptr);
    }

    for (auto imageView : vkCore->GetSwapchainImageViewsFlat()) {
        vkDestroyImageView(vkCore->GetRenderDevice(), imageView, nullptr);
    }

    vkDestroySwapchainKHR(vkCore->GetRenderDevice(), vkCore->GetFlatSwapchain(),
                          nullptr);

    this->CreateFlatSwapChain();
    this->InitFrameBuffer();

    LOGGER(LOGGER::DEBUG) << "Swapchain recreated!";
}

void RenderBackendFlat::PrepareFlatWindow() {
    WindowHandler::VkGetWindowSurface(vkCore->GetRenderInstance(),
                                      &vkCore->GetFlatSurface());

    VkBool32 presentSupport = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(vkCore->GetRenderPhysicalDevice(),
                                         vkCore->GetGraphicsQueueFamilyIndex(),
                                         vkCore->GetFlatSurface(),
                                         &presentSupport);

    if (!presentSupport) {
        LOGGER(LOGGER::WARNING)
            << "Graphics queue doesn't have present support";
        //TODO: graphics queue normaly have present support
    }

    LOGGER(LOGGER::DEBUG) << "Window created";
}
