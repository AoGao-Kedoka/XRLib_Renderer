#pragma once

#include "Info.h"
#include "Logger.h"
#include "RenderBackend.h"

class XRBackend {
   public:
    XRBackend(Info& info, std::shared_ptr<RenderBackend> renderBackend);

    ~XRBackend();

   private:
    Info* info;
    std::shared_ptr<RenderBackend> renderBackend;

    void Cleanup() const;

    XrInstance xrInstance{XR_NULL_HANDLE};
    void CreateXrInstance();
    void LogOpenXRRuntimeProperties() const;

    XrSystemId xrSystemID;
    void GetSystemID();


    XrGraphicsRequirementsVulkanKHR graphicsRequirements;
    XrSession xrSession{XR_NULL_HANDLE};
    XrSessionState xrSessionState{XR_SESSION_STATE_UNKNOWN};
    void CreateXrSession();

    PFN_xrGetVulkanInstanceExtensionsKHR xrGetVulkanInstanceExtensionsKHR{
        nullptr};
    PFN_xrGetVulkanGraphicsDeviceKHR xrGetVulkanGraphicsDeviceKHR{nullptr};
    PFN_xrGetVulkanDeviceExtensionsKHR xrGetVulkanDeviceExtensionsKHR{nullptr};
    PFN_xrGetVulkanGraphicsRequirementsKHR xrGetVulkanGraphicsRequirementsKHR{
        nullptr};
    void LoadXRExtensionFunctions() const;
    void GetFunctionExtensions() const;

    std::vector<const char*> activeAPILayers = {};
    std::vector<const char*> activeInstanceExtensions = {};
    std::vector<std::string> apiLayers = {};
    std::vector<std::string> instanceExtensions = {
        XR_KHR_VULKAN_ENABLE_EXTENSION_NAME};

    XrDebugUtilsMessengerEXT debugUtilsMessenger{XR_NULL_HANDLE};
};
