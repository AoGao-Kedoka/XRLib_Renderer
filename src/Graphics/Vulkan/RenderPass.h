#pragma once

#include "Graphics/Vulkan/Image.h"
#include "Logger.h"
#include "Utils/Util.h"
#include "VkCore.h"

namespace XRLib {
namespace Graphics {
class RenderPass {
   public:
    RenderPass() = default;
    RenderPass(std::shared_ptr<VkCore> core, bool multiview);
    ~RenderPass();

    RenderPass(RenderPass&& other) noexcept
        : core(std::exchange(other.core, nullptr)), pass(std::exchange(other.pass, VK_NULL_HANDLE)),
          graphicsPipeline(std::exchange(other.graphicsPipeline, VK_NULL_HANDLE)), multiview{other.multiview} {}

    RenderPass& operator=(RenderPass&& other) noexcept {
        if (this != &other) {
            core = std::exchange(other.core, nullptr);
            pass = std::exchange(other.pass, VK_NULL_HANDLE);
            graphicsPipeline = std::exchange(other.graphicsPipeline, VK_NULL_HANDLE);
            multiview = other.multiview;
        }
        return *this;
    }

    void SetRenderTarget(std::vector<std::unique_ptr<Image>> images);
    VkRenderPass& GetVkRenderPass() { return pass; }
    void SetGraphicPipeline(VkPipeline* pipeline) { graphicsPipeline = pipeline; }

   private:
    std::shared_ptr<VkCore> core{nullptr};
    VkRenderPass pass{VK_NULL_HANDLE};
    VkPipeline* graphicsPipeline{nullptr};

    std::vector<VkFramebuffer> frameBuffers;
    std::unique_ptr<Image> depthImage;

    bool multiview = false;
};
}    // namespace Graphics
}    // namespace XRLib
