#pragma once

#include <vulkan/vulkan.h>
#define XR_USE_GRAPHICS_API_VULKAN
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>

#include <iostream>
#include <string>

#include "Info.h"
#include "Logger.h"

class XRBackend {
   public:
    XRBackend(Info info);
    ~XRBackend();

   private:
    Info info;

    XrInstance xrInstance;
    void CreateXrInstance();
    void DeleteXRInstance();

    XrSystemId xrSystemID;
    void GetSystemID();

    std::vector<const char*> activeAPILayers = {};
    std::vector<const char*> activeInstanceExtensions = {};
    std::vector<std::string> apiLayers = {};
    std::vector<std::string> instanceExtensions = {
        XR_KHR_VULKAN_ENABLE_EXTENSION_NAME};

    XrDebugUtilsMessengerEXT debugUtilsMessenger;
};
