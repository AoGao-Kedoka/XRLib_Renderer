#pragma once

#include "Graphics/Primitives.h"
#include "Graphics/Window.h"
#include "Logger.h"
#include "MeshManager.h"
#include "Utils/Transform.h"

namespace XRLib {
class Scene {
   public:
    struct Light {
        Transform transform;
        glm::vec4 color;
        float intensity;
    };

   public:
    Scene();
    ~Scene() = default;

    void Validate();

    MeshManager& GetMeshManager() { return meshManager; }
    std::vector<Mesh>& Meshes() { return meshManager.Meshes(); }
    Scene& LoadMeshAsync(Mesh::MeshLoadInfo loadInfo);
    Scene& AttachLeftControllerPose();
    Scene& AttachRightControllerPose();
    Scene& BindToPointer(Mesh*& meshPtr);
    void WaitForAllMeshesToLoad();

    Transform& CameraTransform() { return cameraTransform; }
    glm::mat4 CameraProjection();

    Scene& AddLights(const Light& light);
    std::vector<Light>& Lights() { return lights; }

   private:
    MeshManager meshManager;

    std::vector<Light> lights;

    // camera settings
    glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
    glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    Transform cameraTransform{glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp)};
};
}    // namespace XRLib
