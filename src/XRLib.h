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
    XRLib& Init(bool xr = true);

    XRLib& AddRenderPass(const std::string& vertexShaderPath,
                         const std::string& fragmentShaderPath);
    void Run();

    bool WindowShouldClose(){ return renderBackend->WindowShouldClose(); }
    XRLib& Fullscreen();

   private:
    std::shared_ptr<Info> info{std::make_shared<Info>()};
    std::shared_ptr<VkCore> vkCore{std::make_shared<VkCore>()};
    std::shared_ptr<XrCore> xrCore{std::make_shared<XrCore>()};

    void InitXRBackend();
    void InitRenderBackend();

    std::unique_ptr<XRBackend> xrBackend{nullptr};
    std::unique_ptr<RenderBackend> renderBackend{nullptr};

    std::vector<std::pair<const std::string&, const std::string&>> passesToAdd;
    bool initialized = false;
};
}    // namespace XRLib
