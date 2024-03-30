#pragma once

#include <string>

#include "Info.h"
#include "RenderBackend.h"
#include "XRBackend.h"

class XRLib {
   public:
    XRLib& SetApplicationName(std::string applicationName);
    XRLib& SetVersionNumber(unsigned int majorVersion,
                            unsigned int minorVersion,
                            unsigned int patchVersion);
    XRLib& EnableValidationLayer();

    XRLib& Init();

    std::pair<XRBackend&, RenderBackend&> GetBackend() const {
        return {*xrBackend, *renderBackend};
    }

   private:
    Info info{};
    Core core{};

    void InitXRBackend();
    void InitRenderBackend();

    std::unique_ptr<XRBackend> xrBackend{nullptr};
    std::unique_ptr<RenderBackend> renderBackend{nullptr};
};