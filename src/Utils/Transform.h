#pragma once
#include "Utils/LibMath.h"

class Transform {
   public:
    Transform(glm::vec3 translation, glm::vec3 rotation, float rotationRadians,
              glm::vec3 scale);
    Transform(glm::mat4 transform);
    Transform() = default;

    glm::mat4 GetMatrix() { return transform; }

   private:
    glm::mat4 transform{1.0f};
};
