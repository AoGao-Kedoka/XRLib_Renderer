#pragma once

#include "Graphics/Primitives.h"
#include "Graphics/Window.h"
#include "Logger.h"
#include "Utils/Transform.h"

namespace XRLib {
class Scene {
   public:
    struct Mesh {
        std::vector<Graphics::Primitives::Vertex> vertices;
        std::vector<uint16_t> indices;
        std::string name;
        Transform transform;
        std::vector<uint8_t> textureData;
        int textureWidth = 0;
        int textureHeight = 0;
        int textureChannels = 0; 
    };

    struct MeshLoadInfo {
        std::string meshPath{""};
        std::string texturePath{""};
        Transform transform;
    };

   public:
    Scene();
    ~Scene();
    Scene& LoadMeshAsync(const MeshLoadInfo& loadInfo);
    void WaitForAllMeshesToLoad();
    bool CheckTaskRunning();

    std::vector<Mesh>& Meshes() { return meshes; }
    std::vector<Transform>& Lights() { return lights; }
    Transform& Camera() { return camera; }
    glm::mat4 GetCameraProjection();

   private:
    void LoadMesh(const MeshLoadInfo& meshLoadInfo);
    void MeshLoadingThread();
    std::vector<Mesh> meshes;
    std::vector<Transform> lights;
    Transform camera{glm::lookAt(glm::vec3(3.0f, 3.0f, 3.0f),
                                 glm::vec3(0.0f, 0.0f, 0.0f),
                                 glm::vec3(0.0f, 0.0f, 1.0f))};

    std::vector<std::future<void>> futures;
    std::queue<MeshLoadInfo> meshQueue;
    std::condition_variable cv;
    std::mutex queueMutex;
    std::atomic<bool> done;
    std::atomic<bool> stop;
    std::thread workerThread;
};
}    // namespace XRLib
