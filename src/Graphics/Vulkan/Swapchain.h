#pragma once

#include "Image.h"

namespace XRLib {
namespace Graphics {
class Swapchain {
   public:
    // normal flat swapchain
    Swapchain(VkCore& core);

    // swapchain from openxr
    Swapchain(VkCore& core, std::vector<std::unique_ptr<Image>>& images);

    ~Swapchain();
    std::vector<std::vector<Image*>>& GetSwapchainImages(bool ignoreEmpty = false);
    VkSwapchainKHR GetSwaphcain() { return swapchain; }
    void RecreateSwapchain();
    int FramesInFlight() { return GetSwapchainImages().size(); }

   private:
    void CreateSwapchain();
    VkSurfaceFormatKHR ChooseSwapchainImageFormat();
    VkPresentModeKHR ChooseSwapchainPresentMode();
    VkSurfaceCapabilitiesKHR GetSurfaceCapabilities();

   private:
    VkCore& core;
    VkSurfaceKHR surface{VK_NULL_HANDLE};
    VkFormat swapchainImageFormat{VK_FORMAT_UNDEFINED};
    VkSwapchainKHR swapchain{VK_NULL_HANDLE};
    std::vector<std::unique_ptr<Image>> swapchainImages;
    VkExtent2D swapchainExtent;

    // this is just for render targets output of the swapchain images
    std::vector<std::vector<Image*>> swapchainRenderTargets;
};
}    // namespace Graphics
}    // namespace XRLib
