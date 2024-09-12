#include "Window.h"

namespace XRLib {
namespace Graphics {

GLFWwindow* WindowHandler::window = nullptr;
static double leftMouseDown = false;
static double rightMouseDown = false;
static double lastLeftX = 0.0, lastLeftY = 0.0;
static double lastRightX = 0.0, lastRightY = 0.0;

void WindowHandler::Init(std::shared_ptr<Info> info) {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    window =
        glfwCreateWindow(info->fullscreen ? glfwGetVideoMode(glfwGetPrimaryMonitor())->width : 400,
                         info->fullscreen ? glfwGetVideoMode(glfwGetPrimaryMonitor())->height : 400,
                         info->applicationName.c_str(), info->fullscreen ? glfwGetPrimaryMonitor() : nullptr, nullptr);

    glfwMakeContextCurrent(window);
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
    }
    if (rightMouseDown) {
        double deltaX = xpos - lastRightX;
        double deltaY = ypos - lastRightY;
        EventSystem::TriggerEvent(Events::XRLIB_EVENT_MOUSE_RIGHT_MOVEMENT_EVENT, deltaX, deltaY);
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
