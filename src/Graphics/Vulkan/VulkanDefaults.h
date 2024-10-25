#pragma once
#include "Graphics/Vulkan/Buffer.h"
#include "Graphics/Vulkan/GraphicsRenderPass.h"
#include "Graphics/Vulkan/Swapchain.h"
#include "Graphics/Vulkan/VkCore.h"
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
                                                 std::vector<std::unique_ptr<GraphicsRenderPass>>& renderPasses);
    static void PrepareDefaultFlatRenderPasses(std::shared_ptr<VkCore> core, std::shared_ptr<Scene> scene,
                                               Primitives::ViewProjection& viewProj,
                                               std::vector<std::unique_ptr<GraphicsRenderPass>>& renderPasses,
                                               Swapchain& swapchain);
};
}    // namespace Graphics
}    // namespace XRLib
