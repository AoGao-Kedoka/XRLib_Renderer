#pragma once

#include <algorithm>

#include "RenderBackend.h"
#include "Util.h"

class RenderBackendFlat : public RenderBackend {
   public:
    RenderBackendFlat(std::shared_ptr<Info> info, std::shared_ptr<VkCore> core,
                      std::shared_ptr<XrCore> xrCore)
        : RenderBackend(info, core, xrCore) {}
    ~RenderBackendFlat();

    RenderBackendFlat(RenderBackendFlat&& other) noexcept
        : RenderBackend(std::move(other)) {
        LOGGER(LOGGER::DEBUG) << "RenderBackendFlat move constructor called";
    }

    RenderBackendFlat& operator=(RenderBackendFlat&& rhs) noexcept {
        return *this;
    }

    void Prepare(
        std::vector<std::pair<std::string, std::string>> passesToAdd) override;

    void OnWindowResized() override;


   private:
    void CreateFlatSwapChain();
    void InitFrameBuffer() override;
    void PrepareFlatWindow();
};
