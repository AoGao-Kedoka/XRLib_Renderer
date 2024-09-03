#pragma once

#include "Utils/Util.h"
#include "VkUtil.h"

namespace XRLib {
namespace Graphics {
class VkCore {
   public:
    VkCore() = default;
    ~VkCore();

    VkInstance& GetRenderInstance() { return vkInstance; }
    VkPhysicalDevice& GetRenderPhysicalDevice() { return vkPhysicalDevice; }
    VkDevice& GetRenderDevice() { return vkDevice; }
    VkQueue& GetGraphicsQueue() { return graphicsQueue; }
    int32_t GetGraphicsQueueFamilyIndex() {
        if (graphicsQueueIndex == -1)
            ParseGraphicsQueueFamilyIndex();
        return graphicsQueueIndex;
    }

    // flat renderer
    VkSurfaceKHR& GetFlatSurface() { return surfaceFlat; }
    VkSwapchainKHR& GetFlatSwapchain() { return swapChainFlat; }
    std::vector<VkImage>& GetFlatSwapchainImages() {
        return swapChainImagesFlat;
    }
    VkFormat GetFlatSwapchainImageFormat() { return swapChainImageFormat; }
    void SetFlatSwapchainImageFormat(VkFormat format) {
        this->swapChainImageFormat = format;
    }

    VkExtent2D& GetSwapchainExtent(bool stereo) {
        if (stereo) {
            return swapchainExtentStereo;
        } else {
            return swapChainExtentFlat;
        }
    }
    void SetFlatSwapchainExtent2D(VkExtent2D extent) {
        this->swapChainExtentFlat = extent;
    }
    void SetStereoSwapchainExtent2D(VkExtent2D extent) {
        this->swapchainExtentStereo = extent;
    }

    std::vector<VkImageView>& GetSwapchainImageViewsFlat() {
        return swapChainImageViewsFlat;
    }

    std::vector<VkFramebuffer>& GetSwapchainFrameBuffer() {
        return swapChainFrameBuffers;
    }

    // steoreo
    VkFormat GetStereoSwapchainImageFormat() {
        return stereoSwapchainImageFormat;
    }
    void SetStereoSwapchainImageFormat(VkFormat format) {
        this->stereoSwapchainImageFormat = format;
    }
    std::vector<VkImage>& GetStereoSwapchainImages() {
        return stereoSwapchainImages;
    }
    std::vector<VkImageView>& GetStereoSwapchainImageViews() {
        return stereoSwapchainImageViews;
    }

    // render loop
    VkCommandPool& GetCommandPool() {
        if (commandPool == VK_NULL_HANDLE) {
            CreateCommandPool();
        }
        return commandPool;
    }

    VkSemaphore& GetRenderFinishedSemaphore() {
        if (renderFinishedSemaphore == VK_NULL_HANDLE) {
            CreateSyncSemaphore(renderFinishedSemaphore);
        }
        return renderFinishedSemaphore;
    }

    VkSemaphore& GetImageAvailableSemaphore() {
        if (imageAvailableSemaphore == VK_NULL_HANDLE) {
            CreateSyncSemaphore(imageAvailableSemaphore);
        }
        return imageAvailableSemaphore;
    }

    VkFence& GetInFlightFence() {
        if (inFlightFence == VK_NULL_HANDLE) {
            CreateFence(inFlightFence);
        }
        return inFlightFence;
    }

   private:
    void ParseGraphicsQueueFamilyIndex() {
        uint32_t queueFamilyCount;
        vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice,
                                                 &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(
            vkPhysicalDevice, &queueFamilyCount, queueFamilies.data());

        for (int32_t i = 0; i < queueFamilyCount; ++i) {
            if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                graphicsQueueIndex = i;
                break;
            }
        }
    }

    void CreateCommandPool();
    void CreateCommandBuffer();
    void CreateSyncSemaphore(VkSemaphore& semaphore);
    void CreateFence(VkFence& fence);

   private:
    VkInstance vkInstance{VK_NULL_HANDLE};
    VkPhysicalDevice vkPhysicalDevice{VK_NULL_HANDLE};
    VkDevice vkDevice{VK_NULL_HANDLE};
    VkQueue graphicsQueue{VK_NULL_HANDLE};

    //Flat surface swapchain
    VkSurfaceKHR surfaceFlat{VK_NULL_HANDLE};
    VkSwapchainKHR swapChainFlat{VK_NULL_HANDLE};
    std::vector<VkImage> swapChainImagesFlat;
    VkFormat swapChainImageFormat{VK_FORMAT_UNDEFINED};
    VkExtent2D swapChainExtentFlat{0, 0};
    std::vector<VkImageView> swapChainImageViewsFlat;
    std::vector<VkFramebuffer> swapChainFrameBuffers;

    //stereo swapchain
    VkFormat stereoSwapchainImageFormat{VK_FORMAT_UNDEFINED};
    std::vector<VkImage> stereoSwapchainImages;
    std::vector<VkImageView> stereoSwapchainImageViews;
    VkExtent2D swapchainExtentStereo{0, 0};

    // commands
    VkCommandPool commandPool{VK_NULL_HANDLE};

    // semaphores
    VkSemaphore imageAvailableSemaphore{VK_NULL_HANDLE};
    VkSemaphore renderFinishedSemaphore{VK_NULL_HANDLE};
    VkFence inFlightFence;
    
    int32_t graphicsQueueIndex = -1;

};
}    // namespace Graphics
}    // namespace XRLib
