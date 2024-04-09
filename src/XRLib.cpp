#include "XRLib.h"

namespace XRLib {
XRLib& XRLib::SetApplicationName(std::string applicationName) {
    _LOGFUNC_;

    info.applicationName = std::move(applicationName);
    return *this;
}

XRLib& XRLib::SetVersionNumber(unsigned int majorVersion,
                               unsigned int minorVersion,
                               unsigned int patchVersion) {
    _LOGFUNC_;

    info.majorVersion = majorVersion;
    info.minorVersion = minorVersion;
    info.patchVersion = patchVersion;
    return *this;
}

XRLib& XRLib::EnableValidationLayer() {
    _LOGFUNC_;

    info.validationLayer = true;
    return *this;
}

XRLib& XRLib::Init(bool xr) {
    _LOGFUNC_;

    if (!xr) {
        core.SetXRValid(false);
    }

    if (xr) InitXRBackend();
    InitRenderBackend();

    renderBackend->Prepare();
    
    return *this;
}

void XRLib::InitXRBackend() {
    _LOGFUNC_;

    if (info.applicationName.empty()) {
        LOGGER(LOGGER::ERR) << "No application name specified";
        exit(-1);
    }

    xrBackend = std::make_unique<XRBackend>(std::move(XRBackend{info, core}));
}

void XRLib::InitRenderBackend() {
    _LOGFUNC_;

    if (info.majorVersion == 0 && info.minorVersion == 0 &&
        info.patchVersion == 0) {
        LOGGER(LOGGER::WARNING) << "Version number is 0";
    }

    if (info.applicationName.empty()) {
        LOGGER(LOGGER::ERR) << "No application name specified";
        exit(-1);
    }

    if (!core.IsXRValid()) {
        renderBackend =
            std::make_shared<RenderBackendFlat>(std::move(RenderBackendFlat{info, core}));
    } else {
        renderBackend =
            std::make_shared<RenderBackend>(std::move(RenderBackend{info, core}));
    }

}
}    // namespace XRLib
