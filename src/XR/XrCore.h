#pragma once

#include "vulkan/vulkan.h"
#include <vector>
#define XR_USE_GRAPHICS_API_VULKAN
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>

#include "Utils/LAMath.h"
#include "Utils/Util.h"

class XrCore {
   public:
    XrCore() = default;
    ~XrCore();
    XrInstance& GetXRInstance() { return xrInstance; }
    bool IsXRValid() { return xrValid; }
    void SetXRValid(bool value) { xrValid = value; }
    XrSession& GetXRSession() { return xrSession; }
    XrSystemId& GetSystemID() { return xrSystemID; }
    std::vector<XrViewConfigurationView>& GetXRViewConfigurationView() {
        return xrViewsConfiguration;
    }

    XrGraphicsRequirementsVulkanKHR& GetGraphicsRequirements() {
        return graphicsRequirements;
    }

    XrSwapchain& GetXrSwapchain() { return xrSwapchain; }

    XrFrameState& GetXrFrameState() { return frameState; }

    std::vector<XrSwapchainImageVulkanKHR>& GetSwapchainImages() {
        return swapchainImages;
    }

    const XrSpace GetXrSpace() {
        if (xrSceneSpace == XR_NULL_HANDLE) {
            CreatePlaySpace();
        }
        return xrSceneSpace;
    }

    std::vector<XrCompositionLayerProjectionView>&
        GetCompositionLayerProjectionViews() {
        return compositionLayerProjectionViews;
    }

   private:
    void CreatePlaySpace();

   private:
    XrInstance xrInstance{XR_NULL_HANDLE};
    XrSystemId xrSystemID;
    XrGraphicsRequirementsVulkanKHR graphicsRequirements{
        XR_TYPE_GRAPHICS_REQUIREMENTS_VULKAN_KHR};

    XrSession xrSession{XR_NULL_HANDLE};
    XrSessionState xrSessionState{XR_SESSION_STATE_UNKNOWN};

    XrViewState xrViewState{};
    XrFrameState frameState{XR_TYPE_FRAME_STATE};
    XrSpace xrSceneSpace{XR_NULL_HANDLE};

    bool xrValid = true;
    std::vector<XrViewConfigurationView> xrViewsConfiguration;

    XrSwapchain xrSwapchain;
    std::vector<XrSwapchainImageVulkanKHR> swapchainImages;


    std::vector<XrCompositionLayerProjectionView>
        compositionLayerProjectionViews;
};
