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

    struct ViewProjectionStereo{
        glm::mat4 views[2];
        glm::mat4 proj[2];
    };

};
}    // namespace Graphics
}    // namespace XRLib
