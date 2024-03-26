#pragma once

#include <string>

#include "Info.h"
#include "RenderBackend.h"
#include "XRBackend.h"

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