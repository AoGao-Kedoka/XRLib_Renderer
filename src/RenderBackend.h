#pragma once

#include <vulkan/vulkan.h>
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

   private:
    Info* info;
    void Cleanup();

    VkInstance vkInstance{VK_NULL_HANDLE};
    void CreateVulkanInstance();

    VkPhysicalDevice vkPhysicalDevice{VK_NULL_HANDLE};
    VkDevice vkDevice{VK_NULL_HANDLE};

    const std::vector<const char*> validataionLayers = {
        "VK_LAYER_KHRONOS_validation"};
};