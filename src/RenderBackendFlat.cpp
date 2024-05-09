#include "RenderBackendFlat.h"
#include "Util.h"

RenderBackendFlat::~RenderBackendFlat() {
    if (core == nullptr || info == nullptr)
        return;
    for (auto imageView : swapChainImageViews) {
        Util::VkSafeClean(vkDestroyImageView, core->GetRenderDevice(),
                          imageView, nullptr);
    }
    Util::VkSafeClean(vkDestroySwapchainKHR, core->GetRenderDevice(), swapChain,
                      nullptr);
    Util::VkSafeClean(vkDestroySurfaceKHR, core->GetRenderInstance(), surface,
                      nullptr);
}

void RenderBackendFlat::Prepare() {
    PrepareFlatWindow();
    CreateFlatSwapChain();
}

void RenderBackendFlat::CreateRenderPass(std::string vertexShaderPath,
                                         std::string fragmentShaderPath) {
    Shader vertexShader{this->core, vertexShaderPath, Shader::VERTEX_SHADER};
    Shader fragmentShader{this->core, fragmentShaderPath, Shader::FRAGMENT_SHADER};

    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = swapChainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    VkRenderPass pass{VK_NULL_HANDLE};
    if (vkCreateRenderPass(core->GetRenderDevice(), &renderPassInfo, nullptr,
                           &pass) != VK_SUCCESS) {
        LOGGER(LOGGER::ERR) << "Failed to create render pass";
        exit(-1);
    }

    Pipeline pipeline{this->core, vertexShader, fragmentShader, swapChainExtent, pass};

    GraphicsRenderPass graphicsPass{pass,vertexShader, fragmentShader, pipeline};
    renderPasses.push_back(graphicsPass);
}

void RenderBackendFlat::CreateFlatSwapChain() {
    VkSurfaceFormatKHR swapChainSurfaceFormat;
    VkPresentModeKHR swapChainPresentMode;

    std::vector<VkSurfaceFormatKHR> surfaceFormats;
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(core->GetRenderPhysicalDevice(),
                                         surface, &formatCount, nullptr);
    surfaceFormats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(core->GetRenderPhysicalDevice(),
                                         surface, &formatCount,
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
    swapChainImageFormat = swapChainSurfaceFormat.format;

    std::vector<VkPresentModeKHR> presentModes;
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(
        core->GetRenderPhysicalDevice(), surface, &presentModeCount, nullptr);
    presentModes.resize(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(core->GetRenderPhysicalDevice(),
                                              surface, &presentModeCount,
                                              presentModes.data());
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
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(core->GetRenderPhysicalDevice(),
                                              surface, &capabilities);
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    swapChainExtent = {std::clamp(static_cast<uint32_t>(width),
                                  capabilities.minImageExtent.width,
                                  capabilities.maxImageExtent.width),
                       std::clamp(static_cast<uint32_t>(height),
                                  capabilities.minImageExtent.height,
                                  capabilities.maxImageExtent.height)};

    uint32_t imageCount = capabilities.minImageCount + 1;
    if (imageCount > capabilities.maxImageCount)
        imageCount = capabilities.maxImageCount;

    VkSwapchainCreateInfoKHR swapchainCreateInfo{};
    swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreateInfo.surface = surface;
    swapchainCreateInfo.minImageCount = imageCount;
    swapchainCreateInfo.imageFormat = swapChainSurfaceFormat.format;
    swapchainCreateInfo.imageColorSpace = swapChainSurfaceFormat.colorSpace;
    swapchainCreateInfo.imageExtent = swapChainExtent;
    swapchainCreateInfo.imageArrayLayers = 1;
    swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchainCreateInfo.preTransform = capabilities.currentTransform;
    swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainCreateInfo.presentMode = swapChainPresentMode;
    swapchainCreateInfo.clipped = VK_TRUE;
    swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;
    if (vkCreateSwapchainKHR(core->GetRenderDevice(), &swapchainCreateInfo,
                             nullptr, &swapChain) != VK_SUCCESS) {
        LOGGER(LOGGER::ERR) << "Failed to create swapchain";
        exit(-1);
    }

    vkGetSwapchainImagesKHR(core->GetRenderDevice(), swapChain, &imageCount,
                            nullptr);
    swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(core->GetRenderDevice(), swapChain, &imageCount,
                            swapChainImages.data());

    swapChainImageViews.resize(swapChainImages.size());
    for (size_t i = 0; i < swapChainImages.size(); ++i) {
        VkImageViewCreateInfo imageViewCreateInfo{};
        imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.image = swapChainImages[i];
        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.format = swapChainImageFormat;
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
        if (vkCreateImageView(core->GetRenderDevice(), &imageViewCreateInfo,
                              nullptr, &swapChainImageViews[i]) != VK_SUCCESS) {
            LOGGER(LOGGER::ERR) << "Failed to create image view";
        }
    }
}

void RenderBackendFlat::PrepareFlatWindow() {
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window = glfwCreateWindow(400, 400, info->applicationName.c_str(), nullptr,
                              nullptr);
    glfwSetWindowUserPointer(window, this);
    if (glfwCreateWindowSurface(core->GetRenderInstance(), window, nullptr,
                                &surface) != VK_SUCCESS) {
        LOGGER(LOGGER::ERR) << "Failed to create window surface";
    }

    VkBool32 presentSupport = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(core->GetRenderPhysicalDevice(),
                                         core->GetGraphicsQueueFamilyIndex(),
                                         surface, &presentSupport);

    if (!presentSupport) {
        LOGGER(LOGGER::WARNING)
            << "Graphics queue doesn't have present support";
        //TODO: graphics queue normaly have present support
    }
}
