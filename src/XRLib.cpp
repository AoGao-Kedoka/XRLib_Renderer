#include "XRLib.h"

XRLib::XRLib()
{
    LOGGER(LOGGER::INFO) << "XRLib activated.";
}

XRLib& XRLib::SetApplicationName(std::string applicationName)
{
    _LOGFUNC_;
    info.applicationName = applicationName;
    return *this;
}

XRLib& XRLib::SetVersionNumber(unsigned int versionNumber)
{
    _LOGFUNC_;
    info.version = versionNumber;
    return *this;
}

XRLib& XRLib::EnableValidationLayer()
{
    _LOGFUNC_;
    info.validationLayer = true;
    return *this;
}

XRBackend& XRLib::InitXRBackend()
{
    _LOGFUNC_;
    if (info.applicationName == "") {
        LOGGER(LOGGER::ERR) << "No application name specified";
    }
    XRBackend backend{ info };
    return backend;
}

RenderBackend& XRLib::InitRenderBackend()
{
    _LOGFUNC_;
    RenderBackend renderer;
    return renderer;
}
