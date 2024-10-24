#include "Swapchain.h"

namespace XRLib {
namespace Graphics {
Swapchain::Swapchain(std::shared_ptr<VkCore> core) : core{core} {
    CreateSwapchain();
}

void Swapchain::CreateSwapchain() {
    VkSurfaceCapabilitiesKHR capabilities = GetSurfaceCapabilities();
    VkSurfaceFormatKHR surfaceFormat = ChooseSwapchainImageFormat();
    VkPresentModeKHR presentMode = ChooseSwapchainPresentMode();
    uint32_t imageCount = std::min(capabilities.minImageCount + 1, capabilities.maxImageCount);

    VkSwapchainCreateInfoKHR swapchainCreateInfo{};
    swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreateInfo.surface = core->GetFlatSurface();
    swapchainCreateInfo.minImageCount = imageCount;
    swapchainCreateInfo.imageFormat = surfaceFormat.format;
    swapchainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
    swapchainCreateInfo.imageExtent = swapchainExtent;
    swapchainCreateInfo.imageArrayLayers = 1;
    swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchainCreateInfo.preTransform = capabilities.currentTransform;
    swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainCreateInfo.presentMode = presentMode;
    swapchainCreateInfo.clipped = VK_TRUE;
    swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;
    if (vkCreateSwapchainKHR(core->GetRenderDevice(), &swapchainCreateInfo, nullptr, &swapchain) != VK_SUCCESS) {
        Util::ErrorPopup("Failed to create swapchain");
    }
}

void Swapchain::GetSwapchainImages() {
    uint32_t imageCount;
    std::vector<VkImage> swapchainRawImages;
    vkGetSwapchainImagesKHR(core->GetRenderDevice(), swapchain, &imageCount, nullptr);
    vkGetSwapchainImagesKHR(core->GetRenderDevice(), swapchain, &imageCount, swapchainRawImages.data());
    for (const auto image : swapchainRawImages) {
        swapchainImages.push_back(std::make_unique<Image>(core, image));
    }
}

VkSurfaceFormatKHR Swapchain::ChooseSwapchainImageFormat() {
    VkSurfaceFormatKHR swapchainSurfaceFormat;
    std::vector<VkSurfaceFormatKHR> surfaceFormats;
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(core->GetRenderPhysicalDevice(), core->GetFlatSurface(), &formatCount,
                                         nullptr);
    surfaceFormats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(core->GetRenderPhysicalDevice(), core->GetFlatSurface(), &formatCount,
                                         surfaceFormats.data());
    if (surfaceFormats.empty()) {
        Util::ErrorPopup("Failed to get surface formats");
    }

    swapchainSurfaceFormat = surfaceFormats[0];
    for (const auto& availableFormat : surfaceFormats) {
        if (availableFormat.format == VK_FORMAT_R8G8B8A8_SRGB &&
            availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            swapchainSurfaceFormat = availableFormat;
        }
    }
    swapchainImageFormat = swapchainSurfaceFormat.format;
    return swapchainSurfaceFormat;
}

VkPresentModeKHR Swapchain::ChooseSwapchainPresentMode() {
    VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;
    std::vector<VkPresentModeKHR> presentModes;
    uint32_t presentModesCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(core->GetRenderPhysicalDevice(), core->GetFlatSurface(),
                                              &presentModesCount, nullptr);
    presentModes.resize(presentModesCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(core->GetRenderPhysicalDevice(), core->GetFlatSurface(),
                                              &presentModesCount, presentModes.data());
    if (presentModes.empty()) {
        LOGGER(LOGGER::WARNING) << "Failed to get present mode";
    }

    for (const auto& availablePresentMode : presentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            swapchainPresentMode = availablePresentMode;
        }
    }
    return swapchainPresentMode;
}

VkSurfaceCapabilitiesKHR Swapchain::GetSurfaceCapabilities() {
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(core->GetRenderPhysicalDevice(), core->GetFlatSurface(), &capabilities);
    auto [width, height] = WindowHandler::GetFrameBufferSize();

    swapchainExtent = {
        std::clamp(static_cast<uint32_t>(width), capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
        std::clamp(static_cast<uint32_t>(height), capabilities.minImageExtent.height,
                   capabilities.maxImageExtent.height)};

    return capabilities;
}
}    // namespace Graphics
}    // namespace XRLib
