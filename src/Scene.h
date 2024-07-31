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
#include "Utils/LAMath.h"

class Scene {
   public:
    struct Mesh {
        std::vector<Primitives::Vertex> vertices;
        std::vector<unsigned int> indices;
        std::string name;
        glm::mat4 translation{1.0f};
    };

   public:
    Scene();
    ~Scene();
    Scene& LoadMeshAsync(const std::string& path,
                         glm::mat4 transformation = glm::mat4{1.0f});
    Scene& LoadMeshAsync(const std::string& path, glm::vec3 translation,
                         glm::vec3 rotation,float rotationRadians,
                         glm::vec3 scale);
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
