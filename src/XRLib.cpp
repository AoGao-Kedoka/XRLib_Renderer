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

XRLib& XRLib::Init() {
    InitXRBackend();
    InitRenderBackend();
    renderBackend->CreateVulkanInstance();
    renderBackend->CreatePhysicalDevice();
    return *this;
}

void XRLib::InitXRBackend() {
    _LOGFUNC_;
    
    if (info.applicationName.empty()) {
        LOGGER(LOGGER::ERR) << "No application name specified";
        exit(-1);
    }

    XRBackend xr{info, core};
    xrBackend = std::make_unique<XRBackend>(std::move(xr));
}

void XRLib::InitRenderBackend() {
    _LOGFUNC_;

    if (info.majorVersion == 0 &&
        info.minorVersion == 0 &&
        info.patchVersion == 0) {
        LOGGER(LOGGER::WARNING) << "Version number is 0";
    }

    if (info.applicationName.empty()) {
        LOGGER(LOGGER::ERR) << "No application name specified";
        exit(-1);
    }

    RenderBackend renderer{info, core};
    renderBackend = std::make_unique<RenderBackend>(std::move(renderer));
}
