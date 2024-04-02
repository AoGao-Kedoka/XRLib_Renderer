#pragma once

#include <GLFW/glfw3.h>
#include <vector>

#include "Core.h"
#include "Info.h"
#include "Logger.h"

class RenderBackend {
   public:
    RenderBackend(Info& info, Core& core);
    ~RenderBackend();

    RenderBackend(RenderBackend&& src) noexcept
        : info(std::exchange(src.info, nullptr)),
          core(std::exchange(src.core, nullptr)),
          vkDebugMessenger(std::exchange(src.vkDebugMessenger, VK_NULL_HANDLE)),
          vkCreateDebugUtilsMessengerEXT(
              std::exchange(src.vkCreateDebugUtilsMessengerEXT, nullptr)),
          xrGetVulkanInstanceExtensionsKHR(
              std::exchange(src.xrGetVulkanInstanceExtensionsKHR, nullptr)),
          xrGetVulkanGraphicsDeviceKHR(
              std::exchange(src.xrGetVulkanGraphicsDeviceKHR, nullptr)),
          xrGetVulkanDeviceExtensionsKHR(
              std::exchange(src.xrGetVulkanDeviceExtensionsKHR, nullptr)),
          xrGetVulkanGraphicsRequirementsKHR(
              std::exchange(src.xrGetVulkanGraphicsRequirementsKHR, nullptr)) {
        LOGGER(LOGGER::DEBUG) << "Move constructor called";
    }

    RenderBackend& operator=(RenderBackend&& rhs) noexcept {
        if (this == &rhs)
            return *this;

        LOGGER(LOGGER::DEBUG) << "Move assignment called";
        info = std::exchange(rhs.info, nullptr);
        core = std::exchange(rhs.core, nullptr);
        vkDebugMessenger = std::exchange(rhs.vkDebugMessenger, VK_NULL_HANDLE);
        vkCreateDebugUtilsMessengerEXT =
            std::exchange(rhs.vkCreateDebugUtilsMessengerEXT, nullptr);
        xrGetVulkanInstanceExtensionsKHR =
            std::exchange(rhs.xrGetVulkanInstanceExtensionsKHR, nullptr);
        xrGetVulkanGraphicsDeviceKHR =
            std::exchange(rhs.xrGetVulkanGraphicsDeviceKHR, nullptr);
        xrGetVulkanDeviceExtensionsKHR =
            std::exchange(rhs.xrGetVulkanDeviceExtensionsKHR, nullptr);
        xrGetVulkanGraphicsRequirementsKHR =
            std::exchange(rhs.xrGetVulkanGraphicsRequirementsKHR, nullptr);
        return *this;
    }

    void CreateVulkanInstance();
    void CreatePhysicalDevice();
    void CreateLogicalDevice();

   private:
    Info* info;
    Core* core;

    const std::vector<const char*> validataionLayers = {
        "VK_LAYER_KHRONOS_validation"};
    VkDebugUtilsMessengerEXT vkDebugMessenger{};

    PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT{nullptr};

    PFN_xrGetVulkanInstanceExtensionsKHR xrGetVulkanInstanceExtensionsKHR{
        nullptr};
    PFN_xrGetVulkanGraphicsDeviceKHR xrGetVulkanGraphicsDeviceKHR{nullptr};
    PFN_xrGetVulkanDeviceExtensionsKHR xrGetVulkanDeviceExtensionsKHR{nullptr};
    PFN_xrGetVulkanGraphicsRequirementsKHR xrGetVulkanGraphicsRequirementsKHR{
        nullptr};
    void LoadXRExtensionFunctions(XrInstance xrInstance) const;
};