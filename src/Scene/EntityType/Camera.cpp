#include "Camera.h"

namespace XRLib {

Camera::Camera(glm::vec3 cameraPos, glm::vec3 cameraUp, glm::vec3 cameraFront, const std::string& name)
    : position{cameraPos}, up{cameraUp}, front{cameraFront},
      Entity{glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp), name} {}

glm::mat4 Camera::CameraProjection() {
    auto [width, height] = Graphics::WindowHandler::GetFrameBufferSize();
    auto proj = glm::perspective(glm::radians(45.0f), width / (float)height, 0.1f, 10.0f);
    proj[1][1] *= -1;
    return proj;
}
}    // namespace XRLib