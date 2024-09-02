#pragma once

#include "pch.h"

#include "Utils/Info.h"
#include "Utils/Util.h"

namespace XRLib {
namespace Graphics {
class WindowHandler {
   public:
    inline static std::string XRLIB_EVENT_WINDOW_RESIZED{"window_resized"};

   public:
    static void Init(std::shared_ptr<Info> info);
    static void ShowWindow();
    static void Update();
    static bool WindowShouldClose();
    static std::pair<int, int> GetFrameBufferSize();

    //vulkan specific
    static void VkGetWindowSurface(VkInstance instance, VkSurfaceKHR* surface);
    static const char** VkGetWindowExtensions(uint32_t* requiredExtensionCount);

   private:
    static GLFWwindow* window;
};
}    // namespace Graphics
}    // namespace XRLib
