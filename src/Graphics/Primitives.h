#pragma once

#include <pch.h>

namespace XRLib {
namespace Graphics {
class Primitives {
   public:
    struct Vertex {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 texCoords;
    };
};
}    // namespace Graphics
}    // namespace XRLib
