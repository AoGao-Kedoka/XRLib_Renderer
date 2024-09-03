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
        bool operator==(const Vertex& other) const {
            return position == other.position && normal == other.normal &&
                   texCoords == other.texCoords;
        }
    };

    struct UniformBufferObject {
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 proj;
    };

};
}    // namespace Graphics
}    // namespace XRLib
