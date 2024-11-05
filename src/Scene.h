#pragma once

#include "Graphics/Primitives.h"
#include "Graphics/Window.h"
#include "Logger.h"
#include "Utils/Transform.h"

namespace XRLib {
class Scene {
   public:
    enum TAG {
        MESH_LEFT_CONTROLLER,
        MESH_RIGHT_CONTROLLER,
    };

    struct Mesh {
        std::vector<Graphics::Primitives::Vertex> vertices;
        std::vector<uint16_t> indices;
        std::string name;
        Transform transform;
        std::vector<uint8_t> textureData;
        int textureWidth = 0;
        int textureHeight = 0;
        int textureChannels = 0;

        std::vector<TAG> tags;
    };

    struct MeshLoadInfo {
        std::string meshPath{""};
        std::string texturePath{""};
        Transform transform;
        int localLoadingIndex = -1;
    };

    struct Light {
        Transform transform;
        glm::vec4 color;
        float intensity;
    };

   public:
    Scene();
    ~Scene();
    Scene& LoadMeshAsync(MeshLoadInfo loadInfo);

    Scene& AttachLeftControllerPose();
    Scene& AttachRightControllerPose();
    Scene& BindToPointer(Mesh*& meshPtr);
    Scene& AddLights(const Light& light);

    void WaitForAllMeshesToLoad();

    Transform& CameraTransform() { return cameraTransform; }
    glm::mat4 CameraProjection();

    std::vector<Mesh>& Meshes() { return meshes; }

    std::vector<Light>& Lights() { return lights; }

   private:
    void LoadMesh(const MeshLoadInfo& meshLoadInfo);
    void MeshLoadingThread();
    void Validate();
    void AddNewMesh(const Mesh& newMesh, const MeshLoadInfo& meshLoadInfo);

   private:
    std::vector<Mesh> meshes;
    std::vector<Light> lights;

    // camera settings
    glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
    glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    Transform cameraTransform{glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp)};

    // synchronization
    std::vector<std::future<void>> futures;
    std::queue<MeshLoadInfo> meshQueue;
    std::condition_variable cv;
    std::mutex queueMutex;
    std::atomic<bool> done;
    std::atomic<bool> stop;
    std::thread workerThread;
    int loadingIndex = -1;
    bool MeshLoaded() { return loadingIndex != -1; }
};
}    // namespace XRLib
