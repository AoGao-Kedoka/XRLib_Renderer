#pragma once

#include "EntityType/Mesh.h"
#include "Event/EventSystem.h"
#include "Event/Events.h"
#include "Logger.h"
#include "Utils/Util.h"

namespace XRLib {
class MeshManager {
   public:
    MeshManager(std::vector<Mesh*>& meshesContainer, std::vector<std::unique_ptr<Entity>>& hiearchyRoot);
    ~MeshManager();
    void WaitForAllMeshesToLoad();
    void LoadMeshAsync(const Mesh::MeshLoadConfig& loadConfig, Entity*& bindPtr, Entity* parent = nullptr);
    std::vector<Mesh*>& Meshes() { return meshes; }

   private:
    void LoadMesh(Mesh::MeshLoadConfig& loadConfig, Entity*& bindPtr, Entity* parent);
    void LoadMeshVerticesIndices(Mesh::MeshLoadConfig& meshLoadConfig, Mesh* newMesh, aiMesh* aiMesh);
    void LoadMeshTextures(Mesh::MeshLoadConfig& meshLoadConfig, Mesh* newMesh, aiMesh* aiMesh, const aiScene* scene);
    void LoadEmbeddedTextures(Mesh::MeshLoadConfig& meshLoadConfig, Mesh* newMesh, aiMesh* aiMesh, const aiScene* scene);
    void LoadSpecifiedTextures(Mesh::TextureData& texture, const std::string& path);

    void ProcessNode(aiNode* node, const aiScene* scene, Mesh::MeshLoadConfig& meshLoadConfig, Entity* parent, std::vector<std::future<void>>& loadFutures);
    void ProcessMesh(aiMesh* aiMesh, const aiScene* scene, Mesh::MeshLoadConfig& meshLoadConfig, Entity* parent);

    void HandleInvalidMesh(Mesh::MeshLoadConfig& meshLoadConfig, Mesh* newMesh);

   private:
    std::vector<Mesh*>& meshes;
    std::vector<std::unique_ptr<Entity>>& hiearchyRoot;

    // synchronization
    std::vector<std::future<void>> futures;
    std::mutex mutex;
};
}    // namespace XRLib
