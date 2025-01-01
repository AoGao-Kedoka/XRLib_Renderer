#pragma once

#include "Graphics/Primitives.h"
#include "Graphics/Window.h"
#include "Logger.h"

#include "EntityType/Camera.h"
#include "EntityType/Entity.h"
#include "EntityType/Light.h"
#include "MeshManager.h"

namespace XRLib {
class Scene {
   public:
    Scene();
    ~Scene() = default;

    void Validate();

    MeshManager& GetMeshManager() { return meshManager; }
    std::vector<Mesh*>& Meshes() { return meshes; }

    Scene& LoadMeshAsync(Mesh::MeshLoadInfo loadInfo, Entity* parent = nullptr);
    Scene& LoadMeshAsyncWithBinding(Mesh::MeshLoadInfo loadInfo, Entity*& bindPtr, Entity* parent = nullptr);

    void WaitForAllMeshesToLoad();

    Scene& AttachEntityToLeftControllerPose(Entity*& entity);
    Scene& AttachEntityToRightcontrollerPose(Entity*& entity);

    Scene& AddEntity(Transform transform, std::string name, Entity* parent = nullptr);
    Scene& AddEntityWithBinding(Transform transform, std::string name, Entity*& bindPtr, Entity* parent = nullptr);

    Scene& AddPointLights(Transform transform, glm::vec4 color, float intensity, Entity* parent = nullptr);
    Scene& AddPointLightsWithBinding(Transform transform, glm::vec4 color, float intensity, Entity*& bindPtr,
                          Entity* parent = nullptr);

    Scene& AddPointLights(Transform transform, glm::vec4 color, float intensity, std::string name,
                          Entity* parent = nullptr);
    Scene& AddPointLightsWithBinding(Transform transform, glm::vec4 color, float intensity, std::string name, Entity*& bindPtr,
                          Entity* parent = nullptr);

    std::vector<PointLight*>& PointLights() { return pointLights; }

    Camera*& MainCamera() { return cam; }

    const std::vector<std::unique_ptr<Entity>>& GetHiearchy() const { return sceneHierarchy; }

   private:
    void AddPointLightsInternal(std::unique_ptr<PointLight>& light, Entity* parent);
    void AddMandatoryMainCamera();

   private:
    std::vector<std::unique_ptr<Entity>> sceneHierarchy;

    // store rendering required components along side the scene hiearchy
    std::vector<PointLight*> pointLights;
    std::vector<Mesh*> meshes;
    Camera* cam = nullptr;

    MeshManager meshManager{meshes, sceneHierarchy};
};

inline void buildTreeStr(Entity* node, std::ostringstream& oss, const std::string& prefix = "", bool isLast = true) {
    oss << prefix << (isLast ? "+-- " : "|-- ") << node->GetName() << "\n";

    std::string childPrefix = prefix + (isLast ? "    " : "|   ");

    for (size_t i = 0; i < node->GetChilds().size(); ++i) {
        // Recursively print each child
        buildTreeStr(node->GetChilds()[i].get(), oss, childPrefix, i == node->GetChilds().size() - 1);
    }
}

inline std::ostream& operator<<(std::ostream& str, const Scene& scene) {
    str << "\nScene hiearchy:\n";
    std::ostringstream oss;
    for (size_t i = 0; i < scene.GetHiearchy().size(); ++i) {
        buildTreeStr(scene.GetHiearchy()[i].get(), oss);
    }
    str << oss.str();
    return str;
}

}    // namespace XRLib
