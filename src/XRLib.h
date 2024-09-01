#pragma once

#include "Graphics/RenderBackend.h"
#include "Graphics/RenderBackendFlat.h"
#include "Scene.h"
#include "Utils/Info.h"
#include "XR/XRBackend.h"

namespace XRLib {
class XRLib {
   public:
    XRLib();
    XRLib& SetApplicationName(std::string applicationName);
    XRLib& SetVersionNumber(unsigned int majorVersion,
                            unsigned int minorVersion,
                            unsigned int patchVersion);
    XRLib& EnableValidationLayer();
    XRLib& SetCustomOpenXRRuntime(const std::filesystem::path& runtimePath);
    void Init(bool xr = true);

    XRLib& AddRenderPass(const std::string& vertexShaderPath,
                         const std::string& fragmentShaderPath);
    void Run();

    bool ShouldStop();
    XRLib& Fullscreen();

    Scene& SceneBackend() { return *scene; }

   private:
    std::shared_ptr<Info> info{std::make_shared<Info>()};
    std::shared_ptr<VkCore> vkCore{std::make_shared<VkCore>()};
    std::shared_ptr<XrCore> xrCore{std::make_shared<XrCore>()};
    std::shared_ptr<Scene> scene{std::make_shared<Scene>()};

    void InitXRBackend();
    void InitRenderBackend();

    std::unique_ptr<XrBackend> xrBackend{nullptr};
    std::unique_ptr<RenderBackend> renderBackend{nullptr};

    std::vector<std::pair<const std::string&, const std::string&>> passesToAdd;
    bool initialized = false;
};
}    // namespace XRLib
