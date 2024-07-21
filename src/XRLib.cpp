#include "XRLib.h"

namespace XRLib {
XRLib::XRLib() {
    bool result = Util::CheckPlatformSupport();
    if (result == false) {
        Util::ErrorPopup("Current platform not supported");
    }
}
XRLib& XRLib::SetApplicationName(std::string applicationName) {
    _LOGFUNC_;

    info->applicationName = std::move(applicationName);
    return *this;
}

XRLib& XRLib::SetVersionNumber(unsigned int majorVersion,
                               unsigned int minorVersion,
                               unsigned int patchVersion) {
    _LOGFUNC_;

    info->majorVersion = majorVersion;
    info->minorVersion = minorVersion;
    info->patchVersion = patchVersion;
    return *this;
}

XRLib& XRLib::EnableValidationLayer() {
    _LOGFUNC_;

    info->validationLayer = true;
    return *this;
}

XRLib& XRLib::SetCustomOpenXRRuntime(const std::filesystem::path& runtimePath) {
    auto fullPath = Util::ResolvePath(runtimePath);
    if (!std::filesystem::is_regular_file(fullPath)) {
        return *this;
    }
    fullPath = std::filesystem::canonical(fullPath);
#if defined(_WIN32)
    _putenv_s("XR_RUNTIME_JSON", fullPath.c_str());
#elif defined(__linux__)
    setenv("XR_RUNTIME_JSON", fullPath.c_str(), 1);
#else
    Util::ERR("Platform not supported")
#endif
    LOGGER(LOGGER::INFO) << "Set XR_RUNTIME_JSON to: " << fullPath.string();
    return *this;
}

void XRLib::Init(bool xr) {
    _LOGFUNC_;

    if (!xr) {
        xrCore->SetXRValid(false);
    }

    if (xr)
        InitXRBackend();
    InitRenderBackend();

    if (scene->CheckTaskRunning()) {
        scene->WaitForAllMeshesToLoad();
    }

    if (xrCore->IsXRValid())
        xrBackend->Prepare();
    renderBackend->Prepare(passesToAdd);

    initialized = true;
}

void XRLib::Run() {

    uint32_t imageIndex = 0;
    if (xrCore->IsXRValid()) {
        XrResult result = xrBackend->StartFrame(imageIndex);
        if (result != XR_SUCCESS) {
            return;
        }
    }

    renderBackend->Run(imageIndex);

    if (xrCore->IsXRValid()) {
        xrBackend->EndFrame(imageIndex);
    }
}

XRLib& XRLib::Fullscreen() {
    info->fullscreen = true;
    return *this;
}

XRLib& XRLib::AddRenderPass(const std::string& vertexShaderPath,
                            const std::string& fragmentShaderPath) {
    passesToAdd.push_back({vertexShaderPath, fragmentShaderPath});

    return *this;
}

void XRLib::InitXRBackend() {
    _LOGFUNC_;

    if (info->applicationName.empty()) {
        LOGGER(LOGGER::ERR) << "No application name specified";
        exit(-1);
    }

    xrBackend = std::make_unique<XRBackend>(info, vkCore, xrCore);
}

void XRLib::InitRenderBackend() {
    _LOGFUNC_;

    if (info->majorVersion == 0 && info->minorVersion == 0 &&
        info->patchVersion == 0) {
        LOGGER(LOGGER::WARNING) << "Version number is 0";
    }

    if (info->applicationName.empty()) {
        LOGGER(LOGGER::ERR) << "No application name specified";
        exit(-1);
    }

    if (!xrCore->IsXRValid()) {
        renderBackend =
            std::make_unique<RenderBackendFlat>(info, vkCore, xrCore, scene);
    } else {
        renderBackend =
            std::make_unique<RenderBackend>(info, vkCore, xrCore, scene);
    }
}
}    // namespace XRLib
