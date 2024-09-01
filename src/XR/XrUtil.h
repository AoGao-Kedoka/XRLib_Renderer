#pragma once

#include <pch.h>

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

    static void LogXrRuntimeProperties(XrInstance xrInstance) {
        if (xrInstance == XR_NULL_HANDLE) {
            LOGGER(LOGGER::ERR) << "XR Instance is null";
        }

        XrInstanceProperties instanceProperties{XR_TYPE_INSTANCE_PROPERTIES};
        if (xrGetInstanceProperties(xrInstance, &instanceProperties) !=
            XR_SUCCESS) {
            LOGGER(LOGGER::ERR) << "Failed to get instance properties";
        } else {
            LOGGER(LOGGER::INFO)
                << "Using OpenXR Runtime: " << instanceProperties.runtimeName
                << " - " << XR_VERSION_MAJOR(instanceProperties.runtimeVersion)
                << XR_VERSION_MINOR(instanceProperties.runtimeVersion)
                << XR_VERSION_PATCH(instanceProperties.runtimeVersion);
        }
    }

    static void LogXrSystemProperties(XrInstance xrInstance,
                                      XrSystemId systemId) {
        XrSystemProperties systemProperties{XR_TYPE_SYSTEM_PROPERTIES};
        if (xrGetSystemProperties(xrInstance, systemId, &systemProperties) !=
            XR_SUCCESS) {
            LOGGER(LOGGER::ERR) << "Failed to get system properties";
        } else {
            LOGGER(LOGGER::INFO) << "OpenXR System Properties:";
            LOGGER(LOGGER::INFO)
                << "  System ID: " << systemProperties.systemId;
            LOGGER(LOGGER::INFO)
                << "  Vendor ID: " << systemProperties.vendorId;
            LOGGER(LOGGER::INFO)
                << "  System Name: " << systemProperties.systemName;
            LOGGER(LOGGER::INFO) << "  Graphics Properties:";
            LOGGER(LOGGER::INFO)
                << "    Max Swapchain Image Width: "
                << systemProperties.graphicsProperties.maxSwapchainImageWidth;
            LOGGER(LOGGER::INFO)
                << "    Max Swapchain Image Height: "
                << systemProperties.graphicsProperties.maxSwapchainImageHeight;
            LOGGER(LOGGER::INFO)
                << "    Max Layer Count: "
                << systemProperties.graphicsProperties.maxLayerCount;
            LOGGER(LOGGER::INFO) << "  Tracking Properties:";
            LOGGER(LOGGER::INFO)
                << "    Orientation Tracking: "
                << (systemProperties.trackingProperties.orientationTracking
                        ? "Supported"
                        : "Not Supported");
            LOGGER(LOGGER::INFO)
                << "    Position Tracking: "
                << (systemProperties.trackingProperties.positionTracking
                        ? "Supported"
                        : "Not Supported");
        }
    }
};
