#pragma once

#include "Graphics/Vulkan/VkCore.h"
#include "Logger.h"
#include "Utils/Info.h"
#include "Utils/LibMath.h"
#include "XR/XrCore.h"
#include "XR/XrInput.h"
#include "Event/EventSystem.h"
#include "Event/Events.h"

namespace XRLib {
namespace XR {
class XrBackend {
   public:
    XrBackend(std::shared_ptr<Info> info,
              std::shared_ptr<XRLib::Graphics::VkCore> core,
              std::shared_ptr<XrCore> xrCore);
    ~XrBackend();

    XrResult StartFrame(uint32_t& imageIndex);
    XrResult EndFrame(uint32_t& imageIndex);

    bool XrShouldStop() { return xrShouldStop; };
    void UpdateXrInput() { xrInput.FetchInput(); }

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
    std::shared_ptr<Info> info{nullptr};
    std::shared_ptr<XRLib::Graphics::VkCore> vkCore{nullptr};
    std::shared_ptr<XrCore> xrCore{nullptr};

    XrInput xrInput;

    std::vector<const char*> activeAPILayers = {};
    std::vector<const char*> activeInstanceExtensions = {};
    std::vector<std::string> apiLayers = {};

    std::vector<std::string> instanceExtensions = {
        XR_KHR_VULKAN_ENABLE_EXTENSION_NAME};

    XrDebugUtilsMessengerEXT xrDebugUtilsMessenger{XR_NULL_HANDLE};

    uint32_t viewCount;
    bool sessionRunning{false};
    bool frameStarted{false};
    bool xrShouldStop{false};
};
}    // namespace XR
}    // namespace XRLib
