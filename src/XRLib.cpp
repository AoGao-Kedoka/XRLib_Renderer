#include "XRLib.h"

XRLib& XRLib::SetApplicationName(std::string applicationName) {
    _LOGFUNC_;

    info.applicationName = std::move(applicationName);
    return *this;
}

XRLib& XRLib::SetVersionNumber(unsigned int majorVersion, unsigned int minorVersion, unsigned int patchVersion) {
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

XRLib& XRLib::InitXRBackend() {
    _LOGFUNC_;
    
    if (info.applicationName.empty()) {
        LOGGER(LOGGER::ERR) << "No application name specified";
        return *this;
    }

    if (renderBackend == nullptr) {
        LOGGER(LOGGER::ERR) << "Render backend should initialized first";
        return *this;
    }

    XRBackend xr{info, renderBackend};
    xrBackend = std::make_shared<XRBackend>(std::move(xr));

    return *this;
}

XRLib& XRLib::InitRenderBackend() {
    _LOGFUNC_;

    if (info.majorVersion == 0 &&
        info.minorVersion == 0 &&
        info.patchVersion == 0) {
        LOGGER(LOGGER::WARNING) << "Version number is 0";
    }

    if (info.applicationName.empty()) {
        LOGGER(LOGGER::ERR) << "No application name specified";
        return *this;
    }

    RenderBackend renderer{info};
    renderBackend = std::make_shared<RenderBackend>(renderer);

    return *this;
}
