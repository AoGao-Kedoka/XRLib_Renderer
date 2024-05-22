#include "RenderBackendFlat.h"
#include "Util.h"

RenderBackendFlat::~RenderBackendFlat() {
    if (core == nullptr || info == nullptr)
        return;
    for (auto imageView : core->GetSwapchainImageViewsFlat()) {
        Util::VkSafeClean(vkDestroyImageView, core->GetRenderDevice(),
                          imageView, nullptr);
    }
    Util::VkSafeClean(vkDestroySwapchainKHR, core->GetRenderDevice(),
                      core->GetFlatSwapchain(), nullptr);
    Util::VkSafeClean(vkDestroySurfaceKHR, core->GetRenderInstance(),
                      core->GetFlatSurface(), nullptr);
}

void RenderBackendFlat::Prepare(
    std::vector<std::pair<std::string, std::string>> passesToAdd) {
    PrepareFlatWindow();
    CreateFlatSwapChain();

    // prepare shader
    for (auto& pass : passesToAdd) {
        auto graphicsRenderPass =
            std::make_unique<GraphicsRenderPass>(core, pass.first, pass.second);
        renderPasses.push_back(std::move(graphicsRenderPass));
    }

    // create frame buffer
    core->GetSwapchainFrameBufferFlat().resize(
        core->GetSwapchainImageViewsFlat().size());

    for (size_t i = 0; i < core->GetSwapchainImageViewsFlat().size(); i++) {
        VkImageView attachments[] = {core->GetSwapchainImageViewsFlat()[i]};

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass =
            renderPasses[renderPasses.size() - 1]->renderPass.GetRenderPass();
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = core->GetFlatSwapchainExtent2D().width;
        framebufferInfo.height = core->GetFlatSwapchainExtent2D().height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(
                core->GetRenderDevice(), &framebufferInfo, nullptr,
                &core->GetSwapchainFrameBufferFlat()[i]) != VK_SUCCESS) {
            LOGGER(LOGGER::ERR) << "Failed to create frame buffer";
        }
    }
}

void RenderBackendFlat::CreateFlatSwapChain() {
    VkSurfaceFormatKHR swapChainSurfaceFormat;
    VkPresentModeKHR swapChainPresentMode;

    std::vector<VkSurfaceFormatKHR> surfaceFormats;
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(core->GetRenderPhysicalDevice(),
                                         core->GetFlatSurface(), &formatCount,
                                         nullptr);
    surfaceFormats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(core->GetRenderPhysicalDevice(),
                                         core->GetFlatSurface(), &formatCount,
                                         surfaceFormats.data());
    if (surfaceFormats.empty()) {
        LOGGER(LOGGER::ERR) << "Failed to get surface formats";
        exit(-1);
    }
    swapChainSurfaceFormat = surfaceFormats[0];
    for (const auto& availableFormat : surfaceFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
            availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            swapChainSurfaceFormat = availableFormat;
        }
    }
    core->SetFlatSwapchainImageFormat(swapChainSurfaceFormat.format);

    std::vector<VkPresentModeKHR> presentModes;
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(core->GetRenderPhysicalDevice(),
                                              core->GetFlatSurface(),
                                              &presentModeCount, nullptr);
    presentModes.resize(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(
        core->GetRenderPhysicalDevice(), core->GetFlatSurface(),
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
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        core->GetRenderPhysicalDevice(), core->GetFlatSurface(), &capabilities);
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    core->SetFlatSwapchainExtent2D(
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
    swapchainCreateInfo.surface = core->GetFlatSurface();
    swapchainCreateInfo.minImageCount = imageCount;
    swapchainCreateInfo.imageFormat = swapChainSurfaceFormat.format;
    swapchainCreateInfo.imageColorSpace = swapChainSurfaceFormat.colorSpace;
    swapchainCreateInfo.imageExtent = core->GetFlatSwapchainExtent2D();
    swapchainCreateInfo.imageArrayLayers = 1;
    swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchainCreateInfo.preTransform = capabilities.currentTransform;
    swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainCreateInfo.presentMode = swapChainPresentMode;
    swapchainCreateInfo.clipped = VK_TRUE;
    swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;
    if (vkCreateSwapchainKHR(core->GetRenderDevice(), &swapchainCreateInfo,
                             nullptr,
                             &core->GetFlatSwapchain()) != VK_SUCCESS) {
        LOGGER(LOGGER::ERR) << "Failed to create swapchain";
        exit(-1);
    }

    vkGetSwapchainImagesKHR(core->GetRenderDevice(), core->GetFlatSwapchain(),
                            &imageCount, nullptr);
    core->GetFlatSwapchainImages().resize(imageCount);
    vkGetSwapchainImagesKHR(core->GetRenderDevice(), core->GetFlatSwapchain(),
                            &imageCount, core->GetFlatSwapchainImages().data());

    core->GetSwapchainImageViewsFlat().resize(
        core->GetFlatSwapchainImages().size());
    for (size_t i = 0; i < core->GetFlatSwapchainImages().size(); ++i) {
        VkImageViewCreateInfo imageViewCreateInfo{};
        imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.image = core->GetFlatSwapchainImages()[i];
        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.format = core->GetFlatSwapchainImageFormat();
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
                core->GetRenderDevice(), &imageViewCreateInfo, nullptr,
                &core->GetSwapchainImageViewsFlat()[i]) != VK_SUCCESS) {
            LOGGER(LOGGER::ERR) << "Failed to create image view";
        }
    }

    LOGGER(LOGGER::DEBUG) << "Flat Swapchain created";
}

void RenderBackendFlat::PrepareFlatWindow() {
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window = glfwCreateWindow(400, 400, info->applicationName.c_str(), nullptr,
                              nullptr);
    glfwSetWindowUserPointer(window, this);
    if (glfwCreateWindowSurface(core->GetRenderInstance(), window, nullptr,
                                &core->GetFlatSurface()) != VK_SUCCESS) {
        LOGGER(LOGGER::ERR) << "Failed to create window surface";
    }

    VkBool32 presentSupport = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(
        core->GetRenderPhysicalDevice(), core->GetGraphicsQueueFamilyIndex(),
        core->GetFlatSurface(), &presentSupport);

    if (!presentSupport) {
        LOGGER(LOGGER::WARNING)
            << "Graphics queue doesn't have present support";
        //TODO: graphics queue normaly have present support
    }

    LOGGER(LOGGER::DEBUG) << "Window created";
}
