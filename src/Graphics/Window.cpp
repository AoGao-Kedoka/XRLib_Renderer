#include "Window.h"

namespace XRLib {
namespace Graphics {
GLFWwindow* WindowHandler::window = nullptr;

void WindowHandler::Init(std::shared_ptr<Info> info) {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    window = glfwCreateWindow(
        info->fullscreen ? glfwGetVideoMode(glfwGetPrimaryMonitor())->width
                         : 400,
        info->fullscreen ? glfwGetVideoMode(glfwGetPrimaryMonitor())->height
                         : 400,
        info->applicationName.c_str(),
        info->fullscreen ? glfwGetPrimaryMonitor() : nullptr, nullptr);
}

void WindowHandler::ShowWindow() {
    glfwShowWindow(window);
}

void WindowHandler::Update() {
    glfwPollEvents();
}

bool WindowHandler::WindowShouldClose() {
    return glfwWindowShouldClose(window);
}

void WindowHandler::VkGetWindowSurface(VkInstance instance,
                                       VkSurfaceKHR* surface) {
    if (glfwCreateWindowSurface(instance, window, nullptr, surface) !=
        VK_SUCCESS) {
        Util::ErrorPopup("Failed to create window surface");
    }
}

std::pair<int, int> WindowHandler::GetFrameBufferSize() {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(window, &width, &height);
        glfwWaitEvents();
    }
    return {width, height};
}

const char**
WindowHandler::VkGetWindowExtensions(uint32_t* requiredExtensionCount) {
    const char** glfwExtensions =
        glfwGetRequiredInstanceExtensions(requiredExtensionCount);
    if (!glfwExtensions) {
        Util::ErrorPopup("Error getting glfw instance extension");
    }
    return glfwExtensions;
}
}    // namespace Graphics
}    // namespace XRLib
