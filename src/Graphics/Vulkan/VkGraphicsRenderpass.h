#pragma once

#include "DescriptorSet.h"
#include "Graphics/IGraphicsRenderpass.h"
#include "Pipeline.h"
#include "RenderPass.h"
#include "Shader.h"

namespace XRLib {
namespace Graphics {
class VkGraphicsRenderpass : public IGraphicsRenderpass {
   public:
    // renderTargets: 
    // first level vector: per image frame, second level vector: attachments per image frame
    VkGraphicsRenderpass(VkCore& core, bool multiview,
                         std::vector<std::vector<Image*>>& renderTargets,
                         std::vector<std::unique_ptr<DescriptorSet>>&& descriptorSets = {},
                         std::string vertexShaderPath = "", std::string fragmentShaderPath = "")

        : core{core}, multiview{multiview}, descriptorSets{std::move(descriptorSets)} {
        Shader vertexShader{core, vertexShaderPath, Shader::VERTEX_SHADER, multiview};
        Shader fragmentShader{core, fragmentShaderPath, Shader::FRAGMENT_SHADER, multiview};
        renderPass = std::make_unique<Renderpass>(core, renderTargets, multiview);
        pipeline = std::make_unique<Pipeline>(core, vertexShader, fragmentShader, *renderPass,
                                              this->descriptorSets);
    }

    Renderpass& GetRenderpass() { return *renderPass; }
    Pipeline& GetPipeline() { return *pipeline; }
    std::vector<std::unique_ptr<DescriptorSet>>& GetDescriptorSets() { return descriptorSets; }
    bool Stereo() { return multiview; }

   private:
    VkCore& core;
    std::unique_ptr<Renderpass> renderPass;
    std::unique_ptr<Pipeline> pipeline;
    std::vector<std::unique_ptr<DescriptorSet>> descriptorSets;
    bool multiview;
};
}    // namespace Graphics
}    // namespace XRLib
