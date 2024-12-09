#pragma once

#include "DescriptorSet.h"
#include "RenderPass.h"
#include "Shader.h"

namespace XRLib {
namespace Graphics {
class Pipeline {
   public:
    // TODO: compute pipeline
    Pipeline(VkCore& core, Shader& vertexShader, Shader& fragmentShader, Renderpass& pass,
             const std::vector<std::unique_ptr<DescriptorSet>>& descriptorSets);
    ~Pipeline();

    VkPipeline& GetVkPipeline() { return pipeline; }
    VkPipelineLayout& GetVkPipelineLayout() { return pipelineLayout; }

   private:
    VkCore& core;
    VkPipeline pipeline{VK_NULL_HANDLE};
    VkPipelineLayout pipelineLayout{VK_NULL_HANDLE};
};
}    // namespace Graphics
}    // namespace XRLib
