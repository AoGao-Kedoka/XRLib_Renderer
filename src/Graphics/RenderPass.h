#pragma once
#include "Core.h"
#include "Logger.h"
#include "Util.h"

class RenderPass {
   public:
    RenderPass() = default;
    RenderPass(Core* core);
    ~RenderPass();

    RenderPass(const RenderPass& other) {
        LOGGER(LOGGER::DEBUG) << "RenderPass copy constructor called";
    }

    RenderPass(RenderPass&& other) noexcept
        : core(std::exchange(other.core, nullptr)),
          pass(std::exchange(other.pass, VK_NULL_HANDLE)),
          graphicsPipeline(
              std::exchange(other.graphicsPipeline, VK_NULL_HANDLE)) {
        LOGGER(LOGGER::DEBUG) << "RenderPass move constructor called";
    }

    RenderPass& operator=(RenderPass&& other) noexcept {
        if (this != &other) {
            core = std::exchange(other.core, nullptr);
            pass = std::exchange(other.pass, VK_NULL_HANDLE);
            graphicsPipeline =
                std::exchange(other.graphicsPipeline, VK_NULL_HANDLE);
        }
        return *this;
        LOGGER(LOGGER::DEBUG) << "RenderPass move operation called";
    }

    VkRenderPass& GetRenderPass() { return pass; }
    void Record(uint32_t imageIndex);
    void SetGraphicPipeline(VkPipeline& pipeline) {
        graphicsPipeline = &pipeline;
    }

   private:
    Core* core;
    VkRenderPass pass{VK_NULL_HANDLE};
    VkPipeline* graphicsPipeline{nullptr};
};