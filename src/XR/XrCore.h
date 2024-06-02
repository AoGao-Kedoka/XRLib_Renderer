#pragma once

#include "vulkan/vulkan.h"
#include <vector>
#define XR_USE_GRAPHICS_API_VULKAN
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>

#include "LAMath.h"
#include "Util.h"

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

    std::vector<XrSwapchain>& GetXrSwapchains() { return xrSwapchains; }
    std::vector<std::vector<XrSwapchainImageVulkanKHR>>&
    GetXrSwapchainImages() {
        return swapchainImages;
    }

    const XrSpace GetXrSpace() {
        if (xrSceneSpace == XR_NULL_HANDLE) {
            CreatePlaySpace();
        }
        return xrSceneSpace;
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

    XrSpace xrSceneSpace{XR_NULL_HANDLE};

    bool xrValid = true;
    std::vector<XrViewConfigurationView> xrViewsConfiguration;

    std::vector<XrSwapchain> xrSwapchains;
    std::vector<std::vector<XrSwapchainImageVulkanKHR>> swapchainImages;
};
