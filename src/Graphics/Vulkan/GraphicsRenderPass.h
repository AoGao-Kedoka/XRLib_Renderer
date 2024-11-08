#pragma once

#include "DescriptorSet.h"
#include "Pipeline.h"
#include "RenderPass.h"
#include "Shader.h"

namespace XRLib {
namespace Graphics {
class GraphicsRenderPass {
   public:
    GraphicsRenderPass(std::shared_ptr<VkCore> core, bool multiview, std::vector<std::unique_ptr<Image>>& renderTargets,
                       std::vector<std::unique_ptr<DescriptorSet>>&& descriptorSets = {},
                       std::string vertexShaderPath = "", std::string fragmentShaderPath = "")
        : core{core}, multiview{multiview}, descriptorSets{std::move(descriptorSets)}{
        Shader vertexShader{core, vertexShaderPath, Shader::VERTEX_SHADER, multiview};
        Shader fragmentShader{core, fragmentShaderPath, Shader::FRAGMENT_SHADER, multiview};
        renderPass = std::make_unique<RenderPass>(core, renderTargets, multiview);
        pipeline = std::make_unique<Pipeline>(core, std::move(vertexShader), std::move(fragmentShader), *renderPass,
                                              this->descriptorSets);
    }

    RenderPass& GetRenderPass() { return *renderPass; }
    Pipeline& GetPipeline() { return *pipeline; }
    std::vector<std::unique_ptr<DescriptorSet>>& GetDescriptorSets() { return descriptorSets; }
    bool Stereo() { return multiview; }

   private:

   private:
    std::shared_ptr<VkCore> core;

    std::unique_ptr<RenderPass> renderPass;
    std::unique_ptr<Pipeline> pipeline;
    std::vector<std::unique_ptr<DescriptorSet>> descriptorSets;
    bool multiview;
};
}    // namespace Graphics
}    // namespace XRLib
