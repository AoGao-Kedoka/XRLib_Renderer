#pragma once

#include "Graphics/Vulkan/VkCore.h"
#include "Logger.h"
#include "Utils/Info.h"
#include "Utils/LibMath.h"
#include "XR/XrCore.h"
#include "XR/XrInput.h"

class XrBackend {
   public:
    XrBackend(std::shared_ptr<Info> info, std::shared_ptr<VkCore> core,
              std::shared_ptr<XrCore> xrCore);
    ~XrBackend();
    void Prepare();

    XrResult StartFrame(uint32_t& imageIndex);
    XrResult EndFrame(uint32_t& imageIndex);

    bool XrShouldStop() { return xrShouldStop; };
    void UpdateXrInput() { xrInput.FetchInput(); }

   private:
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
    std::shared_ptr<VkCore> vkCore{nullptr};
    std::shared_ptr<XrCore> xrCore{nullptr};

    XrInput xrInput;

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
    bool xrShouldStop{false};
};
