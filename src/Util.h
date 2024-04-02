#pragma once
#include "core.h"
#include "logger.h"

class Util {
   public:
    template <typename T, typename Func, typename... Args>
    static void VkSafeClean(T variable, Func func, Args&&... args) {
        if (variable != VK_NULL_HANDLE) {
            func(variable, std::forward<Args>(args)...);
        }
    }

    template <typename T, typename Func, typename... Args>
    static void XrSafeClean(T variable, Func func, Args&&... args) {
        if (variable != XR_NULL_HANDLE) {
            func(variable, std::forward<Args>(args)...);
        }
    }

    static bool VkCheckLayerSupport(const char* layer) {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
        std::vector<VkLayerProperties> availableLayers(layerCount);

        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layer, layerProperties.layerName)) {
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
};