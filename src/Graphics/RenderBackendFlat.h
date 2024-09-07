#pragma once

#include "Graphics/Window.h"
#include "RenderBackend.h"
#include "Utils/Util.h"

namespace XRLib {
namespace Graphics {
class RenderBackendFlat : public RenderBackend {
   public:
    RenderBackendFlat(std::shared_ptr<Info> info, std::shared_ptr<VkCore> core,
                      std::shared_ptr<XRLib::XR::XrCore> xrCore,
                      std::shared_ptr<XRLib::Scene> scene)
        : RenderBackend(info, core, xrCore, scene) {}
    ~RenderBackendFlat();

    RenderBackendFlat(RenderBackendFlat&& other) noexcept
        : RenderBackend(std::move(other)) {
        LOGGER(LOGGER::DEBUG) << "RenderBackendFlat move constructor called";
    }

    RenderBackendFlat& operator=(RenderBackendFlat&& rhs) noexcept {
        return *this;
    }

    void Prepare(std::vector<std::pair<const std::string&, const std::string&>>
                     passesToAdd) override;

    void OnWindowResized(int width, int height) override;
    bool WindowShouldClose() override {
        return WindowHandler::WindowShouldClose();
    }

   private:
    void CreateFlatSwapChain();
    void PrepareFlatWindow();

};
}    // namespace Graphics
}    // namespace XRLib
