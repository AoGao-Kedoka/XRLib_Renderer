#pragma once

#include "Graphics/Primitives.h"
#include "Utils/Transform.h"
#include "Entity.h"

namespace XRLib {
class Mesh : public Entity{
   public:
    Mesh() : Entity{"DefaultMesh"} {}

    struct MeshLoadInfo {
        std::string meshPath{""};
        std::string texturePath{""};
        Transform transform;
        Mesh* destPtr{nullptr};
    };

    struct TextureData {
        std::vector<uint8_t> data;
        int textureWidth = 0;
        int textureHeight = 0;
        int textureChannels = 0;
    };

    std::vector<Graphics::Primitives::Vertex>& GetVerticies() { return vertices; }
    std::vector<uint16_t>& GetIndices() { return indices; }
    TextureData& GetTextureData() { return textureData; }

   private:
    std::vector<Graphics::Primitives::Vertex> vertices;
    std::vector<uint16_t> indices;
    TextureData textureData;

};
}    // namespace XRLib
