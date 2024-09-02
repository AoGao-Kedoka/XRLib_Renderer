#pragma once

#include "Graphics/Primitives.h"
#include "Logger.h"
#include "Utils/Transform.h"

namespace XRLib {
class Scene {
   public:
    struct Mesh {
        std::vector<Graphics::Primitives::Vertex> vertices;
        std::vector<unsigned int> indices;
        std::string name;
        glm::mat4 transform{1.0f};
    };

   public:
    Scene();
    ~Scene();
    Scene& LoadMeshAsync(const std::string& path, Transform transform);
    Scene& LoadMeshAsync(const std::string& path);
    void WaitForAllMeshesToLoad();
    bool CheckTaskRunning();
    std::vector<Mesh> Meshes() { return meshes; }

   private:
    void LoadMesh(const std::string& path, glm::mat4 translation);
    void MeshLoadingThread();
    std::vector<Mesh> meshes;

    std::vector<std::future<void>> futures;
    std::queue<std::pair<std::string, glm::mat4>> meshQueue;
    std::condition_variable cv;
    std::mutex queueMutex;
    std::atomic<bool> done;
    std::atomic<bool> stop;
    std::thread workerThread;
};
}    // namespace XRLib
