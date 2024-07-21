#pragma once

#include <glm/glm.hpp>

class Primitives {
   public:
    struct Vertex {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 texCoords;
    };
};