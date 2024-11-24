#pragma once

#include "pch.h"

#include "Event/EventSystem.h"
#include "Event/Events.h"
#include "Utils/Info.h"
#include "Utils/Util.h"

namespace XRLib {
namespace Graphics {
class WindowHandler {
   public:
    static void Init(std::shared_ptr<Info> info);
    static void ShowWindow();
    static void Update();
    static bool WindowShouldClose();
    static void Deinitialize();
    static std::pair<int, int> GetFrameBufferSize();

    enum WindowMode {
        WINDOWED = 0,
        BORDERLESS = 1,
        FULLSCREEN = 2
    };

    //vulkan specific
    static void VkGetWindowSurface(VkInstance instance, VkSurfaceKHR* surface);
    static const char** VkGetWindowExtensions(uint32_t* requiredExtensionCount);

    static void ActivateInput();

    static GLFWwindow* GetWindow() { return window; }

   private:
    static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

    static void MouseMoveCallback(GLFWwindow* window, double xpos, double ypos);

    static void HandleKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

   private:
    static GLFWwindow* window;
};
}    // namespace Graphics
}    // namespace XRLib
