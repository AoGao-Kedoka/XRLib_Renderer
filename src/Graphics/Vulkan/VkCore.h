#pragma once

#include "Graphics/Window.h"
#include "Utils/Info.h"
#include "Utils/Util.h"
#include "VkUtil.h"

namespace XRLib {
namespace Graphics {
class VkCore {
   public:
    VkCore() = default;
    ~VkCore();

    // basis
    void CreateVkInstance(Info& info, const std::vector<const char*>& additionalInstanceExts);
    const VkInstance& GetRenderInstance() { return vkInstance; }

    void SelectPhysicalDevice();
    VkPhysicalDevice& VkPhysicalDeviceRef() { return vkPhysicalDevice; }
    const VkPhysicalDevice& GetRenderPhysicalDevice() { return vkPhysicalDevice; }

    void CreateVkDevice(Info& info, const std::vector<const char*>& additionalDeviceExts, bool xr);
    const VkDevice& GetRenderDevice() { return vkDevice; }

    const VkQueue& GetGraphicsQueue() { return graphicsQueue; }
    int32_t GetGraphicsQueueFamilyIndex() {
        if (graphicsQueueIndex == -1)
            ParseGraphicsQueueFamilyIndex();
        return graphicsQueueIndex;
    }

    uint32_t GetMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    // flat renderer
    VkSurfaceKHR& GetFlatSurface() { return surfaceFlat; }
    VkSwapchainKHR& GetFlatSwapchain() { return swapChainFlat; }
    std::vector<VkImage>& GetFlatSwapchainImages() { return swapChainImagesFlat; }
    VkFormat GetFlatSwapchainImageFormat() { return swapChainImageFormat; }
    void SetFlatSwapchainImageFormat(VkFormat format) { this->swapChainImageFormat = format; }

    VkExtent2D& GetSwapchainExtent(bool stereo) {
        if (stereo) {
            return swapchainExtentStereo;
        } else {
            return swapChainExtentFlat;
        }
    }
    void SetFlatSwapchainExtent2D(VkExtent2D extent) { this->swapChainExtentFlat = extent; }
    void SetStereoSwapchainExtent2D(VkExtent2D extent) { this->swapchainExtentStereo = extent; }

    std::vector<VkImageView>& GetSwapchainImageViewsFlat() { return swapChainImageViewsFlat; }

    std::vector<VkFramebuffer>& GetSwapchainFrameBuffer() { return swapChainFrameBuffers; }

    // steoreo
    VkFormat GetStereoSwapchainImageFormat() { return stereoSwapchainImageFormat; }
    void SetStereoSwapchainImageFormat(VkFormat format) { this->stereoSwapchainImageFormat = format; }
    std::vector<VkImage>& GetStereoSwapchainImages() { return stereoSwapchainImages; }
    std::vector<VkImageView>& GetStereoSwapchainImageViews() { return stereoSwapchainImageViews; }

    VkExtent2D GetswapchainExtentStereo() { return swapchainExtentStereo; }
    // pools
    VkCommandPool& GetCommandPool() {
        if (commandPool == VK_NULL_HANDLE) {
            CreateCommandPool();
        }
        return commandPool;
    }

    VkDescriptorPool& GetDescriptorPool() {
        if (descriptorPool == VK_NULL_HANDLE) {
            CreateDescriptorPool();
        }
        return descriptorPool;
    }

    // rendering loop
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
        vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &queueFamilyCount, queueFamilies.data());

        for (int32_t i = 0; i < queueFamilyCount; ++i) {
            if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                graphicsQueueIndex = i;
                break;
            }
        }
    }

    void CreateCommandPool();
    void CreateDescriptorPool();
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

    // pools
    VkCommandPool commandPool{VK_NULL_HANDLE};
    VkDescriptorPool descriptorPool{VK_NULL_HANDLE};

    // semaphores
    VkSemaphore imageAvailableSemaphore{VK_NULL_HANDLE};
    VkSemaphore renderFinishedSemaphore{VK_NULL_HANDLE};
    VkFence inFlightFence;

    int32_t graphicsQueueIndex = -1;
    uint8_t maxFramesInFlight = 3;

    // validataion layer
    const std::vector<const char*> validataionLayers = {"VK_LAYER_KHRONOS_validation"};
    VkDebugUtilsMessengerEXT vkDebugMessenger{};
    PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT{nullptr};
};
}    // namespace Graphics
}    // namespace XRLib
