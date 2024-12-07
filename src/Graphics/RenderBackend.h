#pragma once

#include "Graphics/Vulkan/CommandBuffer.h"
#include "Graphics/Vulkan/VulkanDefaults.h"
#include "Scene.h"
#include "XR/XrCore.h"

namespace XRLib {
namespace Graphics {
class RenderBackend {
   public:
    RenderBackend(Info& info, std::shared_ptr<VkCore> vkCore, XR::XrCore& xrCore, Scene& scene);
    ~RenderBackend();

    virtual bool WindowShouldClose() { return false; }

    virtual void Prepare(std::vector<std::unique_ptr<IGraphicsRenderpass>>& passes);

    virtual void OnWindowResized(int width, int height) { Util::ErrorPopup("Undefined image resize"); };

    void InitVertexIndexBuffers();

    void Run(uint32_t& imageIndex);

    bool StartFrame(uint32_t& imageIndex);
    void RecordFrame(uint32_t& imageIndex);
    void RecordFrame(uint32_t& imageIndex, std::function<void(uint32_t&, CommandBuffer&)> recordingFunction);
    void EndFrame(uint32_t& imageIndex);

    std::vector<std::unique_ptr<IGraphicsRenderpass>> RenderPasses;

    Swapchain& GetSwapchain() const { return *swapchain; }

    std::vector<std::unique_ptr<Buffer>>& GetVertexBuffers() { return vertexBuffers; }
    std::vector<std::unique_ptr<Buffer>>& GetIndexBuffers() { return indexBuffers; }

   protected:
    Info& info;
    Scene& scene;
    XR::XrCore& xrCore;
    std::shared_ptr<VkCore> vkCore;

    std::vector<std::unique_ptr<Buffer>> vertexBuffers;
    std::vector<std::unique_ptr<Buffer>> indexBuffers;

    std::unique_ptr<Swapchain> swapchain{nullptr};

   private:
    void InitVulkan();
    void GetSwapchainInfo();

   private:
    Primitives::ViewProjectionStereo viewProj;
};
}    // namespace Graphics
}    // namespace XRLib
