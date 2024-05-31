#pragma once

#include "RenderPass.h"
#include "Shader.h"
#include "Util.h"
#include "VkCore.h"

class Pipeline {
   public:
    // TODO: compute pipeline
    Pipeline() = default;
    Pipeline(VkCore* core, Shader vertexShader, Shader fragmentShader,
             RenderPass* pass);
    ~Pipeline();

    Pipeline(Pipeline&& other) noexcept
        : core(std::exchange(other.core, nullptr)),
          pipeline(std::exchange(other.pipeline, VK_NULL_HANDLE)),
          pipelineLayout(std::exchange(other.pipelineLayout, VK_NULL_HANDLE)) {}

    Pipeline& operator=(Pipeline&& other) noexcept {
        if (this != &other) {
            core = std::exchange(other.core, nullptr);
            pipeline = std::exchange(other.pipeline, VK_NULL_HANDLE);
            pipelineLayout =
                std::exchange(other.pipelineLayout, VK_NULL_HANDLE);
        }
        return *this;
    }

    VkPipeline& GetVkPipeline() { return pipeline; }

   private:
    VkCore* core = nullptr;
    VkPipeline pipeline{VK_NULL_HANDLE};
    VkPipelineLayout pipelineLayout{VK_NULL_HANDLE};
};
