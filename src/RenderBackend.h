#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <vector>
#include <utility>

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
          window(std::exchange(src.window, nullptr)),
          vkDebugMessenger(std::exchange(src.vkDebugMessenger, VK_NULL_HANDLE)),
          vkCreateDebugUtilsMessengerEXT(
              std::exchange(src.vkCreateDebugUtilsMessengerEXT, nullptr)) {
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
        return *this;
    }

    void CreateVulkanInstance();
    void CreatePhysicalDevice();
    void CreateLogicalDevice();
    virtual void Prepare(){};

   protected:
    Info* info;
    Core* core;
    GLFWwindow* window;

    const std::vector<const char*> validataionLayers = {
        "VK_LAYER_KHRONOS_validation"};
    VkDebugUtilsMessengerEXT vkDebugMessenger{};

    PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT{nullptr};
};

