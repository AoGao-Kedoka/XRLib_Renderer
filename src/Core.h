#pragma once

#include <vulkan/vulkan.h>
#define XR_USE_GRAPHICS_API_VULKAN
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>

#include <cstring>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "Logger.h"

class Core {
   public:
    Core() = default;
    ~Core();
    /*
    * Vulkan
    */
    VkInstance& GetRenderInstance() { return vkInstance; }
    VkPhysicalDevice& GetRenderPhysicalDevice() { return vkPhysicalDevice; }
    VkDevice& GetRenderDevice() { return vkDevice; }
    VkQueue& GetGraphicsQueue() { return graphicsQueue; }
    int32_t GetGraphicsQueueFamilyIndex() {
        if (graphicsQueueIndex == -1)
            ParseGraphicsQueueFamilyIndex();
        return graphicsQueueIndex;
    }

    VkSurfaceKHR& GetFlatSurface() { return surfaceFlat; }
    VkSwapchainKHR& GetFlatSwapchain() { return swapChainFlat; }
    std::vector<VkImage>& GetFlatSwapchainImages() {
        return swapChainImagesFlat;
    }
    VkFormat GetFlatSwapchainImageFormat() { return swapChainImageFormat; }
    void SetFlatSwapchainImageFormat(VkFormat format) {
        this->swapChainImageFormat = format;
    }
    VkExtent2D& GetFlatSwapchainExtent2D() { return swapChainExtentFlat; }
    void SetFlatSwapchainExtent2D(VkExtent2D extent) {
        this->swapChainExtentFlat = extent;
    }
    std::vector<VkImageView>& GetSwapchainImageViewsFlat() {
        return swapChainImageViewsFlat;
    }

    std::vector<VkFramebuffer>& GetSwapchainFrameBufferFlat() {
        return swapChainFrameBuffers;
    }

    VkCommandPool& GetCommandPool() {
        if (commandPool == VK_NULL_HANDLE) {
            CreateCommandPool();
        }
        return commandPool;
    }

    VkCommandBuffer& GetCommandBuffer() {
        if (commandBuffer == VK_NULL_HANDLE) {
            CreateCommandBuffer();
        }
        return commandBuffer;
    }

    /*
    * OpenXR
    */
    XrInstance& GetXRInstance() { return xrInstance; }
    bool IsXRValid() { return xrValid; }
    void SetXRValid(bool value) { xrValid = value; }
    XrSession& GetXRSession() { return xrSession; }
    XrSystemId& GetSystemID() { return xrSystemID; }

    std::vector<const char*> UnpackExtensionString(const std::string& string) {
        std::vector<const char*> out;
        std::istringstream stream{string};
        std::string extension;
        while (getline(stream, extension, ' ')) {
            const size_t len = extension.size() + 1u;
            char* str = new char[len];
            memcpy(str, extension.c_str(), len);
            out.push_back(str);
        }

        return out;
    }

   private:
    VkInstance vkInstance{VK_NULL_HANDLE};
    VkPhysicalDevice vkPhysicalDevice{VK_NULL_HANDLE};
    VkDevice vkDevice{VK_NULL_HANDLE};
    VkQueue graphicsQueue{VK_NULL_HANDLE};

    XrInstance xrInstance{XR_NULL_HANDLE};
    XrSystemId xrSystemID;
    XrGraphicsRequirementsVulkanKHR graphicsRequirements;
    XrSession xrSession{XR_NULL_HANDLE};
    XrSessionState xrSessionState{XR_SESSION_STATE_UNKNOWN};
    XrSpace xrSceneSpace{XR_NULL_HANDLE};

    //Flat surface swapchain
    VkSurfaceKHR surfaceFlat{VK_NULL_HANDLE};
    VkSwapchainKHR swapChainFlat{VK_NULL_HANDLE};
    std::vector<VkImage> swapChainImagesFlat;
    VkFormat swapChainImageFormat{VK_FORMAT_UNDEFINED};
    VkExtent2D swapChainExtentFlat{0, 0};
    std::vector<VkImageView> swapChainImageViewsFlat;
    std::vector<VkFramebuffer> swapChainFrameBuffers;

    // commands
    VkCommandPool commandPool{VK_NULL_HANDLE};
    VkCommandBuffer commandBuffer{VK_NULL_HANDLE};

    bool xrValid = true;

    int32_t graphicsQueueIndex = -1;

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

    void CreateCommandPool() {
        auto graphicsFamilyIndex = GetGraphicsQueueFamilyIndex();
        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = graphicsFamilyIndex;
        if (vkCreateCommandPool(GetRenderDevice(), &poolInfo, nullptr,
                                &commandPool) != VK_SUCCESS) {
            LOGGER(LOGGER::ERR) << "failed to create command pool!";
            exit(-1);
        }
    }

    void CreateCommandBuffer() {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = GetCommandPool();
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;

        if (vkAllocateCommandBuffers(GetRenderDevice(), &allocInfo,
                                     &commandBuffer) != VK_SUCCESS) {
            LOGGER(LOGGER::ERR)<< "failed to allocate command buffers!";
            exit(-1);
        }
    }
};