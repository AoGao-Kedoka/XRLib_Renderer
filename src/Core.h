#pragma once

#include <vulkan/vulkan.h>
#define XR_USE_GRAPHICS_API_VULKAN
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>

#include <string>
#include <vector>
#include <cstring>
#include <fstream>
#include <sstream>

class Core {
   public:
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
};