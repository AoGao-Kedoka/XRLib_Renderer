#pragma once

#include "Image.h"
#include "Swapchain.h"

namespace XRLib {
namespace Graphics {
class Renderpass {
   public:
    Renderpass(VkCore& core, std::vector<std::vector<Image*>>& renderTargets,
               bool multiview);
    ~Renderpass();

    VkRenderPass& GetVkRenderpass() { return pass; }
    const std::vector<VkFramebuffer>& GetFrameBuffers() { return frameBuffers; }
    void SetGraphicPipeline(VkPipeline* pipeline) { graphicsPipeline = pipeline; }

    std::vector<std::vector<Image*>>& GetRenderTargets();

   private:
    void SetRenderTarget(std::vector<std::vector<Image*>>& images);
    void CreateFramebuffer(VkFramebuffer& framebuffer, const std::unique_ptr<Image>& image, int width, int height);
    void CreateRenderPass();
    void CleanupFrameBuffers();

   private:
    VkCore& core;
    VkRenderPass pass{VK_NULL_HANDLE};
    VkPipeline* graphicsPipeline{nullptr};

    std::vector<VkFramebuffer> frameBuffers;
    Image depthImage;
    std::vector<std::vector<Image*>>& renderTargets;

    bool multiview = false;
};
}    // namespace Graphics
}    // namespace XRLib
