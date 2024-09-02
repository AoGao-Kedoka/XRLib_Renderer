#pragma once

#include "Graphics/Vulkan/Buffer.h"
#include "Graphics/Vulkan/GraphicsRenderPass.h"
#include "Graphics/Vulkan/Pipeline.h"
#include "Graphics/Vulkan/RenderPass.h"
#include "Graphics/Vulkan/Shader.h"
#include "Graphics/Vulkan/VkCore.h"
#include "Graphics/Window.h"
#include "Logger.h"
#include "Scene.h"
#include "Utils/Event.h"
#include "Utils/Info.h"
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

    virtual void OnWindowResized(int width, int height) {
        Util::ErrorPopup("Undefined image resize");
    };

    void InitVertexIndexBuffers();
    virtual void InitFrameBuffer();

    void Run(uint32_t& imageIndex);

    std::vector<std::shared_ptr<GraphicsRenderPass>> renderPasses;

   protected:
    std::shared_ptr<Info> info;
    std::shared_ptr<VkCore> vkCore;
    std::shared_ptr<XrCore> xrCore;
    std::shared_ptr<Scene> scene;

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
