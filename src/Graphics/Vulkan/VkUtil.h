#pragma once

#include "Graphics/Primitives.h"
#include "Logger.h"

class VkUtil {
   public:
    template <typename T, typename Func, typename... Args>
    static void VkSafeClean(Func func, T variable, Args&&... args) {
        if (variable != VK_NULL_HANDLE) {
            func(variable, std::forward<Args>(args)...);
        }
    }

    template <typename Func, typename T, typename K, typename... Args>
    static void VkSafeClean(Func func, T variableA, K variableB,
                            Args&&... args) {
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

    static VKAPI_ATTR VkBool32 VKAPI_CALL
    VkDebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
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
        LOGGER(logLevel) << "Validation layer in Rendering backend: "
                         << pCallbackData->pMessage;
        return VK_FALSE;
    }

    static VkVertexInputBindingDescription GetVertexBindingDescription();

    static std::array<VkVertexInputAttributeDescription, 3>
    GetVertexAttributeDescription();
};
