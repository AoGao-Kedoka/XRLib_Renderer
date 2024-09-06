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

    struct ViewProjection {
        alignas(16) glm::mat4 view;
        alignas(16) glm::mat4 proj;
    };

    struct ModelPos {
        alignas(16) glm::mat4 model;
    };

};
}    // namespace Graphics
}    // namespace XRLib
