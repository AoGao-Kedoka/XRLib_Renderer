#pragma once

#include <algorithm>

#include "RenderBackend.h"

class RenderBackendFlat : public RenderBackend {
   public:
    RenderBackendFlat(Info& info, VkCore& core, XrCore& xrCore) : RenderBackend(info, core, xrCore) {}
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


   private:
    void CreateFlatSwapChain();
    void PrepareFlatWindow();
};
