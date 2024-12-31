#include "Camera.h"

namespace XRLib {

Camera::Camera(glm::vec3 cameraPos, glm::vec3 cameraUp, glm::vec3 cameraFront, const std::string& name)
    : position{cameraPos}, up{cameraUp}, front{cameraFront},
      Entity{glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp), name} {}

glm::mat4 Camera::CameraProjection(float fov, float near, float far) {
    auto [width, height] = Graphics::WindowHandler::GetFrameBufferSize();
    auto proj = glm::perspective(glm::radians(fov), width / (float)height, near, far);
    proj[1][1] *= -1;
    return proj;
}
}    // namespace XRLib
