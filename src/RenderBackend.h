#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <utility>
#include <vector>

#include "Graphics/Buffer.h"
#include "Graphics/Pipeline.h"
#include "Graphics/RenderPass.h"
#include "Graphics/Shader.h"
#include "Graphics/VkCore.h"
#include "Utils/Info.h"
#include "Logger.h"
#include "Scene.h"
#include "XR/XrCore.h"

class RenderBackend {
   public:
    RenderBackend(std::shared_ptr<Info> info, std::shared_ptr<VkCore> vkCore,
                  std::shared_ptr<XrCore> xrCore, std::shared_ptr<Scene> scene);
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

    virtual bool WindowShouldClose() { return false; }

    virtual void
    Prepare(std::vector<std::pair<const std::string&, const std::string&>>
                passesToAdd);


    virtual void OnWindowResized() {
        Util::ErrorPopup("Undefined image resize");
    };

    void InitVertexIndexBuffers();
    virtual void InitFrameBuffer();

    void Run();

    struct GraphicsRenderPass {
        GraphicsRenderPass(std::shared_ptr<VkCore> core, bool multiview,
                           std::string vertexShaderPath = "",
                           std::string fragmentShaderPath = "")
            : core{core} {
            Shader vertexShader{core, vertexShaderPath, Shader::VERTEX_SHADER};
            Shader fragmentShader{core, fragmentShaderPath,
                                  Shader::FRAGMENT_SHADER};
            renderPass = std::make_shared<RenderPass>(core, multiview);

            pipeline = std::make_shared<Pipeline>(core, std::move(vertexShader),
                                                  std::move(fragmentShader),
                                                  renderPass);
        }

        std::shared_ptr<VkCore> core;
        std::shared_ptr<RenderPass> renderPass;
        std::shared_ptr<Pipeline> pipeline;
    };

    std::vector<std::unique_ptr<GraphicsRenderPass>> renderPasses;

   protected:
    std::shared_ptr<Info> info;
    std::shared_ptr<VkCore> vkCore;
    std::shared_ptr<XrCore> xrCore;
    std::shared_ptr<Scene> scene;

    GLFWwindow* window;

    std::vector<std::unique_ptr<Buffer>> vertexBuffers;
    std::vector<std::unique_ptr<Buffer>> indexBuffers;

   private:
    void InitVulkan();
    void GetSwapchainInfo();

    const std::vector<const char*> validataionLayers = {
        "VK_LAYER_KHRONOS_validation"};
    VkDebugUtilsMessengerEXT vkDebugMessenger{};

    PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT{nullptr};
};
