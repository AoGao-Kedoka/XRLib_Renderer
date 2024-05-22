#pragma once

#include "Core.h"
#include "RenderPass.h"
#include "Shader.h"
#include "Util.h"

class Pipeline {
   public:
    // TODO: compute pipeline
    Pipeline() = default;
    Pipeline(Core* core, Shader vertexShader, Shader fragmentShader,
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

   private:
    Core* core = nullptr;
    VkPipeline pipeline{VK_NULL_HANDLE};
    VkPipelineLayout pipelineLayout{VK_NULL_HANDLE};
};
