#pragma once

#include "pch.h"

#include "Graphics/Primitives.h"
#include "Logger.h"
#include "Utils/Util.h"

namespace XRLib {
namespace Graphics {
class VkUtil {
   public:
    template <typename T, typename Func, typename... Args>
    static void VkSafeClean(Func func, T variable, Args&&... args) {
        if (variable != VK_NULL_HANDLE) {
            func(variable, std::forward<Args>(args)...);
        }
    }

    template <typename Func, typename T, typename K, typename... Args>
    static void VkSafeClean(Func func, T variableA, K variableB, Args&&... args) {
        if (variableA != VK_NULL_HANDLE && variableB != VK_NULL_HANDLE) {
            func(variableA, variableB, std::forward<Args>(args)...);
        }
    }

    static bool VkCheckLayerSupport(const char* layer) {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const auto& layerProperties : availableLayers) {
            if (std::strcmp(layer, layerProperties.layerName) == 0) {
                return true;
            }
        }
        return false;
    }

    static VKAPI_ATTR VkBool32 VKAPI_CALL VkDebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                          VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                          const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                          void* pUserData) {
        auto logLevel = LOGGER::INFO;
        switch (messageSeverity) {
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
                logLevel = LOGGER::WARNING;
                break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
                logLevel = LOGGER::ERR;
                break;
            default:
                logLevel = LOGGER::INFO;
        }
        LOGGER(logLevel) << "Validation layer in Rendering backend: " << pCallbackData->pMessage;
        return VK_FALSE;
    }

    static VkFormat FindSupportedFormat(VkPhysicalDevice physicalDevice, const std::vector<VkFormat>& candidates,
                                        VkImageTiling tiling, VkFormatFeatureFlags features) {
        for (VkFormat format : candidates) {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);
            if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
                return format;
            } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
                return format;
            }
        }
        Util::ErrorPopup("Error finding supported format");
        return candidates[0];
    }

    static VkFormat FindDepthFormat(const VkPhysicalDevice physicalDevice) {
        return FindSupportedFormat(physicalDevice,
                                   {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
                                   VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
    }

    static VkVertexInputBindingDescription GetVertexBindingDescription();

    static std::array<VkVertexInputAttributeDescription, 3> GetVertexAttributeDescription();
};
}    // namespace Graphics
}    // namespace XRLib
