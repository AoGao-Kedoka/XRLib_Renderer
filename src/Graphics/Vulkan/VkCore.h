#pragma once

#include "Graphics/Window.h"
#include "Utils/Config.h"
#include "Utils/Util.h"
#include "VkUtil.h"

namespace XRLib {
namespace Graphics {
class VkCore {
   public:
    VkCore() = default;
    ~VkCore();

    // basis
    void CreateVkInstance(Config& config, const std::vector<const char*>& additionalInstanceExts);
    const VkInstance& GetRenderInstance() { return vkInstance; }

    void SelectPhysicalDevice();
    VkPhysicalDevice& VkPhysicalDeviceRef() { return vkPhysicalDevice; }
    const VkPhysicalDevice& GetRenderPhysicalDevice() { return vkPhysicalDevice; }

    void CreateVkDevice(Config& config, const std::vector<const char*>& additionalDeviceExts, bool xr);
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

    uint8_t FramesInFlight = 0;

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

    VkSurfaceKHR surfaceFlat{VK_NULL_HANDLE};

    // pools
    VkCommandPool commandPool{VK_NULL_HANDLE};
    VkDescriptorPool descriptorPool{VK_NULL_HANDLE};

    // semaphores
    VkSemaphore imageAvailableSemaphore{VK_NULL_HANDLE};
    VkSemaphore renderFinishedSemaphore{VK_NULL_HANDLE};
    VkFence inFlightFence{VK_NULL_HANDLE};

    int32_t graphicsQueueIndex = -1;

    // validataion layer
    const std::vector<const char*> validataionLayers = {"VK_LAYER_KHRONOS_validation"};
    VkDebugUtilsMessengerEXT vkDebugMessenger{};
    PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT{nullptr};
};
}    // namespace Graphics
}    // namespace XRLib
