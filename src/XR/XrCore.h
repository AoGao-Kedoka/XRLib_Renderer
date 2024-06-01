#pragma once

#include <vector>
#define XR_USE_GRAPHICS_API_VULKAN
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>

class XrCore {
   public:
    XrInstance& GetXRInstance() { return xrInstance; }
    bool IsXRValid() { return xrValid; }
    void SetXRValid(bool value) { xrValid = value; }
    XrSession& GetXRSession() { return xrSession; }
    XrSystemId& GetSystemID() { return xrSystemID; }
    std::vector<XrViewConfigurationView>& GetXRViewConfigurationView() {
        return xrViewsConfiguration;
    }
    std::vector<XrSwapchain>& GetXrSwapchains() { return xrSwapchains; }
    std::vector<std::vector<XrSwapchainImageVulkanKHR>>&
    GetXrSwapchainImages() {
        return swapchainImages;
    }

   private:

    XrInstance xrInstance{XR_NULL_HANDLE};
    XrSystemId xrSystemID;
    XrGraphicsRequirementsVulkanKHR graphicsRequirements;
    XrSession xrSession{XR_NULL_HANDLE};
    XrSessionState xrSessionState{XR_SESSION_STATE_UNKNOWN};
    XrSpace xrSceneSpace{XR_NULL_HANDLE};

    bool xrValid = true;
    std::vector<XrViewConfigurationView> xrViewsConfiguration;

    std::vector<XrSwapchain> xrSwapchains;
    std::vector<std::vector<XrSwapchainImageVulkanKHR>> swapchainImages;
};
