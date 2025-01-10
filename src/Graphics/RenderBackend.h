#pragma once

#include "Graphics/Vulkan/CommandBuffer.h"
#include "Graphics/Vulkan/VkStandardRB.h"
#include "Scene/Scene.h"
#include "XR/XrCore.h"

namespace XRLib {
namespace Graphics {
class RenderBackend {
   public:
    RenderBackend(Info& info, VkCore& vkCore, XR::XrCore& xrCore, Scene& scene);
    ~RenderBackend();

    virtual bool WindowShouldClose() { return false; }

    virtual void Prepare();
    virtual void Prepare(std::vector<std::unique_ptr<IGraphicsRenderpass>>& passes);

    Swapchain& GetSwapchain() { return *vkSRB.GetSwapchain(); }
    bool StartFrame(uint32_t& imageIndex);
    void RecordFrame(uint32_t& imageIndex);
    void RecordFrame(uint32_t& imageIndex, std::function<void(uint32_t&, CommandBuffer&)> recordingFunction);
    void EndFrame(uint32_t& imageIndex);

    std::vector<std::unique_ptr<IGraphicsRenderpass>> RenderPasses;

   protected:
    Info& info;
    Scene& scene;
    XR::XrCore& xrCore;
    VkCore& vkCore;

    VkStandardRB vkSRB{vkCore, scene, RenderPasses};

   private:
    void InitVulkan();
    void GetSwapchainInfo();

   private:
    Primitives::ViewProjectionStereo viewProj;
};
}    // namespace Graphics
}    // namespace XRLib
