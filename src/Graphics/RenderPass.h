#pragma once
#include "Logger.h"
#include "Util.h"
#include "VkCore.h"

class RenderPass {
   public:
    RenderPass() = default;
    RenderPass(std::shared_ptr<VkCore> core);
    ~RenderPass();

    RenderPass(RenderPass&& other) noexcept
        : core(std::exchange(other.core, nullptr)),
          pass(std::exchange(other.pass, VK_NULL_HANDLE)),
          graphicsPipeline(
              std::exchange(other.graphicsPipeline, VK_NULL_HANDLE)) {
    }

    RenderPass& operator=(RenderPass&& other) noexcept {
        if (this != &other) {
            core = std::exchange(other.core, nullptr);
            pass = std::exchange(other.pass, VK_NULL_HANDLE);
            graphicsPipeline =
                std::exchange(other.graphicsPipeline, VK_NULL_HANDLE);
        }
        return *this;
    }

    VkRenderPass& GetRenderPass() { return pass; }
    void Record(uint32_t imageIndex);
    void SetGraphicPipeline(VkPipeline* pipeline) {
        graphicsPipeline = pipeline;
    }

   private:
    std::shared_ptr<VkCore> core{nullptr};
    VkRenderPass pass{VK_NULL_HANDLE};
    VkPipeline* graphicsPipeline{nullptr};
};
