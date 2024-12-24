#pragma once

#include "Entity.h"

namespace XRLib {
class Camera : public Entity{
   public:
    Camera(glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f),
           glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -0.1f), std::string name = "DefaultCamera");
    ~Camera() = default;

   private:
    glm::vec3 up;
    glm::vec3 front;
    glm::vec3 position;
};
}