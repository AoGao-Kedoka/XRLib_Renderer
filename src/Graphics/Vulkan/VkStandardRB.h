#pragma once

#include "Buffer.h"
#include "CommandBuffer.h"
#include "Scene/Scene.h"
#include "Graphics/StandardRB.h"
#include "Swapchain.h"
#include "VkGraphicsRenderpass.h"

// Vulkan Standard Rendering Behavior
namespace XRLib {
namespace Graphics {
class VkStandardRB : StandardRB {
   public:
    VkStandardRB(VkCore& core, Scene& scene);
    ~VkStandardRB() = default;
    ////////////////////////////////////////////////////
    // Shaders
    ////////////////////////////////////////////////////
    static const std::string_view defaultVertFlat;
    static const std::string_view defaultPhongFrag;
    static const std::string_view defaultVertStereo;

    constexpr static std::string_view defaultShaderCachePath = "./ShaderCache";

    ////////////////////////////////////////////////////
    // Default render passes
    ////////////////////////////////////////////////////

    void PrepareDefaultStereoRenderPasses(Primitives::ViewProjectionStereo& viewProj,
                                          std::vector<std::unique_ptr<IGraphicsRenderpass>>& renderPasses);
    void PrepareDefaultFlatRenderPasses(Primitives::ViewProjection& viewProj,
                                        std::vector<std::unique_ptr<IGraphicsRenderpass>>& renderPasses);

    void InitVerticesIndicesShader();

    std::unique_ptr<Swapchain>& GetSwapchain() { return swapchain; }
    bool StartFrame(uint32_t& imageIndex) override;
    void RecordFrame(uint32_t& imageIndex) override;
    void EndFrame(uint32_t& imageIndex) override;

   private:
    void PrepareDefaultRenderPasses(std::vector<std::vector<std::unique_ptr<Image>>>& swapchainImages, bool isStereo,
                                    std::shared_ptr<Buffer> viewProjBuffer);

   private:
    VkCore& core;
    Scene& scene;

    std::vector<std::unique_ptr<IGraphicsRenderpass>> renderPasses;
    std::vector<std::unique_ptr<Buffer>> vertexBuffers;
    std::vector<std::unique_ptr<Buffer>> indexBuffers;
    std::unique_ptr<Swapchain> swapchain;
};
}    // namespace Graphics
}    // namespace XRLib
