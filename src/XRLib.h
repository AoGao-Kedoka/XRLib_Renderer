// XRLib.h
#pragma once

#include "Scene.h"
#include "pch.h"

namespace XRLib {
class XRLib {
   public:
    XRLib();
    ~XRLib();
    XRLib& SetApplicationName(std::string applicationName);
    XRLib& SetVersionNumber(unsigned int majorVersion,
                            unsigned int minorVersion,
                            unsigned int patchVersion);
    XRLib& EnableValidationLayer();
    XRLib& SetCustomOpenXRRuntime(const std::filesystem::path& runtimePath);
    void Init(bool xr = true);
    XRLib& AddRenderPass(const std::string& vertexShaderPath,
                         const std::string& fragmentShaderPath);
    void Run();
    bool ShouldStop();
    XRLib& Fullscreen();
    Scene& SceneBackend();

   private:
    class Impl;                    // Forward declaration
    std::unique_ptr<Impl> impl;    // Pointer to implementation
};
}    // namespace XRLib
