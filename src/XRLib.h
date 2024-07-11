#pragma once

#include <string>
#include <memory>

#include "Utils/Info.h"
#include "RenderBackend.h"
#include "RenderBackendFlat.h"
#include "Scene.h"
#include "XRBackend.h"

namespace XRLib {
class XRLib {
   public:
    XRLib& SetApplicationName(std::string applicationName);
    XRLib& SetVersionNumber(unsigned int majorVersion,
                            unsigned int minorVersion,
                            unsigned int patchVersion);
    XRLib& EnableValidationLayer();
    void Init(bool xr = true);

    XRLib& AddRenderPass(const std::string& vertexShaderPath,
                         const std::string& fragmentShaderPath);
    void Run();

    bool WindowShouldClose(){ return renderBackend->WindowShouldClose(); }
    XRLib& Fullscreen();

    Scene& SceneBackend() { return *scene; }

   private:
    std::shared_ptr<Info> info{std::make_shared<Info>()};
    std::shared_ptr<VkCore> vkCore{std::make_shared<VkCore>()};
    std::shared_ptr<XrCore> xrCore{std::make_shared<XrCore>()};
    std::shared_ptr<Scene> scene{std::make_shared<Scene>()};

    void InitXRBackend();
    void InitRenderBackend();

    std::unique_ptr<XRBackend> xrBackend{nullptr};
    std::unique_ptr<RenderBackend> renderBackend{nullptr};

    std::vector<std::pair<const std::string&, const std::string&>> passesToAdd;
    bool initialized = false;
};
}    // namespace XRLib
