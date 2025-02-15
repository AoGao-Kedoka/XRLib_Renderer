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
    void LoadMeshAsync(Mesh::MeshLoadConfig loadConfig, Entity*& bindPtr, Entity* parent = nullptr);
    std::vector<Mesh*>& Meshes() { return meshes; }

   private:
    void LoadMesh(const Mesh::MeshLoadConfig& meshLoadConfig, Entity*& bindPtr);
    void LoadMeshVerticesIndices(const Mesh::MeshLoadConfig& meshLoadConfig, Mesh* newMesh, aiMesh* aiMesh);
    void LoadMeshTextures(const Mesh::MeshLoadConfig& meshLoadConfig, Mesh* newMesh, aiMesh* aiMesh, const aiScene* scene);
    void LoadEmbeddedTextures(const Mesh::MeshLoadConfig& meshLoadConfig, Mesh* newMesh, aiMesh* aiMesh, const aiScene* scene);
    void LoadSpecifiedTextures(Mesh::TextureData& texture, const std::string& path);
    void MeshLoadingThread();

    void HandleInvalidMesh(const Mesh::MeshLoadConfig& meshLoadConfig, Mesh* newMesh);

   private:
    std::vector<Mesh*>& meshes;
    std::vector<std::unique_ptr<Entity>>& hiearchyRoot;

    // synchronization
    std::vector<std::future<void>> futures;
    std::queue<std::pair<Mesh::MeshLoadConfig, Entity*&>> meshQueue;
    std::condition_variable cv;
    std::mutex queueMutex;
    std::atomic<bool> stop;
    std::thread workerThread;


    // count for loading tasks registered
    int loadingRegistrationCounter{-1};
    // count when loading tasks finished
    int loadingStatusCounter{-1};
};
}    // namespace XRLib
