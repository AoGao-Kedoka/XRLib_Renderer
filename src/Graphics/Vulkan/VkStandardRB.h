#pragma once

#include "Buffer.h"
#include "CommandBuffer.h"
#include "Graphics/StandardRB.h"
#include "Scene/Scene.h"
#include "Swapchain.h"
#include "VkGraphicsRenderpass.h"

// Vulkan Standard Rendering Behavior
namespace XRLib {
namespace Graphics {
class VkStandardRB : public StandardRB {
   public:
    VkStandardRB(VkCore& core, Scene& scene, std::vector<std::unique_ptr<IGraphicsRenderpass>>& renderPasses);
    ~VkStandardRB() = default;

    ////////////////////////////////////////////////////
    // Shaders
    ////////////////////////////////////////////////////

    static const std::string_view defaultVertFlat;
    static const std::string_view defaultPhongFrag;
    static const std::string_view defaultVertStereo;

    inline constexpr static std::string_view defaultShaderCachePath = "./ShaderCache";

    ////////////////////////////////////////////////////
    // Default render passes
    ////////////////////////////////////////////////////

    virtual void PrepareDefaultStereoRenderPasses(Primitives::ViewProjectionStereo& viewProj,
                                                  std::vector<std::unique_ptr<IGraphicsRenderpass>>& renderPasses);
    virtual void PrepareDefaultFlatRenderPasses(Primitives::ViewProjection& viewProj,
                                                std::vector<std::unique_ptr<IGraphicsRenderpass>>& renderPasses);

    virtual void InitVerticesIndicesBuffers();

    ////////////////////////////////////////////////////
    // Frame Rendering
    ////////////////////////////////////////////////////

    std::unique_ptr<Swapchain>& GetSwapchain() { return swapchain; }
    bool StartFrame(uint32_t& imageIndex) override;
    void RecordFrame(uint32_t& imageIndex) override;
    void EndFrame(uint32_t& imageIndex) override;

   private:
    void PrepareDefaultRenderPasses(std::vector<std::vector<std::unique_ptr<Image>>>& swapchainImages,
                                    std::shared_ptr<Buffer> viewProjBuffer);

   protected:
    VkCore& core;
    Scene& scene;

    bool stereo;

    std::vector<std::unique_ptr<IGraphicsRenderpass>>& renderPasses;
    std::vector<std::unique_ptr<Buffer>> vertexBuffers;
    std::vector<std::unique_ptr<Buffer>> indexBuffers;
    std::unique_ptr<Swapchain> swapchain;
};
}    // namespace Graphics
}    // namespace XRLib
