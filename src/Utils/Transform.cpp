#include "Transform.h"

Transform::Transform(glm::vec3 translation, glm::vec3 rotation, float rotationRadians, glm::vec3 scale) {
    this->transform = LibMath::GetTransformationMatrix(translation, rotation, rotationRadians, scale);
}

Transform::Transform(glm::mat4 transform) : transform{transform} {}

glm::mat4 Transform::GetMatrix() const {
    return transform;
}

glm::vec3 Transform::FrontVector() const {
    glm::mat4 inverted = glm::inverse(transform);
    return normalize(glm::vec3(inverted[2])) * glm::vec3(-1);
}

glm::vec3 Transform::BackVector() const {
    return -FrontVector();
}

glm::vec3 Transform::DownVector() const {
    glm::mat4 inverted = glm::inverse(transform);
    return normalize(glm::vec3(inverted[1])) * glm::vec3(-1);
}

glm::vec3 Transform::UpVector() const {
    return -DownVector();
}

glm::vec3 Transform::Position() const {
    return glm::vec3(transform[3]);
}

glm::vec3 Transform::LeftVector() const {
    glm::mat4 inverted = glm::inverse(transform);
    return normalize(glm::vec3(inverted[0])) * glm::vec3(-1);
}

glm::vec3 Transform::RightVector() const {
    return -LeftVector();
}
