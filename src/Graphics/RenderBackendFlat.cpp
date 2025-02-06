#include "RenderBackendFlat.h"

namespace XRLib {
namespace Graphics {
RenderBackendFlat::RenderBackendFlat(Config& info, VkCore& core, XR::XrCore& xrCore, Scene& scene)
    : RenderBackend(info, core, xrCore, scene) {
    PrepareFlatWindow();
}
RenderBackendFlat::~RenderBackendFlat() {
    vkDeviceWaitIdle(vkCore.GetRenderDevice());
    VkUtil::VkSafeClean(vkDestroySurfaceKHR, vkCore.GetRenderInstance(), vkCore.GetFlatSurface(), nullptr);
}

void RenderBackendFlat::Prepare() {
    vkSRB->GetSwapchain() = std::make_unique<Swapchain>(vkCore);

    // register window resize callback
    EventSystem::Callback<int, int> windowResizeCallback =
        std::bind(&RenderBackendFlat::OnWindowResized, this, std::placeholders::_1, std::placeholders::_2);
    EventSystem::RegisterListener(Events::XRLIB_EVENT_WINDOW_RESIZED, windowResizeCallback);

    // register key press callback
    EventSystem::Callback<int> keyPressCallback =
        std::bind(&RenderBackendFlat::OnKeyPressed, this, std::placeholders::_1);
    EventSystem::RegisterListener(Events::XRLIB_EVENT_KEY_PRESSED, keyPressCallback);

    // register mouse movement callback
    EventSystem::Callback<double, double> onMouseCallback =
        std::bind(&RenderBackendFlat::OnMouseMovement, this, std::placeholders::_1, std::placeholders::_2);
    EventSystem::RegisterListener(Events::XRLIB_EVENT_MOUSE_RIGHT_MOVEMENT_EVENT, onMouseCallback);

    WindowHandler::ActivateInput();

    vkSRB->InitVerticesIndicesBuffers();
    vkSRB->Prepare();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Handling events
//////////////////////////////////////////////////////////////////////////////////////////////////////////

void RenderBackendFlat::OnMouseMovement(double deltaX, double deltaY) {
    auto cam = scene.MainCamera();

    cam->Yaw += deltaX * info.mouseSensitivity;
    cam->Pitch += deltaY * info.mouseSensitivity;

    if (cam->Pitch > 89.0f)
        cam->Pitch = 89.0f;
    if (cam->Pitch < -89.0f)
        cam->Pitch = -89.0f;

    glm::vec3 direction;
    direction.x = cos(glm::radians(cam->Yaw)) * cos(glm::radians(cam->Pitch));
    direction.y = sin(glm::radians(cam->Pitch));
    direction.z = sin(glm::radians(cam->Yaw)) * cos(glm::radians(cam->Pitch));

    glm::vec3 cameraFront = glm::normalize(direction);

    scene.MainCamera()->UpdateCamera(cameraFront);
}

void RenderBackendFlat::OnKeyPressed(int keyCode) {
    float movementSensitivity = info.movementSpeed;
    auto& cam = scene.MainCamera()->GetLocalTransform();
    if (keyCode == GLFW_KEY_W) {
        cam.Translate(-cam.FrontVector() * movementSensitivity);
    }
    if (keyCode == GLFW_KEY_S) {
        cam.Translate(-cam.BackVector() * movementSensitivity);
    }
    if (keyCode == GLFW_KEY_A) {
        cam.Translate(-cam.LeftVector() * movementSensitivity);
    }
    if (keyCode == GLFW_KEY_D) {
        cam.Translate(-cam.RightVector() * movementSensitivity);
    }
    if (keyCode == GLFW_KEY_SPACE) {
        cam.Translate(-cam.UpVector() * movementSensitivity);
    }
    if (keyCode == GLFW_KEY_LEFT_CONTROL) {
        cam.Translate(-cam.DownVector() * movementSensitivity);
    }
}

void RenderBackendFlat::OnWindowResized(int width, int height) {
    LOGGER(LOGGER::DEBUG) << "Window resized";
}

void RenderBackendFlat::PrepareFlatWindow() {
    WindowHandler::VkGetWindowSurface(vkCore.GetRenderInstance(), &vkCore.GetFlatSurface());

    VkBool32 presentSupport = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(vkCore.GetRenderPhysicalDevice(), vkCore.GetGraphicsQueueFamilyIndex(),
                                         vkCore.GetFlatSurface(), &presentSupport);

    if (!presentSupport) {
        LOGGER(LOGGER::WARNING) << "Graphics queue doesn't have present support";
        //TODO: graphics queue normaly have present support
    }

    LOGGER(LOGGER::DEBUG) << "Window created";
}
}    // namespace Graphics
}    // namespace XRLib
