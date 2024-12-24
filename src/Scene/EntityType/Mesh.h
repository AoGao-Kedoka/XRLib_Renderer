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

        // used by program
        int localLoadingIndex = -1;
        Mesh* destPtr;
    };

    enum MESH_TAG {
        MESH_LEFT_CONTROLLER,
        MESH_RIGHT_CONTROLLER,
    };

    struct TextureData {
        std::vector<uint8_t> data;
        int textureWidth = 0;
        int textureHeight = 0;
        int textureChannels = 0;
    };

    std::vector<Graphics::Primitives::Vertex>& GetVerticies() { return vertices; }
    std::vector<uint16_t>& GetIndices() { return indices; }
    Transform& GetTransform() { return transform; }
    TextureData& GetTextureData() { return textureData; }
    std::vector<MESH_TAG>& Tags() { return tags; }

   private:
    std::vector<Graphics::Primitives::Vertex> vertices;
    std::vector<uint16_t> indices;
    TextureData textureData;

    std::vector<MESH_TAG> tags;
};
}    // namespace XRLib
