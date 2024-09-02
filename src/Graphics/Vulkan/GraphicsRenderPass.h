#pragma once

#include "Pipeline.h"
#include "RenderPass.h"
#include "Shader.h"

namespace XRLib {
namespace Graphics {
class GraphicsRenderPass {
   public:
    GraphicsRenderPass(std::shared_ptr<VkCore> core, bool multiview,
                       std::string vertexShaderPath = "",
                       std::string fragmentShaderPath = "")
        : core{core}, multiview{multiview} {
        Shader vertexShader{core, vertexShaderPath, Shader::VERTEX_SHADER};
        Shader fragmentShader{core, fragmentShaderPath,
                              Shader::FRAGMENT_SHADER};
        renderPass = std::make_shared<RenderPass>(core, multiview);

        pipeline =
            std::make_shared<Pipeline>(core, std::move(vertexShader),
                                       std::move(fragmentShader), renderPass);
    }

    RenderPass& GetRenderPass() { return *renderPass; }
    Pipeline& GetPipeline() { return *pipeline; }
    bool Stereo() { return multiview; }

   private:
    std::shared_ptr<VkCore> core;
    std::shared_ptr<RenderPass> renderPass;
    std::shared_ptr<Pipeline> pipeline;
    bool multiview;
};
}    // namespace Graphics
}    // namespace XRLib
