#pragma once

#include "pch.h"

#include "Graphics/RenderBackend.h"
#include "Graphics/RenderBackendFlat.h"
#include "Scene/Scene.h"
#include "XR/XrBackend.h"

namespace XRLib {
class XRLib {
   public:
    XRLib();
    ~XRLib();
    XRLib& SetApplicationName(const std::string& applicationName);
    XRLib& SetVersionNumber(unsigned int majorVersion, unsigned int minorVersion, unsigned int patchVersion);
    XRLib& EnableValidationLayer();
    XRLib& SetCustomOpenXRRuntime(const std::filesystem::path& runtimePath);
    void Init(bool xr = true);
    void Prepare(std::vector<std::unique_ptr<Graphics::IGraphicsRenderpass>> customRenderpass = {});
    void Run(std::function<void(uint32_t&, Graphics::CommandBuffer&)> customRecordingFunction = nullptr);
    bool ShouldStop();
    XRLib& SetWindowProperties(Graphics::WindowHandler::WindowMode windowMode);
    XRLib& SetWindowProperties(Graphics::WindowHandler::WindowMode windowMode, unsigned int width, unsigned int height);
    Scene& SceneBackend();
    Graphics::RenderBackend& RenderBackend();
    XR::XrBackend& XrBackend();

    Graphics::VkCore& GetVkCore() { return vkCore; }
    XR::XrCore& GetXrCore() { return xrCore; }

   private:
    Info info;
    Scene scene;
    Graphics::VkCore vkCore;
    XR::XrCore xrCore;
    std::unique_ptr<XR::XrBackend> xrBackend{nullptr};
    std::unique_ptr<Graphics::RenderBackend> renderBackend{nullptr};
    bool initialized = false;

    void InitXRBackend();
    void InitRenderBackend();
};

static XRLib* instance;
}    // namespace XRLib
