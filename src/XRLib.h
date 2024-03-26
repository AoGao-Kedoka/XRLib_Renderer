#pragma once

#include <string>

#include "Info.h"
#include "XRBackend.h"
#include "RenderBackend.h"

class XRLib {
public:
    XRLib();
    XRLib& SetApplicationName(std::string applicationName);
    XRLib& SetVersionNumber(unsigned int versionNumber);
    XRLib& EnableValidationLayer();
    XRBackend& InitXRBackend();
    RenderBackend& InitRenderBackend();
private:
    Info info;
};