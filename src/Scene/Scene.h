#pragma once

#include "Graphics/Primitives.h"
#include "Graphics/Window.h"
#include "Logger.h"

#include "MeshManager.h"
#include "EntityType/Entity.h"
#include "EntityType/Camera.h"
#include "EntityType/Light.h"

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
    std::vector<Mesh*>& Meshes() { return meshes; }
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

    std::vector<Light> lights;

    std::vector<std::unique_ptr<Entity>> sceneHierarchy;

    // store rendering required components along side the scene hiearchy
    std::vector<PointLight*> pointLights;
    std::vector<Mesh*> meshes;
    Camera* cam = nullptr;

    MeshManager meshManager{meshes, sceneHierarchy};

    // camera settings
    glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
    glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    Transform cameraTransform{glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp)};
};
}    // namespace XRLib
