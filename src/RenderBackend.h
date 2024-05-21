#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <utility>
#include <vector>

#include "Core.h"
#include "Graphics/Pipeline.h"
#include "Graphics/RenderPass.h"
#include "Graphics/Shader.h"
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

    virtual void Prepare(
        std::vector<std::pair<std::string, std::string>> passesToAdd){};

    struct GraphicsRenderPass {
        GraphicsRenderPass(Core* core, std::string vertexShaderPath,
                           std::string fragmentShaderPath)
            : core{core} {
            //TODO: Change back to normal vertex and fragment shader
            Shader vertexShader{core, Shader::VERTEX_SHADER};
            Shader fragmentShader{core, Shader::FRAGMENT_SHADER};
            RenderPass tRenderPass{core};

            renderPass = &tRenderPass;

            Pipeline tPipeline{core, std::move(vertexShader),
                               std::move(fragmentShader), renderPass};
            pipeline = &tPipeline;
        }

        Core* core;
        RenderPass* renderPass{nullptr};
        Pipeline* pipeline{nullptr};
    };

    std::vector<GraphicsRenderPass*> renderPasses;

   protected:
    Info* info;
    Core* core;
    GLFWwindow* window;

   private:
    const std::vector<const char*> validataionLayers = {
        "VK_LAYER_KHRONOS_validation"};
    VkDebugUtilsMessengerEXT vkDebugMessenger{};

    PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT{nullptr};
};
