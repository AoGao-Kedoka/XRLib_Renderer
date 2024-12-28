#pragma once

#include "Event/EventSystem.h"
#include "Event/Events.h"
#include "Logger.h"
#include "Utils/Util.h"
#include "EntityType/Mesh.h"

namespace XRLib {
class MeshManager {
   public:
    MeshManager(std::vector<Mesh*>& meshesContainer, std::vector<std::unique_ptr<Entity>>& hiearchyRoot);
    ~MeshManager();
    void WaitForAllMeshesToLoad();
    void LoadMeshAsync(Mesh::MeshLoadInfo loadInfo, Entity* parent = nullptr);
    std::vector<Mesh*>& Meshes() { return meshes; }
    int GetCurrentLoadingIndex() { return loadingIndex; }

    void AttachLeftControllerPose();
    void AttachRightControllerPose();

    void BindToPointer(Mesh*& meshPtr);

   private:
    void LoadMesh(const Mesh::MeshLoadInfo& meshLoadInfo);
    void MeshLoadingThread();
    void AddNewMesh(Mesh& newMesh, const Mesh::MeshLoadInfo& meshLoadInfo);
    bool MeshLoaded() { return loadingIndex != -1; }

   private:
    std::vector<Mesh*>& meshes;
    std::vector<std::unique_ptr<Entity>>& hiearchyRoot;
    // synchronization
    std::vector<std::future<void>> futures;
    std::queue<Mesh::MeshLoadInfo> meshQueue;
    std::condition_variable cv;
    std::mutex queueMutex;
    std::atomic<bool> done;
    std::atomic<bool> stop;
    std::thread workerThread;
    int loadingIndex{-1};
    int loadingStatuscounter{0};

};
}    // namespace XRLib
