#pragma once

#include <string>
#include <memory>

#include "Info.h"
#include "RenderBackendFlat.h"
#include "XRBackend.h"

namespace XRLib {
class XRLib {
   public:
    XRLib& SetApplicationName(std::string applicationName);
    XRLib& SetVersionNumber(unsigned int majorVersion,
                            unsigned int minorVersion,
                            unsigned int patchVersion);
    XRLib& EnableValidationLayer();
    XRLib& Init(bool xr);

    XRLib& AddRenderPass(std::string vertexShaderPath, std::string fragmentShaderPath);
    XRLib& Run();
   private:
    Info info{};
    Core core{};

    void InitXRBackend();
    void InitRenderBackend();

    std::unique_ptr<XRBackend> xrBackend{nullptr};
    std::shared_ptr<RenderBackend> renderBackend{nullptr};

    std::vector<std::pair<std::string, std::string>> passesToAdd;
    bool initialized = false;
};
}    // namespace XRLib
