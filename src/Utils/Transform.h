#pragma once
#include "Utils/MathUtil.h"

namespace XRLib {
class Transform {
   public:
    Transform(glm::vec3 translation, glm::vec3 rotation, float rotationRadians,
              glm::vec3 scale);
    Transform(glm::mat4 transform);
    Transform() = default;

    glm::vec3 FrontVector() const;
    glm::vec3 BackVector() const;
    glm::vec3 LeftVector() const;
    glm::vec3 RightVector() const;
    glm::vec3 DownVector() const;
    glm::vec3 UpVector() const;
    glm::vec3 Position() const;

    Transform& Rotate(glm::vec3 rotation, float rotationRadians);
    Transform& Translate(glm::vec3 translation);
    Transform& Scale(glm::vec3 scale);

    glm::mat4 GetMatrix() const;

   private:
    glm::mat4 transform{1.0f};
};
}    // namespace XRLib
