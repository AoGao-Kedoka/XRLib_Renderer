#pragma once

#include "Graphics/RenderBackend.h"
#include "Graphics/RenderBackendFlat.h"
#include "Graphics/Window.h"
#include "Scene.h"
#include "XR/XRBackend.h"
#include "pch.h"

namespace XRLib {
class XRLib {
   public:
    XRLib();
    ~XRLib();
    XRLib& SetApplicationName(std::string applicationName);
    XRLib& SetVersionNumber(unsigned int majorVersion, unsigned int minorVersion, unsigned int patchVersion);
    XRLib& EnableValidationLayer();
    XRLib& SetCustomOpenXRRuntime(const std::filesystem::path& runtimePath);
    void Init(bool xr = true);
    XRLib& AddRenderPass(const std::string& vertexShaderPath, const std::string& fragmentShaderPath);
    void Run();
    bool ShouldStop();
    XRLib& Fullscreen();
    Scene& SceneBackend();
    Graphics::RenderBackend& RenderBackend();
    XR::XrBackend& XrBackend();

   private:
    std::shared_ptr<Info> info;
    std::shared_ptr<Graphics::VkCore> vkCore;
    std::shared_ptr<XR::XrCore> xrCore;
    std::shared_ptr<Scene> scene;
    std::unique_ptr<XR::XrBackend> xrBackend{nullptr};
    std::unique_ptr<Graphics::RenderBackend> renderBackend{nullptr};
    std::vector<std::pair<const std::string&, const std::string&>> passesToAdd;
    bool initialized = false;

    void InitXRBackend();
    void InitRenderBackend();
};
}    // namespace XRLib
