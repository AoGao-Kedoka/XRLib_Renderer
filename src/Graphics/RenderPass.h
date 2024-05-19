#pragma once
#include "Core.h"
#include "Util.h"

class RenderPass {
   public:
    RenderPass() = default;
    RenderPass(Core* core);
    ~RenderPass();
    VkRenderPass& GetRenderPass() { return pass; }
    void Record(uint32_t imageIndex);
    void SetGraphicPipeline(VkPipeline pipeline) {
        graphicsPipeline = pipeline;
    }

   private:
    Core* core;
    VkRenderPass pass{VK_NULL_HANDLE};
    VkPipeline graphicsPipeline{VK_NULL_HANDLE};
};