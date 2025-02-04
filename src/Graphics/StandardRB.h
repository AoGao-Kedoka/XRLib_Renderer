#pragma once

#include "IGraphicsRenderpass.h"
#include "Scene/Scene.h"

// Standard rendering behaviour, should be ignored if custom render pass is defined
namespace XRLib {
namespace Graphics {
class StandardRB {
   public:
    StandardRB(Scene& scene, std::vector<std::unique_ptr<IGraphicsRenderpass>>* renderPasses, bool stereo)
        : stereo{stereo}, renderPasses{renderPasses}, scene{scene} {}
    virtual ~StandardRB() = default;

    virtual void Prepare() = 0;

    virtual bool StartFrame(uint32_t& imageIndex) = 0;
    virtual void RecordFrame(uint32_t& imageIndex) = 0;
    virtual void EndFrame(uint32_t& imageIndex) = 0;

    virtual void UpdateRenderPasses(std::vector<std::unique_ptr<IGraphicsRenderpass>>& renderPasses) {
        this->renderPasses = &renderPasses;
    }

   protected:
    bool stereo;
    Scene& scene;
    std::vector<std::unique_ptr<IGraphicsRenderpass>>* renderPasses;
};
}    // namespace Graphics
}    // namespace XRLib
