#include "Scene.h"

namespace XRLib {
Scene::Scene() {

    sceneHierarchy.push_back(std::make_unique<Camera>());

    EventSystem::Callback<> allMeshesLoadCallback = [this]() {
        // validate meshes
        Validate();

        // validate lights. if no light, add a default white light
        if (lights.empty()) {
            LOGGER(LOGGER::WARNING) << "No light in the scene is defined, creating default light at location (0,0,0)";
            Transform defaultTransform;
            lights.push_back({defaultTransform, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), 0.5f});
            sceneHierarchy.push_back(std::make_unique<PointLight>(defaultTransform, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), 0.5f));
        }
    };
    EventSystem::RegisterListener(Events::XRLIB_EVENT_APPLICATION_INIT_STARTED, allMeshesLoadCallback);
}

Scene& Scene::LoadMeshAsync(Mesh::MeshLoadInfo loadInfo) {
    meshManager.LoadMeshAsync(loadInfo);
    return *this;
}

void Scene::Validate() {
    for (auto& mesh : meshes) {
        if (Util::VectorContains(mesh->Tags(), Mesh::MESH_TAG::MESH_LEFT_CONTROLLER) &&
            Util::VectorContains(mesh->Tags(), Mesh::MESH_TAG::MESH_RIGHT_CONTROLLER)) {
            Util::ErrorPopup("Mesh can't be left and right controller and the same time");
        }
    }
}

glm::mat4 Scene::CameraProjection() {
    auto [width, height] = Graphics::WindowHandler::GetFrameBufferSize();
    auto proj = glm::perspective(glm::radians(45.0f), width / (float)height, 0.1f, 10.0f);
    proj[1][1] *= -1;
    return proj;
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

Scene& Scene::AddLights(const Light& light) {
    lights.push_back(light);
    return *this;
}

void Scene::WaitForAllMeshesToLoad() {
    meshManager.WaitForAllMeshesToLoad();
}

}    // namespace XRLib
