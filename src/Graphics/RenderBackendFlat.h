#pragma once

#include "RenderBackend.h"

namespace XRLib {
namespace Graphics {
class RenderBackendFlat : public RenderBackend {
   public:
    RenderBackendFlat(std::shared_ptr<Info> info, std::shared_ptr<VkCore> core,
                      std::shared_ptr<XR::XrCore> xrCore, std::shared_ptr<Scene> scene)
        : RenderBackend(info, core, xrCore, scene) {}
    ~RenderBackendFlat();

    void Prepare(std::vector<std::unique_ptr<GraphicsRenderPass>>& passes) override;

    void OnWindowResized(int width, int height) override;
    bool WindowShouldClose() override { return WindowHandler::WindowShouldClose(); }

   private:
    void OnMouseMovement(double deltaX, double deltaY);
    void OnKeyPressed(int keyCode);
    void CreateFlatSwapChain();
    void PrepareFlatWindow();

   private:
    Primitives::ViewProjection viewProj;
};
}    // namespace Graphics
}    // namespace XRLib
