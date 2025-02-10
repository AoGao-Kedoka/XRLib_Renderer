#pragma once

#include "Entity.h"
#include "Graphics/Primitives.h"
#include "Utils/Transform.h"

namespace XRLib {
class Mesh : public Entity {
   public:
    Mesh() : Entity{"DefaultMesh"} {}

    struct MeshLoadConfig {
        std::string meshPath{""};

        Transform transform{};

        std::string diffuseTexturePath{""};
        std::string normalTexturePath{""};
        std::string roughnessTexturePath{""};
        std::string emissiveTexturePath{""};
        std::string metallicTexturePath{""};
    };

    struct TextureData {
        std::vector<uint8_t> textureData;
        int textureWidth = 0;
        int textureHeight = 0;
        int textureChannels = 0;
    };

    std::vector<Graphics::Primitives::Vertex>& GetVerticies() { return vertices; }
    std::vector<uint16_t>& GetIndices() { return indices; }

    TextureData Diffuse{{255, 255, 255, 255}, 1, 1, 4};
    TextureData Normal{{128, 128, 255, 255}, 1, 1, 4};
    TextureData Metallic{{0, 0, 0, 0}, 1, 1, 4};
    TextureData Roughness{{255, 255, 255, 255}, 1, 1, 4};
    TextureData Emissive{{0, 0, 0, 0}, 1, 1, 4};

   private:
    std::vector<Graphics::Primitives::Vertex> vertices;
    std::vector<uint16_t> indices;
};
}    // namespace XRLib
