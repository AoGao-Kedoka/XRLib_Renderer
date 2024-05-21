#pragma once

#include <utility>

#include "Core.h"
#include "Info.h"
#include "Logger.h"

class XRBackend {
   public:
    XRBackend(Info& info, Core& core);
    ~XRBackend();

    XRBackend(XRBackend&& other) noexcept
        : info(std::exchange(other.info, nullptr)),
          core(std::exchange(other.core, nullptr)),
          activeAPILayers(std::move(other.activeAPILayers)),
          activeInstanceExtensions(std::move(other.activeInstanceExtensions)),
          apiLayers(std::move(other.apiLayers)),
          instanceExtensions(std::move(other.instanceExtensions)),
          xrDebugUtilsMessenger(
              std::exchange(other.xrDebugUtilsMessenger, XR_NULL_HANDLE)) {
        LOGGER(LOGGER::DEBUG) << "Move constructor called";
    }

    XRBackend& operator=(XRBackend&& rhs) noexcept {
        if (this == &rhs)
            return *this;
        LOGGER(LOGGER::DEBUG) << "Move assignment called";

        info = std::exchange(rhs.info, nullptr);
        core = std::exchange(rhs.core, nullptr);
        activeAPILayers = std::move(rhs.activeAPILayers);
        activeInstanceExtensions = std::move(rhs.activeInstanceExtensions);
        apiLayers = std::move(rhs.apiLayers);
        instanceExtensions = std::move(rhs.instanceExtensions);
        xrDebugUtilsMessenger =
            std::exchange(rhs.xrDebugUtilsMessenger, XR_NULL_HANDLE);
        return *this;
    }

    void XrCreateSwapcahin();

   private:
    Info* info;
    Core* core;

    void CreateXrInstance();
    void LogOpenXRRuntimeProperties() const;
    void LogOpenXRSystemProperties() const;
    void GetSystemID();

    void CreateXrSession();

    std::vector<const char*> activeAPILayers = {};
    std::vector<const char*> activeInstanceExtensions = {};
    std::vector<std::string> apiLayers = {};
    std::vector<std::string> instanceExtensions = {
        XR_KHR_VULKAN_ENABLE_EXTENSION_NAME};

    XrDebugUtilsMessengerEXT xrDebugUtilsMessenger{XR_NULL_HANDLE};
};
