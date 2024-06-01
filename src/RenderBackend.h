#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <utility>
#include <vector>

#include "Graphics/VkCore.h"
#include "Graphics/Pipeline.h"
#include "Graphics/RenderPass.h"
#include "Graphics/Shader.h"
#include "XR/XrCore.h"
#include "Info.h"
#include "Logger.h"

class RenderBackend {
   public:
    RenderBackend(Info& info, VkCore& vkCore, XrCore& xrCore);
    ~RenderBackend();

    RenderBackend(RenderBackend&& src) noexcept
        : info(std::exchange(src.info, nullptr)),
          vkCore(std::exchange(src.vkCore, nullptr)),
          xrCore(std::exchange(src.xrCore, nullptr)),
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
        vkCore = std::exchange(rhs.vkCore, nullptr);
        xrCore = std::exchange(rhs.xrCore, nullptr);
        vkDebugMessenger = std::exchange(rhs.vkDebugMessenger, VK_NULL_HANDLE);
        vkCreateDebugUtilsMessengerEXT =
            std::exchange(rhs.vkCreateDebugUtilsMessengerEXT, nullptr);
        return *this;
    }

    bool WindowShouldClose() { return glfwWindowShouldClose(window); }

    virtual void Prepare(
        std::vector<std::pair<std::string, std::string>> passesToAdd){};

    virtual void OnWindowResized() {
        LOGGER(LOGGER::ERR) << "Undefined image resize";
        exit(-1);
    };
    void Run();

    struct GraphicsRenderPass {
        GraphicsRenderPass(VkCore* core, std::string vertexShaderPath,
                           std::string fragmentShaderPath)
            : core{core} {
            Shader vertexShader{core, vertexShaderPath, Shader::VERTEX_SHADER};
            Shader fragmentShader{core, fragmentShaderPath,
                                  Shader::FRAGMENT_SHADER};
            renderPass = RenderPass{core};

            pipeline = Pipeline{core, std::move(vertexShader),
                                std::move(fragmentShader), &renderPass};
        }

        GraphicsRenderPass(VkCore* core) : core{ core } {
            Shader vertexShader{core, Shader::VERTEX_SHADER};
            Shader fragmentShader{core, Shader::FRAGMENT_SHADER};
            renderPass = RenderPass{core};

            pipeline = Pipeline{core, std::move(vertexShader),
                                std::move(fragmentShader), &renderPass};

            renderPass.SetGraphicPipeline(pipeline.GetVkPipeline());
        }

        VkCore* core;
        RenderPass renderPass;
        Pipeline pipeline;
    };

    std::vector<std::unique_ptr<GraphicsRenderPass>> renderPasses;

   protected:
    Info* info;
    VkCore* vkCore;
    XrCore* xrCore;
    GLFWwindow* window;

   private:
    void InitVulkan();

    const std::vector<const char*> validataionLayers = {
        "VK_LAYER_KHRONOS_validation"};
    VkDebugUtilsMessengerEXT vkDebugMessenger{};

    PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT{nullptr};
};
