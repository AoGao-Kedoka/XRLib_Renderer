#pragma once

#include "Graphics/Primitives.h"
#include "Utils/Transform.h"
#include "Entity.h"

namespace XRLib {
class Mesh : public Entity{
   public:
    Mesh() : Entity{"DefaultMesh"} {}

    struct MeshLoadConfig {
        std::string meshPath{""};

        Transform transform{};

        std::string diffuseTexturePath{""};
        std::string normalTexturePath{""};
        std::string roughnessTexturePath{""};
        std::string emissiveTexturePath{""};
    };

    struct TextureData {
        std::vector<uint8_t> textureData;
        int textureWidth = 0;
        int textureHeight = 0;
        int textureChannels = 0;
    };

    std::vector<Graphics::Primitives::Vertex>& GetVerticies() { return vertices; }
    std::vector<uint16_t>& GetIndices() { return indices; }

    TextureData Diffuse;
    TextureData Normal;
    TextureData Roughness;
    TextureData Emissive;

   private:
    std::vector<Graphics::Primitives::Vertex> vertices;
    std::vector<uint16_t> indices;
};
}    // namespace XRLib
