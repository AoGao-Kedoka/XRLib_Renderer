#pragma once

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

   private:

    XrInstance xrInstance{XR_NULL_HANDLE};
    XrSystemId xrSystemID;
    XrGraphicsRequirementsVulkanKHR graphicsRequirements;
    XrSession xrSession{XR_NULL_HANDLE};
    XrSessionState xrSessionState{XR_SESSION_STATE_UNKNOWN};
    XrSpace xrSceneSpace{XR_NULL_HANDLE};

    bool xrValid = true;
    std::vector<XrViewConfigurationView> xrViewsConfiguration;
};