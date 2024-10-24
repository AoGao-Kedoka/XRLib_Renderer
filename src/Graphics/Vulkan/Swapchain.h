#pragma once

#include "Graphics/Vulkan/Image.h"

namespace XRLib {
namespace Graphics {
class Swapchain {
   public:
    Swapchain(std::shared_ptr<VkCore> core);

   private:
    void CreateSwapchain();
    void GetSwapchainImages();
    VkSurfaceFormatKHR ChooseSwapchainImageFormat();
    VkPresentModeKHR ChooseSwapchainPresentMode();
    VkSurfaceCapabilitiesKHR GetSurfaceCapabilities();

   private:
    std::shared_ptr<VkCore> core;
    VkSurfaceKHR surface{VK_NULL_HANDLE};
    VkFormat swapchainImageFormat{VK_FORMAT_UNDEFINED};
    VkSwapchainKHR swapchain{VK_NULL_HANDLE};
    std::vector<std::unique_ptr<Image>> swapchainImages;
    std::vector<VkFramebuffer> swapchainFrameBuffers;
    VkExtent2D swapchainExtent;
};
}    // namespace Graphics
}    // namespace XRLib
