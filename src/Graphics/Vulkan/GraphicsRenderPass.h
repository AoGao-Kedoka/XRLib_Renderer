#pragma once

#include "DescriptorSet.h"
#include "Pipeline.h"
#include "RenderPass.h"
#include "Shader.h"

namespace XRLib {
namespace Graphics {
class GraphicsRenderPass {
   public:
    GraphicsRenderPass(std::shared_ptr<VkCore> core, bool multiview,
                       std::shared_ptr<DescriptorSet> descriptorSet = nullptr, std::string vertexShaderPath = "",
                       std::string fragmentShaderPath = "")
        : core{core}, multiview{multiview}, descriptorSet{descriptorSet} {
        Shader vertexShader{core, vertexShaderPath, Shader::VERTEX_SHADER, multiview};
        Shader fragmentShader{core, fragmentShaderPath, Shader::FRAGMENT_SHADER, multiview};
        renderPass = std::make_shared<RenderPass>(core, multiview);

        pipeline = std::make_shared<Pipeline>(core, std::move(vertexShader), std::move(fragmentShader), renderPass,
                                              descriptorSet);
    }

    RenderPass& GetRenderPass() { return *renderPass; }
    Pipeline& GetPipeline() { return *pipeline; }
    std::shared_ptr<DescriptorSet> GetDescriptorSet() { return descriptorSet; }
    bool Stereo() { return multiview; }

   private:
    std::shared_ptr<VkCore> core;
    std::shared_ptr<RenderPass> renderPass;
    std::shared_ptr<Pipeline> pipeline;
    std::shared_ptr<DescriptorSet> descriptorSet;
    bool multiview;
};
}    // namespace Graphics
}    // namespace XRLib
