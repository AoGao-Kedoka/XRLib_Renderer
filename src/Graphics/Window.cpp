#include "Window.h"

namespace XRLib {
namespace Graphics {

GLFWwindow* WindowHandler::window = nullptr;
static double leftMouseDown = false;
static double rightMouseDown = false;
static double lastLeftX = 0.0, lastLeftY = 0.0;
static double lastRightX = 0.0, lastRightY = 0.0;

void WindowHandler::Init(Info& info) {
    WindowMode mode = static_cast<WindowMode>(info.windowMode);

    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* videoMode = glfwGetVideoMode(primaryMonitor);

    if (mode == BORDERLESS) {
        glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
    }

    int width = info.defaultWindowWidth, height = info.defaultWindowHeight;
    GLFWmonitor* monitor = nullptr;

    if (mode == FULLSCREEN || mode == BORDERLESS) {
        width = videoMode->width;
        height = videoMode->height;
        monitor = (mode == FULLSCREEN) ? primaryMonitor : nullptr;
    }

    window = glfwCreateWindow(width, height, info.applicationName.c_str(), monitor, nullptr);
    glfwMakeContextCurrent(window);
}

void WindowHandler::Deinitialize() {
    glfwDestroyWindow(window);
}

void WindowHandler::ActivateInput() {
    glfwSetMouseButtonCallback(window, MouseButtonCallback);
    glfwSetCursorPosCallback(window, MouseMoveCallback);
    glfwSetKeyCallback(window, HandleKeyCallback);
}

void WindowHandler::ShowWindow() {
    glfwShowWindow(window);
}

void WindowHandler::Update() {
    glfwPollEvents();
}

void WindowHandler::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            EventSystem::TriggerEvent(Events::XRLIB_EVENT_MOUSE_LEFT_DOWN_EVENT);
            glfwGetCursorPos(window, &lastLeftX, &lastLeftY);
            leftMouseDown = true;
        }
        if (action == GLFW_RELEASE) {
            leftMouseDown = false;
        }
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (action == GLFW_PRESS) {
            EventSystem::TriggerEvent(Events::XRLIB_EVENT_MOUSE_LEFT_DOWN_EVENT);
            glfwGetCursorPos(window, &lastRightX, &lastRightY);
            rightMouseDown = true;
        }
        if (action == GLFW_RELEASE) {
            rightMouseDown = false;
        }
    }
}

void WindowHandler::MouseMoveCallback(GLFWwindow* window, double xpos, double ypos) {
    if (leftMouseDown) {
        double deltaX = xpos - lastLeftX;
        double deltaY = ypos - lastLeftY;
        EventSystem::TriggerEvent(Events::XRLIB_EVENT_MOUSE_LEFT_MOVEMENT_EVENT, deltaX, deltaY);
        lastLeftX = xpos;
        lastLeftY = ypos;
    }
    if (rightMouseDown) {
        double deltaX = xpos - lastRightX;
        double deltaY = ypos - lastRightY;
        EventSystem::TriggerEvent(Events::XRLIB_EVENT_MOUSE_RIGHT_MOVEMENT_EVENT, deltaX, deltaY);
        lastRightX = xpos;
        lastRightY = ypos;
    }
}

void WindowHandler::HandleKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_REPEAT || action == GLFW_PRESS) {
        if (key == -1)
            return;
        EventSystem::TriggerEvent(Events::XRLIB_EVENT_KEY_PRESSED, key);
    }
}

bool WindowHandler::WindowShouldClose() {
    return glfwWindowShouldClose(window);
}

void WindowHandler::VkGetWindowSurface(VkInstance instance, VkSurfaceKHR* surface) {
    if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS) {
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

const char** WindowHandler::VkGetWindowExtensions(uint32_t* requiredExtensionCount) {
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(requiredExtensionCount);
    if (!glfwExtensions) {
        Util::ErrorPopup("Error getting glfw instance extension");
    }
    return glfwExtensions;
}
}    // namespace Graphics
}    // namespace XRLib
