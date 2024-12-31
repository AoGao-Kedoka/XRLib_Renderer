#pragma once

#include "Entity.h"
#include "Graphics/Window.h"
#include "pch.h"

namespace XRLib {
class Camera : public Entity {
   public:
    Camera(glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f),
           glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -0.1f), const std::string& name = "DefaultCamera");
    ~Camera() = default;

    glm::mat4 CameraProjection(float fov = 45.0f, float near = 0.1f, float far = 1000.0f);

   private:
    glm::vec3 up;
    glm::vec3 front;
    glm::vec3 position;
};
}    // namespace XRLib
