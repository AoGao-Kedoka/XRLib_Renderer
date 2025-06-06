#pragma once

#include "pch.h"

#include "Graphics/RenderBackend.h"
#include "Graphics/RenderBackendFlat.h"
#include "Scene/Scene.h"
#include "XR/XrBackend.h"
#include "Utils/Time.h"

namespace XRLib {
class XRLib {
   public:
    XRLib();
    ~XRLib();
    XRLib& SetApplicationName(const std::string& applicationName);
    XRLib& SetVersionNumber(unsigned int majorVersion, unsigned int minorVersion, unsigned int patchVersion);
    XRLib& EnableValidationLayer();
    XRLib& SetCustomOpenXRRuntime(const std::filesystem::path& runtimePath);
    XRLib& Init(bool xr = true, std::unique_ptr<Graphics::StandardRB> renderBahavior = nullptr);
    XRLib& InitDefaultRenderPasses();

    void Run();
    bool ShouldStop();
    XRLib& SetWindowProperties(Graphics::WindowHandler::WindowMode windowMode, bool resizable);
    XRLib& SetWindowProperties(Graphics::WindowHandler::WindowMode windowMode, bool resizable, unsigned int width, unsigned int height);
    Scene& SceneBackend();
    Graphics::RenderBackend& RenderBackend();
    XR::XrBackend& XrBackend();

    Graphics::VkCore& GetVkCore() { return vkCore; }
    XR::XrCore& GetXrCore() { return xrCore; }

   private:
    void UpdateDeltaTime();

   private:
    Config info;
    Scene scene;
    Graphics::VkCore vkCore;
    XR::XrCore xrCore;
    std::unique_ptr<XR::XrBackend> xrBackend{nullptr};
    std::unique_ptr<Graphics::RenderBackend> renderBackend{nullptr};
    bool initialized = false;

    void InitXRBackend();
    void InitRenderBackend();

    std::chrono::steady_clock::time_point prevTime{std::chrono::steady_clock::now()};
};

static XRLib* instance;
}    // namespace XRLib
