#include "XRLib.h"

namespace XRLib {

XRLib::XRLib()
    : info(std::make_shared<Info>()), vkCore(std::make_shared<Graphics::VkCore>()),
      xrCore(std::make_shared<XR::XrCore>()), scene(std::make_shared<Scene>()) {
    Util::CheckPlatformSupport();
}

XRLib::~XRLib() {
    Graphics::WindowHandler::Deinitialize();
}

XRLib& XRLib::SetApplicationName(const std::string& applicationName) {
    info->applicationName = applicationName;
    return *this;
}

XRLib& XRLib::SetVersionNumber(unsigned int majorVersion, unsigned int minorVersion, unsigned int patchVersion) {
    info->majorVersion = majorVersion;
    info->minorVersion = minorVersion;
    info->patchVersion = patchVersion;
    return *this;
}

XRLib& XRLib::EnableValidationLayer() {
    info->validationLayer = true;
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
    Util::ErrorPopup("Platform not supported");
#endif
    return *this;
}

void XRLib::Init(bool xr) {
    EventSystem::TriggerEvent(Events::XRLIB_EVENT_APPLICATION_INIT_STARTED);
    if (!xr) {
        xrCore->SetXRValid(false);
    } else {
        InitXRBackend();
    }

    Graphics::WindowHandler::Init(info);
    InitRenderBackend();

    SceneBackend().WaitForAllMeshesToLoad();

    renderBackend->Prepare(passesToAdd);
    initialized = true;

    if (!xrCore->IsXRValid()) {
        Graphics::WindowHandler::ShowWindow();
    }

    LOGGER(LOGGER::INFO) << "XRLib Initialized";
    EventSystem::TriggerEvent(Events::XRLIB_EVENT_APPLICATION_INIT_FINISHED);
}

void XRLib::Run() {
    EventSystem::TriggerEvent(Events::XRLIB_EVENT_APPLICATION_PRE_RENDERING);
    if (!xrCore->IsXRValid()) {
        Graphics::WindowHandler::Update();
    }

    uint32_t imageIndex = 0;
    if (xrCore->IsXRValid()) {
        XrResult result = xrBackend->StartFrame(imageIndex);

        if (result == XR_SUCCESS) {
            renderBackend->Run(imageIndex);
        }

        xrBackend->EndFrame(imageIndex);
    } else {
        renderBackend->Run(imageIndex);
    }
    EventSystem::TriggerEvent(Events::XRLIB_EVENT_APPLICATION_POST_RENDERING);
}

bool XRLib::ShouldStop() {
    return xrCore->IsXRValid() ? xrBackend->XrShouldStop() : renderBackend->WindowShouldClose();
}

XRLib& XRLib::Fullscreen() {
    info->fullscreen = true;
    return *this;
}

XRLib& XRLib::AddRenderPass(const std::string& vertexShaderPath, const std::string& fragmentShaderPath) {
    passesToAdd.push_back({vertexShaderPath, fragmentShaderPath});
    return *this;
}

void XRLib::InitXRBackend() {
    if (info->applicationName.empty()) {
        Util::ErrorPopup("No application name specified");
    }

    xrBackend = std::make_unique<XR::XrBackend>(info, vkCore, xrCore);
}

void XRLib::InitRenderBackend() {
    if (info->majorVersion == 0 && info->minorVersion == 0 && info->patchVersion == 0) {
        LOGGER(LOGGER::WARNING) << "Version number is 0";
    }

    if (info->applicationName.empty()) {
        Util::ErrorPopup("No application name specified");
    }

    if (!xrCore->IsXRValid()) {
        renderBackend = std::make_unique<Graphics::RenderBackendFlat>(info, vkCore, xrCore, scene);
    } else {
        renderBackend = std::make_unique<Graphics::RenderBackend>(info, vkCore, xrCore, scene);
    }
}

Scene& XRLib::SceneBackend() {
    return *scene;
}

Graphics::RenderBackend& XRLib::RenderBackend() {
    return *renderBackend;
}

XR::XrBackend& XRLib::XrBackend() {
    return *xrBackend;
}

}    // namespace XRLib
