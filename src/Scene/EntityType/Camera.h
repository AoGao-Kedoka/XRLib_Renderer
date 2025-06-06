#pragma once

#include "Entity.h"
#include "Graphics/Window.h"
#include "pch.h"

namespace XRLib {
class Camera : public Entity {
   public:
    Camera(glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f),
           glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f), const std::string& name = "DefaultCamera");
    ~Camera() = default;

    glm::mat4 CameraProjection(float near = 0.1f, float far = 1000.0f);
    glm::mat4 CameraView();

    void UpdateCamera(glm::vec3 cameraFront);

   public:
    float Yaw{-90};
    float Pitch{0};
    float FOV{45.0};

   private:
    glm::vec3 cameraUp;
};
}    // namespace XRLib
