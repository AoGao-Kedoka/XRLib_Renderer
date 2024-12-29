#include "Scene.h"
#include "Scene.tpp"

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
    auto meshPlaceHolder = std::make_unique<Mesh>();
    loadInfo.destPtr = meshPlaceHolder.get();
    AddEntityInternal<Mesh>(meshPlaceHolder, parent, &meshes);

    meshManager.LoadMeshAsync(loadInfo, parent);
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

Scene& Scene::AttachLeftControllerPose() {
    meshManager.AttachLeftControllerPose();
    return *this;
}

Scene& Scene::AttachRightControllerPose() {
    meshManager.AttachRightControllerPose();
    return *this;
}

Scene& Scene::BindToPointer(Mesh*& meshPtr) {
    meshManager.BindToPointer(meshPtr);
    return *this;
}

Scene& Scene::AddPointLights(Transform transform, glm::vec4 color, float intensity, Entity* parent) {
    auto light = std::make_unique<PointLight>(transform, color, intensity);
    AddPointLightsInternal(light, parent);
    return *this;
}

Scene& Scene::AddPointLights(Transform transform, glm::vec4 color, float intensity, std::string name, Entity* parent) {
    auto light = std::make_unique<PointLight>(transform, color, intensity, name);
    AddPointLightsInternal(light, parent);
    return *this;
}

void Scene::AddPointLightsInternal(std::unique_ptr<PointLight>& light, Entity* parent) {
    AddEntityInternal<PointLight>(light, parent, &pointLights);
}

Scene& Scene::AddEntity(Transform transform, std::string name, Entity* parent) {
    auto entity = std::make_unique<Entity>(transform, name);
    AddEntityInternal<Entity>(entity, parent);
    return *this;
}

void Scene::WaitForAllMeshesToLoad() {
    meshManager.WaitForAllMeshesToLoad();
}

}    // namespace XRLib
