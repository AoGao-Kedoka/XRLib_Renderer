#include "Scene.h"

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

Scene& Scene::LoadMeshAsync(const std::string& path) {
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        meshQueue.push(path);
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

void Scene::LoadMesh(const std::string& filename) {
    Assimp::Importer importer;

    const aiScene* scene =
        importer.ReadFile(filename, aiProcess_Triangulate | aiProcess_FlipUVs |
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
            Vertex vertex;
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
            std::lock_guard<std::mutex> lock(queueMutex);
            meshes.push_back(newMesh);
        }
    }

    LOGGER(LOGGER::INFO) << "Loaded mesh: " << filename;
}

void Scene::MeshLoadingThread() {
    while (true) {
        std::string filename;
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            cv.wait(lock, [this] { return !meshQueue.empty() || stop; });
            if (stop && meshQueue.empty()) {
                return;
            }
            filename = meshQueue.front();
            meshQueue.pop();
        }

        std::future<void> future =
            std::async(std::launch::async, &Scene::LoadMesh, this, filename);
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