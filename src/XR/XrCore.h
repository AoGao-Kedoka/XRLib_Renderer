#pragma once

#include "Utils/LibMath.h"
#include "Utils/Util.h"
#include "XrUtil.h"

namespace XRLib {
namespace XR {
class XrCore {
   public:
    XrCore() = default;
    ~XrCore();
    XrInstance& GetXRInstance() { return xrInstance; }
    bool IsXRValid() { return xrValid; }
    void SetXRValid(bool value) { xrValid = value; }
    XrSession& GetXRSession() { return xrSession; }
    XrSystemId& GetSystemID() { return xrSystemID; }

    XrViewConfigurationType& GetXrViewConfigurationType() {
        return viewConfigurationType;
    }

    std::vector<XrViewConfigurationView>& GetXRViewConfigurationView() {
        return xrViewsConfiguration;
    }

    XrGraphicsRequirementsVulkanKHR& GetGraphicsRequirements() {
        return graphicsRequirements;
    }

    XrSwapchain& GetXrSwapchain() { return xrSwapchain; }

    XrViewState& GetXrViewState() { return xrViewState; }

    XrFrameState& GetXrFrameState() { return frameState; }

    std::vector<XrSwapchainImageVulkanKHR>& GetSwapchainImages() {
        return swapchainImages;
    }

    std::vector<XrView>& GetXrViews() { return xrViews; }

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

    XrSessionState& GetXrSessionState() { return xrSessionState; }

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

    std::vector<XrView> xrViews;

    XrViewConfigurationType viewConfigurationType =
        XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;

    std::vector<XrViewConfigurationView> xrViewsConfiguration;

    XrSwapchain xrSwapchain;
    std::vector<XrSwapchainImageVulkanKHR> swapchainImages;

    std::vector<XrCompositionLayerProjectionView>
        compositionLayerProjectionViews;
};
}    // namespace XR
}    // namespace XRLib
