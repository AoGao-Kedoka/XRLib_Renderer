#pragma once

#include "Core.h"
#include "Shader.h"
#include "Util.h"

class Pipeline {
   public:
    // TODO: compute pipeline
    Pipeline() = default;
    Pipeline(Core* core, Shader vertexShader, Shader fragmentShader,
             VkRenderPass renderPass);
    ~Pipeline();

   private:
    Core* core = nullptr;
    VkPipeline pipeline{VK_NULL_HANDLE};
    VkPipelineLayout pipelineLayout{VK_NULL_HANDLE};
};
