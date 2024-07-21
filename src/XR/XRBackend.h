#pragma once

#include <cstring>
#include <utility>

#include "Graphics/Vulkan/VkCore.h"
#include "Utils/Info.h"
#include "Logger.h"
#include "XR/XrCore.h"

class XRBackend {
   public:
    XRBackend(std::shared_ptr<Info> info, std::shared_ptr<VkCore> core,
              std::shared_ptr<XrCore> xrCore);
    ~XRBackend();

    XRBackend(XRBackend&& other) noexcept
        : info(std::exchange(other.info, nullptr)),
          xrCore(std::exchange(other.xrCore, nullptr)),
          vkCore(std::exchange(other.vkCore, nullptr)),
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
        xrCore = std::exchange(rhs.xrCore, nullptr);
        vkCore = std::exchange(rhs.vkCore, nullptr);
        activeAPILayers = std::move(rhs.activeAPILayers);
        activeInstanceExtensions = std::move(rhs.activeInstanceExtensions);
        apiLayers = std::move(rhs.apiLayers);
        instanceExtensions = std::move(rhs.instanceExtensions);
        xrDebugUtilsMessenger =
            std::exchange(rhs.xrDebugUtilsMessenger, XR_NULL_HANDLE);
        return *this;
    }

    void Prepare();
    XrResult StartFrame(uint32_t& imageIndex);
    XrResult EndFrame(uint32_t& imageIndex);

   private:
    void CreateXrInstance();
    void LogOpenXRRuntimeProperties() const;
    void LogOpenXRSystemProperties() const;
    void GetSystemID();

    void CreateXrSession();
    void CreateXrSwapchain();
    void PrepareXrSwapchainImages();

    void PollXREvents();
    void BeginSession();
    void EndSession();

   private:
    std::shared_ptr<Info> info{nullptr};
    std::shared_ptr<VkCore> vkCore{nullptr};
    std::shared_ptr<XrCore> xrCore{nullptr};

    std::vector<const char*> activeAPILayers = {};
    std::vector<const char*> activeInstanceExtensions = {};
    std::vector<std::string> apiLayers = {};

    std::vector<std::string> instanceExtensions = {
        XR_KHR_VULKAN_ENABLE_EXTENSION_NAME};

    XrDebugUtilsMessengerEXT xrDebugUtilsMessenger{XR_NULL_HANDLE};
};
