#pragma once

#include <fstream>
#include <iostream>

#include <vulkan/vulkan.h>
#define XR_USE_GRAPHICS_API_VULKAN
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>

#if defined(_WIN32) || defined(_WIN64)
#include <shellapi.h>
#include <windows.h>
#include <filesystem>
#endif

#if defined(__linux__)
#include <unistd.h>
#endif

#include "logger.h"

class Util {
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

    static std::string ReadFile(std::string file_path) {
        std::ifstream t(file_path);
        std::string result = std::string((std::istreambuf_iterator<char>(t)),
                                         (std::istreambuf_iterator<char>()));
        if (result == "") {
            LOGGER(LOGGER::ERR) << "FileReader: FILE IS EMPTY!!";
            return result;
        }
        LOGGER(LOGGER::DEBUG) << result;
        return result;
    }

    static void OpenFileWithSystemEditor(std::string file_path) {
        LOGGER(LOGGER::DEBUG) << file_path;
#if defined(_WIN32) || defined(_WIN64)
        ShellExecute(NULL, NULL,
                     std::filesystem::canonical(file_path).string().c_str(),
                     NULL, NULL, SW_SHOW);
#elif defined(__linux__)
        execl("/usr/bin/xdg-open", "xdg-open", file_path.c_str(), (char*)0);
#endif
    }

    static std::string GetFileExtension(std::string file_path) {
        return file_path.substr(file_path.find_last_of(".") + 1);
    }
};