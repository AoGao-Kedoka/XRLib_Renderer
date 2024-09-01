#include "Transform.h"

Transform::Transform(glm::vec3 translation, glm::vec3 rotation,
                     float rotationRadians, glm::vec3 scale) {
    this->transform = LibMath::GetTransformationMatrix(translation, rotation,
                                                       rotationRadians, scale);
}

Transform::Transform(glm::mat4 transform) : transform{transform} {}
