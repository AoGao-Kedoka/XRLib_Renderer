#include "Scene.h"

namespace XRLib {
Scene::Scene() : done(false), stop(false) {
    workerThread = std::thread(&Scene::MeshLoadingThread, this);
}

Scene::~Scene() {
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        stop = true;
    }
    cv.notify_all();
    if (workerThread.joinable()) {
        workerThread.join();
    }
}

Scene& Scene::LoadMeshAsync(const MeshLoadInfo& loadInfo) {
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        meshQueue.push(loadInfo);
    }
    cv.notify_all();
    return *this;
}

void Scene::WaitForAllMeshesToLoad() {
    std::unique_lock<std::mutex> lock(queueMutex);
    done = true;
    cv.notify_all();
    lock.unlock();

    for (auto& future : futures) {
        future.get();
    }
}

void Scene::LoadMesh(const MeshLoadInfo& meshLoadInfo) {
    Assimp::Importer importer;

    const aiScene* scene =
        importer.ReadFile(meshLoadInfo.meshPath, aiProcess_Triangulate | aiProcess_FlipUVs |
                                        aiProcess_JoinIdenticalVertices);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
        !scene->mRootNode) {
        LOGGER(LOGGER::ERR) << importer.GetErrorString();
        return;
    }

    for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
        aiMesh* aiMesh = scene->mMeshes[i];
        Mesh newMesh;

        for (unsigned int j = 0; j < aiMesh->mNumVertices; j++) {
            Graphics::Primitives::Vertex vertex;
            vertex.position = {aiMesh->mVertices[j].x, aiMesh->mVertices[j].y,
                               aiMesh->mVertices[j].z};
            if (aiMesh->mNormals) {
                vertex.normal = {aiMesh->mNormals[j].x, aiMesh->mNormals[j].y,
                                 aiMesh->mNormals[j].z};
            }
            if (aiMesh->mTextureCoords[0]) {
                vertex.texCoords = {aiMesh->mTextureCoords[0][j].x,
                                    aiMesh->mTextureCoords[0][j].y};
            } else {
                vertex.texCoords = {0.0f, 0.0f};
            }
            newMesh.vertices.push_back(vertex);
        }

        // Extract indices
        for (unsigned int j = 0; j < aiMesh->mNumFaces; j++) {
            aiFace face = aiMesh->mFaces[j];
            for (unsigned int k = 0; k < face.mNumIndices; k++) {
                newMesh.indices.push_back(face.mIndices[k]);
            }
        }

        {
            newMesh.name = meshLoadInfo.meshPath;
            newMesh.transform = meshLoadInfo.transform;
            newMesh.texturePath = meshLoadInfo.texturePath;
            std::lock_guard<std::mutex> lock(queueMutex);
            meshes.push_back(newMesh);
        }
    }

    LOGGER(LOGGER::INFO) << "Loaded mesh: " << meshLoadInfo.meshPath;
}

void Scene::MeshLoadingThread() {
    while (true) {
        MeshLoadInfo loadInfo;
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            cv.wait(lock, [this] { return !meshQueue.empty() || stop; });
            if (stop && meshQueue.empty()) {
                return;
            }
            loadInfo = meshQueue.front();
            meshQueue.pop();
        }

        std::future<void> future =
            std::async(std::launch::async, &Scene::LoadMesh, this, loadInfo);
        futures.push_back(std::move(future));
    }
}

bool Scene::CheckTaskRunning() {
    std::lock_guard<std::mutex> lock(queueMutex);
    for (auto& future : futures) {
        if (future.wait_for(std::chrono::seconds(0)) !=
            std::future_status::ready) {
            return true;
        }
    }
    return false;
}

glm::mat4 Scene::GetCameraProjection() {
    auto [width, height] = Graphics::WindowHandler::GetFrameBufferSize();
    auto proj = glm::perspective(glm::radians(45.0f), width / (float)height,
                                 0.1f, 10.0f);
    proj[1][1] *= -1;
    return proj;
}

}    // namespace XRLib
