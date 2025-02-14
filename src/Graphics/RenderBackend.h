#pragma once

#include "Graphics/Vulkan/CommandBuffer.h"
#include "Graphics/Vulkan/VkStandardRB.h"
#include "Scene/Scene.h"
#include "XR/XrCore.h"

namespace XRLib {
namespace Graphics {
class RenderBackend {
   public:
    RenderBackend(Config& info, VkCore& vkCore, XR::XrCore& xrCore, Scene& scene);
    ~RenderBackend();

    virtual bool WindowShouldClose() { return false; }

    virtual void Prepare();

    bool StartFrame(uint32_t& imageIndex);
    void RecordFrame(uint32_t& imageIndex);
    void RecordFrame(uint32_t& imageIndex, std::function<void(uint32_t&, CommandBuffer&)> recordingFunction);
    void EndFrame(uint32_t& imageIndex);

    void SetRenderBehavior(std::unique_ptr<StandardRB>& newRenderBahavior) {
        this->renderBehavior = std::move(newRenderBahavior);
        renderBehavior->UpdateRenderPasses(RenderPasses);
    }

    std::vector<std::unique_ptr<IGraphicsRenderpass>> RenderPasses;

   protected:
    Config& info;
    Scene& scene;
    XR::XrCore& xrCore;
    VkCore& vkCore;

    std::unique_ptr<StandardRB> renderBehavior =
        std::make_unique<VkStandardRB>(vkCore, scene, &RenderPasses, xrCore.IsXRValid());

   private:
    void InitVulkan();
    void GetSwapchainConfig();

   private:
    Primitives::ViewProjectionStereo viewProj;
};
}    // namespace Graphics
}    // namespace XRLib
