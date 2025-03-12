#pragma once

#include "Utils/MathUtil.h"
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

    XrViewConfigurationType& GetXrViewConfigurationType() { return viewConfigurationType; }

    std::vector<XrViewConfigurationView>& GetXRViewConfigurationView() { return xrViewsConfiguration; }

    XrGraphicsRequirementsVulkanKHR& GetGraphicsRequirements() { return graphicsRequirements; } // deprecated
    XrGraphicsRequirementsVulkan2KHR& GetGraphicsRequirements2() { return graphicsRequirements2; }

    XrSwapchain& GetXrSwapchain() { return xrSwapchain; }

    XrViewState& GetXrViewState() { return xrViewState; }

    XrFrameState& GetXrFrameState() { return frameState; }

    std::vector<XrSwapchainImageVulkanKHR>& GetSwapchainImages() { return swapchainImages; }

    std::vector<XrView>& GetXrViews() { return xrViews; }

    const XrSpace GetXrSpace() {
        if (xrSceneSpace == XR_NULL_HANDLE) {
            CreatePlaySpace();
        }
        return xrSceneSpace;
    }

    std::vector<XrCompositionLayerProjectionView>& GetCompositionLayerProjectionViews() {
        return compositionLayerProjectionViews;
    }

    XrSessionState& GetXrSessionState() { return xrSessionState; }

    XrTime& GetXrTime() { return time; }

    std::vector<int64_t>& SwapchainFormats() { return swapchainFormats; }

    std::pair<uint32_t, uint32_t> SwapchainExtent() { 
        return {GetXRViewConfigurationView()[0].recommendedImageRectWidth,
                GetXRViewConfigurationView()[0].recommendedImageRectHeight};
    }

    // Vulkan
    std::vector<const char*> VkAdditionalInstanceExts(); //deprecated
    std::vector<const char*> VkAdditionalInstanceExts2();
    std::vector<const char*> VkAdditionalDeviceExts(); //deprecated
    std::vector<const char*> VkAdditionalDeviceExts2();
    void VkSetPhysicalDevice(VkInstance instance, VkPhysicalDevice* physicalDevice);

   private:
    void CreatePlaySpace();

   private:
    XrTime time{0};
    XrInstance xrInstance{XR_NULL_HANDLE};
    XrSystemId xrSystemID;

    XrGraphicsRequirementsVulkanKHR graphicsRequirements{XR_TYPE_GRAPHICS_REQUIREMENTS_VULKAN_KHR}; //deprecated
    XrGraphicsRequirementsVulkan2KHR graphicsRequirements2{XR_TYPE_GRAPHICS_REQUIREMENTS_VULKAN2_KHR};

    XrSession xrSession{XR_NULL_HANDLE};
    XrSessionState xrSessionState{XR_SESSION_STATE_UNKNOWN};

    XrViewState xrViewState{};
    XrFrameState frameState{XR_TYPE_FRAME_STATE};
    XrSpace xrSceneSpace{XR_NULL_HANDLE};

    std::vector<int64_t> swapchainFormats;

    bool xrValid = true;

    std::vector<XrView> xrViews;

    XrViewConfigurationType viewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;

    std::vector<XrViewConfigurationView> xrViewsConfiguration;

    XrSwapchain xrSwapchain;
    std::vector<XrSwapchainImageVulkanKHR> swapchainImages;

    std::vector<XrCompositionLayerProjectionView> compositionLayerProjectionViews;
};
}    // namespace XR
}    // namespace XRLib
