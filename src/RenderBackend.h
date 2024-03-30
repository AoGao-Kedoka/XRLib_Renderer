#pragma once

#include <vector>
#include <GLFW/glfw3.h>

#include "Core.h"
#include "Info.h"
#include "Logger.h"

class RenderBackend {
   public:
    RenderBackend(Info& info, Core& core);

    void CreateVulkanInstance();
    void CreatePhysicalDevice();

   private:
    Info* info;
    Core* core;
    void Cleanup();


    const std::vector<const char*> validataionLayers = {
        "VK_LAYER_KHRONOS_validation"};

    PFN_xrGetVulkanInstanceExtensionsKHR xrGetVulkanInstanceExtensionsKHR{
        nullptr};
    PFN_xrGetVulkanGraphicsDeviceKHR xrGetVulkanGraphicsDeviceKHR{nullptr};
    PFN_xrGetVulkanDeviceExtensionsKHR xrGetVulkanDeviceExtensionsKHR{nullptr};
    PFN_xrGetVulkanGraphicsRequirementsKHR xrGetVulkanGraphicsRequirementsKHR{
        nullptr};
    void LoadXRExtensionFunctions(XrInstance xrInstance) const;
};