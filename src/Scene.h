#pragma once

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <atomic>
#include <condition_variable>
#include <future>
#include <glm/glm.hpp>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

#include "Logger.h"

class Scene {
   public:
    Scene();
    ~Scene();
    Scene& LoadMeshAsync(const std::string& path);
    void WaitForAllMeshesToLoad();

    struct Vertex {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 texCoords;
    };

    struct Mesh {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        std::string name;
    };

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
