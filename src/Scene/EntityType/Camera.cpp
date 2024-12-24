#include "Camera.h"

namespace XRLib {

Camera::Camera(glm::vec3 cameraPos, glm::vec3 cameraUp, glm::vec3 cameraFront, std::string name)
    : position{cameraPos}, up{cameraUp}, front{cameraFront},
      Entity{glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp), name} {
}

}    // namespace XRLib