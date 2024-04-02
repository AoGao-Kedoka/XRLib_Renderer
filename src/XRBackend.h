#pragma once

#include "Core.h"
#include "Info.h"
#include "Logger.h"

class XRBackend {
   public:
    XRBackend(Info& info, Core& core);

    ~XRBackend();

   private:
    Info* info;
    Core* core;

    void Cleanup() const;
    void CreateXrInstance();
    void LogOpenXRRuntimeProperties() const;
    void GetSystemID();

    void CreateXrSession();

    std::vector<const char*> activeAPILayers = {};
    std::vector<const char*> activeInstanceExtensions = {};
    std::vector<std::string> apiLayers = {};
    std::vector<std::string> instanceExtensions = {
        XR_KHR_VULKAN_ENABLE_EXTENSION_NAME};

    XrDebugUtilsMessengerEXT xrDebugUtilsMessenger{XR_NULL_HANDLE};
};
