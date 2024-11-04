#pragma once

#include "Buffer.h"
#include "GraphicsRenderPass.h"
#include "Swapchain.h"
#include "Scene.h"

namespace XRLib {
namespace Graphics {
class VulkanDefaults {
   public:
    ////////////////////////////////////////////////////
    // Shaders
    ////////////////////////////////////////////////////
    static const std::string defaultVertFlat;
    static const std::string defaultPhongFrag;
    static const std::string defaultVertStereo;

    ////////////////////////////////////////////////////
    // Default render passes
    ////////////////////////////////////////////////////
    static void PrepareDefaultStereoRenderPasses(std::shared_ptr<VkCore> core, std::shared_ptr<Scene> scene,
                                                 Primitives::ViewProjectionStereo& viewProj,
                                                 std::vector<std::unique_ptr<GraphicsRenderPass>>& renderPasses,
                                                 std::vector<std::unique_ptr<Image>>& swapchainImages);
    static void PrepareDefaultFlatRenderPasses(std::shared_ptr<VkCore> core, std::shared_ptr<Scene> scene,
                                               Primitives::ViewProjection& viewProj,
                                               std::vector<std::unique_ptr<GraphicsRenderPass>>& renderPasses,
                                               std::vector<std::unique_ptr<Image>>& swapchainImages);

   private:
    std::vector<std::unique_ptr<DescriptorSet>> descriptorSets;
};
}    // namespace Graphics
}    // namespace XRLib
