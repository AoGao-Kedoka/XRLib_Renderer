#include "Scene.h"

namespace XRLib {
Scene::Scene() {
    AddMandatoryMainCamera();

    EventSystem::Callback<> allMeshesLoadCallback = [this]() {
        // validate meshes
        Validate();

        // validate lights. if no light, add a default white light
        if (pointLights.empty()) {
            LOGGER(LOGGER::WARNING) << "No light in the scene is defined, creating default light at location (0,0,0)";
            AddPointLights({}, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), 0.5f);
        }
    };
    EventSystem::RegisterListener(Events::XRLIB_EVENT_APPLICATION_INIT_STARTED, allMeshesLoadCallback);
}
void Scene::AddMandatoryMainCamera() {
    auto camera = std::make_unique<Camera>();
    camera->Tags().push_back(Entity::TAG::MAIN_CAMERA);
    cam = camera.get();
    sceneHierarchy.push_back(std::move(camera));
}

Scene& Scene::LoadMeshAsync(Mesh::MeshLoadInfo loadInfo, Entity* parent) {
    Entity* _ = nullptr;
    meshManager.LoadMeshAsync(loadInfo, _, parent);
    return *this;
}

Scene& Scene::LoadMeshAsyncWithBinding(Mesh::MeshLoadInfo loadInfo, Entity*& bindPtr, Entity* parent) {
    meshManager.LoadMeshAsync(loadInfo, bindPtr, parent);
    return *this;
}

void Scene::Validate() {
    for (auto& mesh : meshes) {
        if (Util::VectorContains(mesh->Tags(), Mesh::TAG::MESH_LEFT_CONTROLLER) &&
            Util::VectorContains(mesh->Tags(), Mesh::TAG::MESH_RIGHT_CONTROLLER)) {
            Util::ErrorPopup("Mesh can't be left and right controller and the same time");
        }
    }
}

Scene& Scene::AttachEntityToLeftControllerPose(Entity*& entity) {
    EventSystem::Callback<> tagCallback = [this, &entity] (){
        if (entity == nullptr)
            return;
        entity->Tags().push_back(Entity::TAG::MESH_LEFT_CONTROLLER);
    };
    EventSystem::RegisterListener(Events::XRLIB_EVENT_MESHES_LOADING_FINISHED, tagCallback);

    EventSystem::Callback<Transform> positionCallback = [this, &entity](Transform transform) {
        if (entity == nullptr)
            return;
        entity->GetRelativeTransform() = transform;
    };
    EventSystem::RegisterListener<Transform>(Events::XRLIB_EVENT_LEFT_CONTROLLER_POSITION, positionCallback);

    return *this;
}

Scene& Scene::AttachEntityToRightcontrollerPose(Entity*& entity) {
    EventSystem::Callback<> tagCallback = [this, &entity] (){
        if (entity == nullptr)
            return;
        entity->Tags().push_back(Entity::TAG::MESH_RIGHT_CONTROLLER);
    };
    EventSystem::RegisterListener(Events::XRLIB_EVENT_MESHES_LOADING_FINISHED, tagCallback);

    EventSystem::Callback<Transform> positionCallback = [this, &entity](Transform transform) {
        if (entity == nullptr)
            return;
        entity->GetRelativeTransform() = transform;
    };
    EventSystem::RegisterListener<Transform>(Events::XRLIB_EVENT_RIGHT_CONTROLLER_POSITION, positionCallback);

    return *this;
}

Scene& Scene::AddPointLights(Transform transform, glm::vec4 color, float intensity, Entity* parent) {
    Entity* _ = nullptr;
    AddPointLightsWithBinding(transform, color, intensity, _, parent);
    return *this;
}

Scene& Scene::AddPointLightsWithBinding(Transform transform, glm::vec4 color, float intensity, Entity*& bindPtr,
                                        Entity* parent) {
    auto light = std::make_unique<PointLight>(transform, color, intensity);
    bindPtr = light.get();
    AddPointLightsInternal(light, parent);
    return *this;
}

Scene& Scene::AddPointLights(Transform transform, glm::vec4 color, float intensity, std::string name, Entity* parent) {
    Entity* _ = nullptr;
    AddPointLightsWithBinding(transform, color, intensity, name, _, parent);
    return *this;
}

Scene& Scene::AddPointLightsWithBinding(Transform transform, glm::vec4 color, float intensity, std::string name,
                                        Entity*& bindPtr, Entity* parent) {
    auto light = std::make_unique<PointLight>(transform, color, intensity, name);
    bindPtr = light.get();
    AddPointLightsInternal(light, parent);
    return *this;
}

void Scene::AddPointLightsInternal(std::unique_ptr<PointLight>& light, Entity* parent) {
    if (parent == nullptr) {
        Entity::AddEntity(light, sceneHierarchy, &pointLights);
    } else {
        Entity::AddEntity(light, parent, &pointLights);
    }
}

Scene& Scene::AddEntity(Transform transform, std::string name, Entity* parent) {
    Entity* _ = nullptr;
    AddEntityWithBinding(transform, name, _, parent);
    return *this;
}

Scene& Scene::AddEntityWithBinding(Transform transform, std::string name, Entity*& bindPtr, Entity* parent) {
    auto entity = std::make_unique<Entity>(transform, name);
    bindPtr = entity.get();
    if (parent == nullptr)
        Entity::AddEntity(entity, sceneHierarchy);
    else {
        Entity::AddEntity(entity, parent);
    }
    return *this;
}

void Scene::WaitForAllMeshesToLoad() {
    meshManager.WaitForAllMeshesToLoad();
}

}    // namespace XRLib
