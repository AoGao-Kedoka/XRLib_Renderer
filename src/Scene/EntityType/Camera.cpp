#include "Camera.h"

namespace XRLib {

Camera::Camera(glm::vec3 cameraPos, glm::vec3 cameraUp, glm::vec3 cameraFront, const std::string& name)
    : cameraUp{cameraUp}, Entity{glm::inverse(glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp)), name} {
    LOGGER(LOGGER::DEBUG) << glm::to_string(glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp));
}

glm::mat4 Camera::CameraProjection(float near, float far) {
    auto [width, height] = Graphics::WindowHandler::GetFrameBufferSize();
    auto proj = glm::perspective(glm::radians(FOV), width / (float)height, near, far);
    proj[1][1] *= -1;
    return proj;
}

glm::vec3 extractCamPosition(const glm::mat4& viewMatrix) {
    glm::mat3 rotation = glm::mat3(viewMatrix);
    glm::vec3 translation = glm::vec3(viewMatrix[3]);
    return -glm::transpose(rotation) * translation;
}

void Camera::UpdateCamera(glm::vec3 cameraFront) {
    glm::vec3 cameraPos = extractCamPosition(glm::inverse(this->transform.GetMatrix()));
    this->transform = {glm::inverse(glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp))};
}

glm::mat4 Camera::CameraView() {
    return glm::inverse(this->GetGlobalTransform().GetMatrix());
}

}    // namespace XRLib
