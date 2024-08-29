#pragma once

#include <cstring>
#include <utility>

#include "Graphics/Vulkan/VkCore.h"
#include "Logger.h"
#include "Utils/Info.h"
#include "Utils/LibMath.h"
#include "XR/XrCore.h"

class XRBackend {
   public:
    XRBackend(std::shared_ptr<Info> info, std::shared_ptr<VkCore> core,
              std::shared_ptr<XrCore> xrCore);
    ~XRBackend();
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

    void BeginSession();
    void EndSession();

    void UpdateViews();
    void PollEvents();

   private:
    std::shared_ptr<Info> info{nullptr};
    std::shared_ptr<VkCore> vkCore{nullptr};
    std::shared_ptr<XrCore> xrCore{nullptr};

    std::vector<const char*> activeAPILayers = {};
    std::vector<const char*> activeInstanceExtensions = {};
    std::vector<std::string> apiLayers = {};

    std::vector<glm::mat4> viewMatrices;
    std::vector<glm::mat4> projectionMatrices;

    std::vector<std::string> instanceExtensions = {
        XR_KHR_VULKAN_ENABLE_EXTENSION_NAME};

    XrDebugUtilsMessengerEXT xrDebugUtilsMessenger{XR_NULL_HANDLE};

    uint32_t viewCount;
    bool sessionRunning{false};
    bool frameStarted{false};
};
