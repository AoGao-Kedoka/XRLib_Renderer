#pragma once

#include "Image.h"

namespace XRLib {
namespace Graphics {
class Swapchain {
   public:
    Swapchain() = default;

    // normal flat swapchain
    Swapchain(std::shared_ptr<VkCore> core);

    ~Swapchain();
    std::vector<std::vector<std::unique_ptr<Image>>>& GetSwapchainImages(bool ignoreEmpty = false);
    VkSwapchainKHR GetSwaphcain() { return swapchain; }
    void RecreateSwapchain();

   private:
    void CreateSwapchain();
    VkSurfaceFormatKHR ChooseSwapchainImageFormat();
    VkPresentModeKHR ChooseSwapchainPresentMode();
    VkSurfaceCapabilitiesKHR GetSurfaceCapabilities();

   private:
    std::shared_ptr<VkCore> core{nullptr};
    VkSurfaceKHR surface{VK_NULL_HANDLE};
    VkFormat swapchainImageFormat{VK_FORMAT_UNDEFINED};
    VkSwapchainKHR swapchain{VK_NULL_HANDLE};
    std::vector<std::vector<std::unique_ptr<Image>>> swapchainImages;
    VkExtent2D swapchainExtent;
};
}    // namespace Graphics
}    // namespace XRLib
