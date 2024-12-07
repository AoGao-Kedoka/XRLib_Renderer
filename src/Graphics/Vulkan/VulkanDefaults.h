#pragma once

#include "Buffer.h"
#include "Scene.h"
#include "Swapchain.h"
#include "VkGraphicsRenderpass.h"

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
    static void PrepareDefaultStereoRenderPasses(std::shared_ptr<VkCore> core, Scene& scene,
                                                 Primitives::ViewProjectionStereo& viewProj,
                                                 std::vector<std::unique_ptr<IGraphicsRenderpass>>& renderPasses,
                                                 std::vector<std::vector<std::unique_ptr<Image>>>& swapchainImages);
    static void PrepareDefaultFlatRenderPasses(std::shared_ptr<VkCore> core, Scene& scene,
                                               Primitives::ViewProjection& viewProj,
                                               std::vector<std::unique_ptr<IGraphicsRenderpass>>& renderPasses,
                                               std::vector<std::vector<std::unique_ptr<Image>>>& swapchainImages);
};
}    // namespace Graphics
}    // namespace XRLib
