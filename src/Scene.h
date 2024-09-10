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

    class Camera {
       public:
        Transform& GetTransform() { return camera; }

        glm::vec3 FrontVector() { return cameraFront; }
        glm::vec3 BackVector() { return -cameraFront; }
        glm::vec3 UpVector() { return cameraUp; }
        glm::vec3 DownVector() { return -cameraUp; }
        glm::vec3 LeftVector() { return glm::cross(cameraFront, cameraUp); }
        glm::vec3 RightVector () { return -glm::cross(cameraFront, cameraUp); }

        glm::vec3 TranslationVector() { return cameraPos; }
        
        void SetCameraFront(glm::vec3 front);
        void SetCameraUp(glm::vec3 up);
        void SetCameraPos(glm::vec3 pos);

        glm::mat4 GetCameraProjection();

       private:
        glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
        glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
        glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);

        Transform camera{
            glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp)};
    };

   public:
    Scene();
    ~Scene();
    Scene& LoadMeshAsync(const MeshLoadInfo& loadInfo);
    void WaitForAllMeshesToLoad();
    bool CheckTaskRunning();

    Camera& Cam() { return cam; };

    std::vector<Mesh>& Meshes() { return meshes; }
    std::vector<Transform>& Lights() { return lights; }

   private:
    void LoadMesh(const MeshLoadInfo& meshLoadInfo);
    void MeshLoadingThread();
    std::vector<Mesh> meshes;
    std::vector<Transform> lights;
    Camera cam;

    std::vector<std::future<void>> futures;
    std::queue<MeshLoadInfo> meshQueue;
    std::condition_variable cv;
    std::mutex queueMutex;
    std::atomic<bool> done;
    std::atomic<bool> stop;
    std::thread workerThread;
};
}    // namespace XRLib
