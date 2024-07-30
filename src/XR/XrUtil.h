#pragma once

#include <cstring>
#include <vector>

#define XR_USE_GRAPHICS_API_VULKAN
#include <openxr/openxr.h>

#include "Logger.h"

class XrUtil {
   public:
    template <typename T, typename Func, typename... Args>
    static void XrSafeClean(Func func, T variable, Args&&... args) {
        if (variable != XR_NULL_HANDLE) {
            func(variable, std::forward<Args>(args)...);
        }
    }

    template <typename T>
    static T XrGetXRFunction(XrInstance instance, const char* name) {
        PFN_xrVoidFunction func;
        if (xrGetInstanceProcAddr(instance, name, &func) != XR_SUCCESS) {
            LOGGER(LOGGER::ERR) << "Failed to get xr function: " << name;
        }
        return (T)func;
    }

    static bool XrCheckLayerSupport(const char* layer) {
        uint32_t layerCount;
        xrEnumerateApiLayerProperties(0, &layerCount, nullptr);
        std::vector<XrApiLayerProperties> availableLayers{
            layerCount, {XR_TYPE_API_LAYER_PROPERTIES}};
        xrEnumerateApiLayerProperties(layerCount, &layerCount,
                                      availableLayers.data());
        for (const auto& layerProperties : availableLayers) {
            if (std::strcmp(layer, layerProperties.layerName) == 0) {
                return true;
            }
        }
        return true;
    }

    static XRAPI_ATTR XrBool32 XRAPI_CALL
    XrDebugCallback(XrDebugUtilsMessageSeverityFlagsEXT messageSeverity,
                    XrDebugUtilsMessageTypeFlagsEXT messageTypes,
                    const XrDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                    void* pUserData) {
        auto logLevel = LOGGER::INFO;
        switch (messageSeverity) {
            case XR_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
                logLevel = LOGGER::WARNING;
                break;
            case XR_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
                logLevel = LOGGER::ERR;
                break;
            default:
                logLevel = LOGGER::INFO;
        }
        LOGGER(logLevel) << "Validation layer in XR backend: "
                         << pCallbackData->message;
        return XR_FALSE;
    }
};
