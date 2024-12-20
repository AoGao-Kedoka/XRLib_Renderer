#pragma once

#include "Graphics/Vulkan/VkCore.h"
#include "Utils/Info.h"
#include "Utils/MathUtil.h"
#include "XR/XrInput.h"

namespace XRLib {
namespace XR {
class XrBackend {
   public:
    XrBackend(Info& info, Graphics::VkCore& core, XrCore& xrCore);
    ~XrBackend();

    XrResult StartFrame(uint32_t& imageIndex);
    XrResult EndFrame(uint32_t& imageIndex);

    bool XrShouldStop() { return xrShouldStop; };

   private:
    void Prepare();
    void CreateXrInstance();

    void CreateXrSession();
    void CreateXrSwapchain();
    void PrepareXrSwapchainImages();

    void BeginSession();
    void EndSession();

    void UpdateViews();
    void PollEvents();

   private:
    Info& info;
    Graphics::VkCore& vkCore;
    XrCore& xrCore;

    XrInput input;

    std::vector<const char*> activeAPILayers = {};
    std::vector<const char*> activeInstanceExtensions = {};
    std::vector<std::string> apiLayers = {};

    //TODO: XR_KHR_vulkan_enable is deprecated, change to XR_KHR_vulkan_enable2
    std::vector<std::string> instanceExtensions = {XR_KHR_VULKAN_ENABLE_EXTENSION_NAME};

    XrDebugUtilsMessengerEXT xrDebugUtilsMessenger{XR_NULL_HANDLE};

    uint32_t viewCount;
    bool sessionRunning{false};
    bool frameStarted{false};
    bool xrShouldStop{false};
};
}    // namespace XR
}    // namespace XRLib
