#include "RenderBackendFlat.h"

namespace XRLib {
namespace Graphics {
RenderBackendFlat::RenderBackendFlat(Info& info, std::shared_ptr<VkCore> core, XR::XrCore& xrCore, Scene& scene)
    : RenderBackend(info, core, xrCore, scene) {
    PrepareFlatWindow();
    swapchain = std::make_unique<Swapchain>(core);
}
RenderBackendFlat::~RenderBackendFlat() {
    if (vkCore == nullptr)
        return;
    vkDeviceWaitIdle(vkCore->GetRenderDevice());
    VkUtil::VkSafeClean(vkDestroySurfaceKHR, vkCore->GetRenderInstance(), vkCore->GetFlatSurface(), nullptr);
}

void RenderBackendFlat::Prepare(std::vector<std::unique_ptr<IGraphicsRenderpass>>& passes) {
    InitVertexIndexBuffers();

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

    // prepare shader
    if (passes.empty()) {
        VulkanDefaults::PrepareDefaultFlatRenderPasses(vkCore, scene, viewProj, RenderPasses,
                                                       swapchain->GetSwapchainImages());
    } else {
        LOGGER(LOGGER::INFO) << "Using custom render pass";
        this->RenderPasses = std::move(passes);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Handling events
//////////////////////////////////////////////////////////////////////////////////////////////////////////

void RenderBackendFlat::OnMouseMovement(double deltaX, double deltaY) {
    auto& cam = scene.CameraTransform();
    auto camMatrix = cam.GetMatrix();

    float sensitivity = 0.1f;
    float yaw = deltaX * sensitivity;
    float pitch = deltaY * sensitivity;

    glm::mat4 rotationYaw = glm::rotate(glm::mat4(1.0f), glm::radians(yaw), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 rotationPitch = glm::rotate(glm::mat4(1.0f), glm::radians(pitch), cam.RightVector());

    camMatrix = {rotationPitch * camMatrix};
    camMatrix = {rotationYaw * camMatrix};
    for (int i = 0; i < 3; ++i) {
        glm::vec3 column(camMatrix[0][i], camMatrix[1][i], camMatrix[2][i]);
        column = glm::normalize(column);
        camMatrix[0][i] = column.x;
        camMatrix[1][i] = column.y;
        camMatrix[2][i] = column.z;
    }

    scene.CameraTransform() = {camMatrix};
}

void RenderBackendFlat::OnKeyPressed(int keyCode) {
    float movementSensitivity = 0.02;
    auto& cam = scene.CameraTransform();
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
    WindowHandler::VkGetWindowSurface(vkCore->GetRenderInstance(), &vkCore->GetFlatSurface());

    VkBool32 presentSupport = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(vkCore->GetRenderPhysicalDevice(), vkCore->GetGraphicsQueueFamilyIndex(),
                                         vkCore->GetFlatSurface(), &presentSupport);

    if (!presentSupport) {
        LOGGER(LOGGER::WARNING) << "Graphics queue doesn't have present support";
        //TODO: graphics queue normaly have present support
    }

    LOGGER(LOGGER::DEBUG) << "Window created";
}
}    // namespace Graphics
}    // namespace XRLib
