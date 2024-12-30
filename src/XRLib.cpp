#include "XRLib.h"

namespace XRLib {

XRLib::XRLib() {
    Util::CheckPlatformSupport();

    if (instance == nullptr) {
        instance = this;
    } else {
        Util::ErrorPopup("Only one instance of XRLib is allowed.");
    }
}

XRLib::~XRLib() {
    Graphics::WindowHandler::Deinitialize();
}

XRLib& XRLib::SetApplicationName(const std::string& applicationName) {
    info.applicationName = applicationName;
    return *this;
}

XRLib& XRLib::SetVersionNumber(unsigned int majorVersion, unsigned int minorVersion, unsigned int patchVersion) {
    info.majorVersion = majorVersion;
    info.minorVersion = minorVersion;
    info.patchVersion = patchVersion;
    return *this;
}

XRLib& XRLib::EnableValidationLayer() {
    info.validationLayer = true;
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
    if (!xr) {
        xrCore.SetXRValid(false);
    } else {
        InitXRBackend();
    }

    Graphics::WindowHandler::Init(info);
    InitRenderBackend();

    LOGGER(LOGGER::INFO) << "XRLib Initialized";
}

void XRLib::Prepare(std::vector<std::unique_ptr<Graphics::IGraphicsRenderpass>> customRenderpass) {
    LOGGER(LOGGER::DEBUG) << "Preparing renderer";
    EventSystem::TriggerEvent(Events::XRLIB_EVENT_APPLICATION_INIT_STARTED);

    SceneBackend().WaitForAllMeshesToLoad();

    renderBackend->Prepare(customRenderpass);
    initialized = true;

    if (!xrCore.IsXRValid()) {
        Graphics::WindowHandler::ShowWindow();
    }
    EventSystem::TriggerEvent(Events::XRLIB_EVENT_APPLICATION_INIT_FINISHED);
}

void XRLib::Run(std::function<void(uint32_t&, Graphics::CommandBuffer&)> customRecordingFunction) {

    if (!initialized) {
        Prepare();
    }

    EventSystem::TriggerEvent(Events::XRLIB_EVENT_APPLICATION_PRE_RENDERING);

    if (!xrCore.IsXRValid()) {
        Graphics::WindowHandler::Update();
    }

    uint32_t imageIndex = 0;

    auto recordFrame = [&](uint32_t index) {
        if (customRecordingFunction) {
            renderBackend->RecordFrame(index, customRecordingFunction);
        } else {
            renderBackend->RecordFrame(index);
        }
    };

    if (xrCore.IsXRValid()) {
        if (xrBackend->StartFrame(imageIndex) == XR_SUCCESS) {
            recordFrame(imageIndex);
        }
        xrBackend->EndFrame(imageIndex);
    } else if (renderBackend->StartFrame(imageIndex)) {
        recordFrame(imageIndex);
        renderBackend->EndFrame(imageIndex);
    }

    EventSystem::TriggerEvent(Events::XRLIB_EVENT_APPLICATION_POST_RENDERING);
}

bool XRLib::ShouldStop() {
    return xrCore.IsXRValid() ? xrBackend->XrShouldStop() : renderBackend->WindowShouldClose();
}

XRLib& XRLib::SetWindowProperties(Graphics::WindowHandler::WindowMode windowMode) {
    info.windowMode = windowMode;
    return *this;
}

XRLib& XRLib::SetWindowProperties(Graphics::WindowHandler::WindowMode windowMode, unsigned int width,
                                  unsigned int height) {
    info.windowMode = windowMode;
    info.defaultWindowWidth = width;
    info.defaultWindowHeight = height;
    return *this;
}

void XRLib::InitXRBackend() {
    if (info.applicationName.empty()) {
        Util::ErrorPopup("No application name specified");
    }

    xrBackend = std::make_unique<XR::XrBackend>(info, vkCore, xrCore);
}

void XRLib::InitRenderBackend() {
    if (info.majorVersion == 0 && info.minorVersion == 0 && info.patchVersion == 0) {
        LOGGER(LOGGER::WARNING) << "Version number is 0";
    }

    if (info.applicationName.empty()) {
        Util::ErrorPopup("No application name specified");
    }

    if (!xrCore.IsXRValid()) {
        renderBackend = std::make_unique<Graphics::RenderBackendFlat>(info, vkCore, xrCore, scene);
    } else {
        renderBackend = std::make_unique<Graphics::RenderBackend>(info, vkCore, xrCore, scene);
    }
}

Scene& XRLib::SceneBackend() {
    return scene;
}

Graphics::RenderBackend& XRLib::RenderBackend() {
    return *renderBackend;
}

XR::XrBackend& XRLib::XrBackend() {
    return *xrBackend;
}

}    // namespace XRLib
