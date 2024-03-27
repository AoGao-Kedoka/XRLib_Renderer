#pragma once

#include <vulkan/vulkan.h>

#include "Info.h"

class RenderBackend {
   public:
    RenderBackend(Info& info);

    uint32_t GetQueueFamilyIndex();
    VkInstance GetRenderInstance() const { return vkInstance; }
    VkPhysicalDevice GetRenderPhysicalDevice() const { return vkPhysicalDevice; }
    VkDevice GetRenderDevice() const { return vkDevice; }

   private:
    Info* info;
    VkInstance vkInstance{VK_NULL_HANDLE};
    VkPhysicalDevice vkPhysicalDevice{VK_NULL_HANDLE};
    VkDevice vkDevice{VK_NULL_HANDLE};
};