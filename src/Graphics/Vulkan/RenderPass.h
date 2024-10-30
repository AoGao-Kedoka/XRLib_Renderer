#pragma once

#include "Graphics/Vulkan/Image.h"
#include "Graphics/Vulkan/Swapchain.h"
#include "Logger.h"
#include "Utils/Util.h"
#include "VkCore.h"

namespace XRLib {
namespace Graphics {
class RenderPass {
   public:
    RenderPass() = default;
    RenderPass(std::shared_ptr<VkCore> core, std::vector<std::unique_ptr<Image>>& renderTargets, bool multiview);
    ~RenderPass();

    VkRenderPass& GetVkRenderPass() { return pass; }
    const std::vector<VkFramebuffer>& GetFrameBuffers() { return frameBuffers; }
    void SetGraphicPipeline(VkPipeline* pipeline) { graphicsPipeline = pipeline; }

    std::vector<std::unique_ptr<Image>>& GetRenderTargets();

   private:
    void SetRenderTarget(std::vector<std::unique_ptr<Image>>& images);
    void CreateFramebuffer(VkFramebuffer& framebuffer, const std::unique_ptr<Image>& image, int width, int height);
    void CreateRenderPass();
    void CleanupFrameBuffers();

   private:
    std::shared_ptr<VkCore> core{nullptr};
    VkRenderPass pass{VK_NULL_HANDLE};
    VkPipeline* graphicsPipeline{nullptr};

    std::vector<VkFramebuffer> frameBuffers;
    std::unique_ptr<Image> depthImage;
    std::vector<std::unique_ptr<Image>>& renderTargets;

    Swapchain* swapchainPtr{nullptr};

    bool multiview = false;
};
}    // namespace Graphics
}    // namespace XRLib
