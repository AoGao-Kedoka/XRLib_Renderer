#pragma once

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <atomic>
#include <condition_variable>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

#include "Graphics/Primitives.h"
#include "Logger.h"

class Scene {
   public:
    struct Mesh {
        std::vector<Primitives::Vertex> vertices;
        std::vector<unsigned int> indices;
        std::string name;
    };

   public:
    Scene();
    ~Scene();
    Scene& LoadMeshAsync(const std::string& path);
    void WaitForAllMeshesToLoad();
    bool CheckTaskRunning();
    std::vector<Mesh> Meshes() { return meshes; }

   private:
    void LoadMesh(const std::string& path);
    void MeshLoadingThread();
    std::vector<Mesh> meshes;

    std::vector<std::future<void>> futures;
    std::queue<std::string> meshQueue;
    std::condition_variable cv;
    std::mutex queueMutex;
    std::atomic<bool> done;
    std::atomic<bool> stop;
    std::thread workerThread;
};
