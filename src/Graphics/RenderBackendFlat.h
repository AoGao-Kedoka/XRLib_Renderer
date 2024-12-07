#pragma once

#include "RenderBackend.h"

namespace XRLib {
namespace Graphics {
class RenderBackendFlat : public RenderBackend {
   public:
    RenderBackendFlat(Info& info, std::shared_ptr<VkCore> core, XR::XrCore& xrCore, Scene& scene);
    ~RenderBackendFlat();

    void Prepare(std::vector<std::unique_ptr<IGraphicsRenderpass>>& passes) override;

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
