#include "XRLib.h"

namespace XRLib {
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

void XRLib::Init(bool xr) {
    _LOGFUNC_;

    if (!xr) {
        xrCore->SetXRValid(false);
    }

    if (xr) InitXRBackend();
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
    if (xrCore->IsXRValid()) {
        xrBackend->Run();
    }
    renderBackend->Run();
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
