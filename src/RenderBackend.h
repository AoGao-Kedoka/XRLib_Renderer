#pragma once

#include <vulkan/vulkan.h>
#define XR_USE_GRAPHICS_API_VULKAN
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>
#include <vector>
#include <GLFW/glfw3.h>

#include "Info.h"
#include "Logger.h"

class RenderBackend {
   public:
    RenderBackend(Info& info);

    uint32_t GetQueueFamilyIndex();
    VkInstance GetRenderInstance() const { return vkInstance; }
    VkPhysicalDevice GetRenderPhysicalDevice() const { return vkPhysicalDevice; }
    VkDevice GetRenderDevice() const { return vkDevice; }
    
    std::vector<const char*> GetVulkanInstanceExtensions() const {
        return vulkanInstanceExtensions;
    }
    void CreateVulkanInstance();

   private:
    Info* info;
    void Cleanup();

    VkInstanceCreateInfo createInfo{};
    VkInstance vkInstance{VK_NULL_HANDLE};
    std::vector<const char*> vulkanInstanceExtensions;

    VkPhysicalDevice vkPhysicalDevice{VK_NULL_HANDLE};
    VkDevice vkDevice{VK_NULL_HANDLE};

    const std::vector<const char*> validataionLayers = {
        "VK_LAYER_KHRONOS_validation"};
};