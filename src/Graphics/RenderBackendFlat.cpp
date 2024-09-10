#include "RenderBackendFlat.h"

namespace XRLib {
namespace Graphics {
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

    // register window resize callback
    EventSystem::Callback<int, int> windowResizeCallback =
        std::bind(&RenderBackendFlat::OnWindowResized, this,
                  std::placeholders::_1, std::placeholders::_2);
    EventSystem::RegisterListener(Events::XRLIB_EVENT_WINDOW_RESIZED,
                                  windowResizeCallback);
    // register key press callback
    EventSystem::Callback<int> keyPressCallback = std::bind(
        &RenderBackendFlat::OnKeyPressed, this, std::placeholders::_1);
    EventSystem::RegisterListener(Events::XRLIB_EVENT_KEY_PRESSED,
                                  keyPressCallback);

    // register mouse movement callback
    EventSystem::Callback<double, double> onMouseCallback =
        std::bind(&RenderBackendFlat::OnMouseMovement, this,
                  std::placeholders::_1, std::placeholders::_2);
    EventSystem::RegisterListener(
        Events::XRLIB_EVENT_MOUSE_RIGHT_MOVEMENT_EVENT, onMouseCallback);

    WindowHandler::ActivateInput();

    depthImage = std::make_unique<Image>(
        vkCore, WindowHandler::GetFrameBufferSize(),
        VkUtil::FindDepthFormat(vkCore->GetRenderPhysicalDevice()),
        VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    // prepare shader
    if (passesToAdd.empty()) {
        // default flat render pass
        viewProj.view = scene->Cam().GetTransform().GetMatrix();
        viewProj.proj = scene->Cam().GetCameraProjection();
        LOGGER(LOGGER::DEBUG) << "View matrix: \n"
                              << glm::to_string(viewProj.view) << "\n"
                              << "Projection matrix \n"
                              << glm::to_string(viewProj.proj);

        // TODO: adapt to multiple objects
        Primitives::ModelPos model;
        model.model = glm::mat4(1);

        auto ViewProjBuffer =
            std::make_shared<Buffer>(vkCore, sizeof(Primitives::ViewProjection),
                                     VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT |
                                         VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                         VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                     static_cast<void*>(&viewProj), false);

        std::vector<DescriptorLayoutElement> layoutElements{
            {ViewProjBuffer},
            {std::make_shared<Buffer>(vkCore, sizeof(Primitives::ModelPos),
                                      VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                      static_cast<void*>(&model), false)},
            {std::make_shared<Image>(vkCore, scene->Meshes()[0].textureData,
                                     scene->Meshes()[0].textureWidth,
                                     scene->Meshes()[0].textureHeight,
                                     scene->Meshes()[0].textureChannels,
                                     VK_FORMAT_R8G8B8A8_SRGB)}};

        std::shared_ptr<DescriptorSet> descriptorSet =
            std::make_shared<DescriptorSet>(vkCore, layoutElements);

        auto graphicsRenderPass =
            std::make_unique<GraphicsRenderPass>(vkCore, false, descriptorSet);

        renderPasses.push_back(std::move(graphicsRenderPass));

        // register buffers updates when events
        EventSystem::Callback<int> bufferOnKeyShouldUpdateCallback =
            [this, ViewProjBuffer](int keyCode) {
                viewProj.view = scene->Cam().GetTransform().GetMatrix();
                ViewProjBuffer->UpdateBuffer(sizeof(Primitives::ViewProjection),
                                             static_cast<void*>(&viewProj));
            };

        EventSystem::RegisterListener(Events::XRLIB_EVENT_KEY_PRESSED,
                                      bufferOnKeyShouldUpdateCallback);

        EventSystem::Callback<double, double>
            bufferOnMouseShouldUpdateCallback =
                [this, ViewProjBuffer](double deltaX, double deltaY) {
                    viewProj.view = scene->Cam().GetTransform().GetMatrix();
                    ViewProjBuffer->UpdateBuffer(
                        sizeof(Primitives::ViewProjection),
                        static_cast<void*>(&viewProj));
                };
        EventSystem::RegisterListener(
            Events::XRLIB_EVENT_MOUSE_RIGHT_MOVEMENT_EVENT,
            bufferOnMouseShouldUpdateCallback);
    } else {
        for (auto& pass : passesToAdd) {
            auto graphicsRenderPass = std::make_unique<GraphicsRenderPass>(
                vkCore, false, nullptr, pass.first, pass.second);
            renderPasses.push_back(std::move(graphicsRenderPass));
        }
    }

    InitFrameBuffer();
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
    auto [width, height] = WindowHandler::GetFrameBufferSize();

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

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Handling events
//////////////////////////////////////////////////////////////////////////////////////////////////////////

void RenderBackendFlat::OnMouseMovement(double deltaX, double deltaY) {
    float sensitivity = 0.01f;
    float xoffset = deltaX * sensitivity;
    float yoffset = deltaY * sensitivity;

    auto cameraUp = scene->Cam().UpVector();
    auto cameraFront = scene->Cam().FrontVector();

    glm::quat quatX = glm::angleAxis(glm::radians(xoffset), cameraUp);
    glm::quat quatY = glm::angleAxis(glm::radians(yoffset),
                                     glm::cross(cameraFront, cameraUp));

    glm::quat rotation = quatX * quatY;

    scene->Cam().SetCameraFront(glm::normalize(rotation * cameraFront));
    scene->Cam().SetCameraUp(glm::normalize(
        glm::cross(glm::cross(cameraFront, cameraUp), cameraFront)));
}

void RenderBackendFlat::OnKeyPressed(int keyCode) {
    float movementSensitivity = 0.01;
    auto& cam = scene->Cam();
    if (keyCode == GLFW_KEY_W) {
        cam.SetCameraPos(cam.TranslationVector() +
                         cam.FrontVector() *
                             movementSensitivity);    // TODO: change to front
    }
    if (keyCode == GLFW_KEY_S) {
        cam.SetCameraPos(cam.TranslationVector() +
                         cam.BackVector() * movementSensitivity);
    }
    if (keyCode == GLFW_KEY_A) {
        cam.SetCameraPos(cam.TranslationVector() +
                         cam.LeftVector() * movementSensitivity);
    }
    if (keyCode == GLFW_KEY_D) {
        cam.SetCameraPos(cam.TranslationVector() +
                         cam.RightVector() * movementSensitivity);
    }
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

    depthImage = std::make_unique<Image>(
        vkCore, WindowHandler::GetFrameBufferSize(),
        VkUtil::FindDepthFormat(vkCore->GetRenderPhysicalDevice()),
        VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    this->CreateFlatSwapChain();
    this->InitFrameBuffer();

    LOGGER(LOGGER::DEBUG) << "Window resized";
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
}    // namespace Graphics
}    // namespace XRLib
