#pragma once

#include "RenderBackend.h"

namespace XRLib {
namespace Graphics {
class RenderBackendFlat : public RenderBackend {
   public:
    RenderBackendFlat(Config& info, VkCore& core, XR::XrCore& xrCore, Scene& scene);
    ~RenderBackendFlat();

    void Prepare() override;

    bool WindowShouldClose() override { return WindowHandler::WindowShouldClose(); }

   private:
    void OnWindowResized(int width, int height);
    void OnMouseMovement(double deltaX, double deltaY);
    void OnKeyPressed(int keyCode);
    void CreateFlatSwapChain();
    void PrepareFlatWindow();

   private:
    Primitives::ViewProjection viewProj;
};
}    // namespace Graphics
}    // namespace XRLib
