#include "Graphics/RenderBackend.h"
#include "Graphics/RenderBackendFlat.h"
#include "Graphics/Window.h"
#include "XR/XRBackend.h"

#include "XRLib.h"

namespace XRLib {
class XRLib::Impl {
   public:
    Impl()
        : info(std::make_shared<Info>()),
          vkCore(std::make_shared<Graphics::VkCore>()),
          xrCore(std::make_shared<XR::XrCore>()),
          scene(std::make_shared<Scene>()) {}

    std::shared_ptr<Info> info;
    std::shared_ptr<Graphics::VkCore> vkCore;
    std::shared_ptr<XR::XrCore> xrCore;
    std::shared_ptr<Scene> scene;
    std::unique_ptr<XR::XrBackend> xrBackend{nullptr};
    std::unique_ptr<Graphics::RenderBackend> renderBackend{nullptr};
    std::vector<std::pair<const std::string&, const std::string&>> passesToAdd;
    bool initialized = false;

    void Init(bool xr);
    void Run();
    bool ShouldStop();
    void InitXRBackend();
    void InitRenderBackend();
};

XRLib::XRLib() : impl(std::make_unique<Impl>()) {}

XRLib::~XRLib() = default;

XRLib& XRLib::SetApplicationName(std::string applicationName) {
    impl->info->applicationName = std::move(applicationName);
    return *this;
}

XRLib& XRLib::SetVersionNumber(unsigned int majorVersion,
                               unsigned int minorVersion,
                               unsigned int patchVersion) {
    impl->info->majorVersion = majorVersion;
    impl->info->minorVersion = minorVersion;
    impl->info->patchVersion = patchVersion;
    return *this;
}

XRLib& XRLib::EnableValidationLayer() {
    impl->info->validationLayer = true;
    return *this;
}

XRLib& XRLib::SetCustomOpenXRRuntime(const std::filesystem::path& runtimePath) {
    auto fullPath = Util::ResolvePath(runtimePath);
    if (!std::filesystem::is_regular_file(fullPath)) {
        return *this;
    }
    fullPath = std::filesystem::canonical(fullPath);
    const char* env_name = "XR_RUNTIME_JSON";
#if defined(_WIN32)
    _putenv_s(env_name, fullPath.generic_string().c_str());
#elif defined(__linux__)
    setenv(env_name, fullPath.generic_string().c_str(), 1);
#else
    Util::ERR("Platform not supported")
#endif
    return *this;
}

void XRLib::Init(bool xr) {
    impl->Init(xr);
}

void XRLib::Impl::Init(bool xr) {
    if (!xr) {
        xrCore->SetXRValid(false);
    } else {
        InitXRBackend();
    }

    Graphics::WindowHandler::Init(info);
    InitRenderBackend();

    if (scene->CheckTaskRunning()) {
        scene->WaitForAllMeshesToLoad();
    }

    renderBackend->Prepare(passesToAdd);

    initialized = true;

    if (!xrCore->IsXRValid())
        Graphics::WindowHandler::ShowWindow();

    LOGGER(LOGGER::INFO) << "XRLib Initialized";
}

void XRLib::Run() {
    impl->Run();
}

void XRLib::Impl::Run() {
    if (!xrCore->IsXRValid())
        Graphics::WindowHandler::Update();

    uint32_t imageIndex = 0;
    if (xrCore->IsXRValid()) {
        XrResult result = xrBackend->StartFrame(imageIndex);

        if (result == XR_SUCCESS) {
            xrBackend->UpdateXrInput();
            renderBackend->Run(imageIndex);
        }

        xrBackend->EndFrame(imageIndex);
    } else {
        renderBackend->Run(imageIndex);
    }
}

bool XRLib::ShouldStop() {
    return impl->ShouldStop();
}

bool XRLib::Impl::ShouldStop() {
    return xrCore->IsXRValid() ? xrBackend->XrShouldStop()
                               : renderBackend->WindowShouldClose();
}

XRLib& XRLib::Fullscreen() {
    impl->info->fullscreen = true;
    return *this;
}

XRLib& XRLib::AddRenderPass(const std::string& vertexShaderPath,
                            const std::string& fragmentShaderPath) {
    impl->passesToAdd.push_back({vertexShaderPath, fragmentShaderPath});
    return *this;
}

void XRLib::Impl::InitXRBackend() {
    if (info->applicationName.empty()) {
        Util::ErrorPopup("No application name specified");
    }

    xrBackend = std::make_unique<XR::XrBackend>(info, vkCore, xrCore);
}

void XRLib::Impl::InitRenderBackend() {
    if (info->majorVersion == 0 && info->minorVersion == 0 &&
        info->patchVersion == 0) {
        LOGGER(LOGGER::WARNING) << "Version number is 0";
    }

    if (info->applicationName.empty()) {
        Util::ErrorPopup("No application name specified");
    }

    if (!xrCore->IsXRValid()) {
        renderBackend = std::make_unique<Graphics::RenderBackendFlat>(
            info, vkCore, xrCore, scene);
    } else {
        renderBackend = std::make_unique<Graphics::RenderBackend>(
            info, vkCore, xrCore, scene);
    }
}

Scene& XRLib::SceneBackend() {
    return *(impl->scene);
}

}    // namespace XRLib
