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

    XRLib& InitXRBackend();
    XRLib& InitRenderBackend();

    std::pair<XRBackend&, RenderBackend&> GetBackend() const {
        return std::pair<XRBackend&, RenderBackend&>(*xrBackend,
                                                     *renderBackend);
    }

    XRBackend& GetXRBackend() const { return *xrBackend; }
    RenderBackend& GetRenderBackend() const { return *renderBackend; }

   private:
    Info info{};

    std::shared_ptr<XRBackend> xrBackend{nullptr};
    std::shared_ptr<RenderBackend> renderBackend{nullptr};
};