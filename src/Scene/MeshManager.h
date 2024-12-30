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
    void LoadMeshAsync(Mesh::MeshLoadInfo loadInfo, Entity* parent = nullptr);
    std::vector<Mesh*>& Meshes() { return meshes; }

    void AttachLeftControllerPose();
    void AttachRightControllerPose();

    void BindToPointer(Mesh*& meshPtr);

   private:
    void LoadMesh(const Mesh::MeshLoadInfo& meshLoadInfo);
    void LoadMeshVerticesIndices(const Mesh::MeshLoadInfo& meshLoadInfo, Mesh* newMesh, aiMesh* aiMesh);
    void LoadMeshTextures(const Mesh::MeshLoadInfo& meshLoadInfo, Mesh* newMesh, aiMesh* aiMesh, const aiScene* scene);
    void MeshLoadingThread();

    void HandleInvalidMesh(const Mesh::MeshLoadInfo& meshLoadInfo, Mesh* newMesh);

   private:
    std::vector<Mesh*>& meshes;
    std::vector<std::unique_ptr<Entity>>& hiearchyRoot;

    // synchronization
    std::vector<std::future<void>> futures;
    std::queue<Mesh::MeshLoadInfo> meshQueue;
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
