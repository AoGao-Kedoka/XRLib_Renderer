#include "Scene.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

static int LOADING_STATUS_COUNTER{0};

namespace XRLib {
Scene::Scene() : done(false), stop(false) {
    workerThread = std::thread(&Scene::MeshLoadingThread, this);

    EventSystem::Callback<> allMeshesLoadCallback = [this]() {
        // validate meshes
        Validate();

        // validate lights. if no light, add a default white light
        if (lights.empty()) {
            LOGGER(LOGGER::WARNING) << "No light in the scene is defined, creating default light at location (0,0,0)";
            Transform defaultTransform;
            lights.push_back({defaultTransform, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), 0.5f});
        }
    };
    EventSystem::RegisterListener(Events::XRLIB_EVENT_APPLICATION_INIT_STARTED, allMeshesLoadCallback);
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

Scene& Scene::LoadMeshAsync(MeshLoadInfo loadInfo) {
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        Mesh meshPlaceHolder;
        meshes.push_back(std::move(meshPlaceHolder));

        loadingIndex++;
        loadInfo.localLoadingIndex = loadingIndex;
        meshQueue.push(loadInfo);
    }
    cv.notify_all();
    return *this;
}

void Scene::Validate() {
    for (const auto& mesh : meshes) {
        if (Util::VectorContains(mesh.tags, TAG::MESH_LEFT_CONTROLLER) &&
            Util::VectorContains(mesh.tags, TAG::MESH_RIGHT_CONTROLLER)) {
            Util::ErrorPopup("Mesh can't be left and right controller and the same time");
        }
    }
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
                                                     aiProcess_JoinIdenticalVertices | aiProcess_PreTransformVertices);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        LOGGER(LOGGER::ERR) << importer.GetErrorString();
        return;
    }

    for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
        aiMesh* aiMesh = scene->mMeshes[i];
        Mesh newMesh;

        // Process vertices
        for (unsigned int j = 0; j < aiMesh->mNumVertices; j++) {
            Graphics::Primitives::Vertex vertex;
            vertex.position = {aiMesh->mVertices[j].x, aiMesh->mVertices[j].y, aiMesh->mVertices[j].z};
            if (aiMesh->mNormals) {
                vertex.normal = {aiMesh->mNormals[j].x, aiMesh->mNormals[j].y, aiMesh->mNormals[j].z};
            }
            if (aiMesh->mTextureCoords[0]) {
                vertex.texCoords = {aiMesh->mTextureCoords[0][j].x, aiMesh->mTextureCoords[0][j].y};
            } else {
                vertex.texCoords = {0.0f, 0.0f};
            }
            newMesh.vertices.push_back(vertex);
        }

        // Process indices
        for (unsigned int j = 0; j < aiMesh->mNumFaces; j++) {
            aiFace face = aiMesh->mFaces[j];
            for (unsigned int k = 0; k < face.mNumIndices; k++) {
                newMesh.indices.push_back(face.mIndices[k]);
            }
        }

        // Load texture data
        if (aiMesh->mMaterialIndex >= 0) {
            aiMaterial* material = scene->mMaterials[aiMesh->mMaterialIndex];
            aiString texturePath;

            if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == AI_SUCCESS) {
                const aiTexture* embeddedTexture = scene->GetEmbeddedTexture(texturePath.C_Str());
                if (embeddedTexture) {
                    if (embeddedTexture->mHeight == 0) {
                        newMesh.textureData.resize(embeddedTexture->mWidth);
                        memcpy(newMesh.textureData.data(), embeddedTexture->pcData, embeddedTexture->mWidth);

                        int width, height, channels;
                        unsigned char* decodedData =
                            stbi_load_from_memory(reinterpret_cast<const unsigned char*>(embeddedTexture->pcData),
                                                  embeddedTexture->mWidth, &width, &height, &channels, STBI_rgb_alpha);
                        if (decodedData) {
                            newMesh.textureWidth = width;
                            newMesh.textureHeight = height;
                            newMesh.textureChannels = 4;
                            newMesh.textureData.resize(width * height * 4);
                            memcpy(newMesh.textureData.data(), decodedData, width * height * 4);
                            stbi_image_free(decodedData);
                        } else {
                            newMesh.textureWidth = embeddedTexture->mWidth;
                            newMesh.textureHeight = embeddedTexture->mHeight;
                            newMesh.textureChannels = 4;
                            newMesh.textureData.resize(newMesh.textureWidth * newMesh.textureHeight * 4);
                            memcpy(newMesh.textureData.data(), embeddedTexture->pcData, newMesh.textureData.size());
                        }
                    } else {
                        newMesh.textureWidth = embeddedTexture->mWidth;
                        newMesh.textureHeight = embeddedTexture->mHeight;
                        newMesh.textureChannels = 4;
                        newMesh.textureData.resize(newMesh.textureWidth * newMesh.textureHeight * 4);
                        memcpy(newMesh.textureData.data(), embeddedTexture->pcData, newMesh.textureData.size());
                    }
                }
            }
        }

        if (newMesh.textureData.empty() && !meshLoadInfo.texturePath.empty()) {
            unsigned char* imageData = stbi_load(meshLoadInfo.texturePath.c_str(), &newMesh.textureWidth,
                                                 &newMesh.textureHeight, &newMesh.textureChannels, STBI_rgb_alpha);

            if (imageData) {
                size_t imageSize = newMesh.textureWidth * newMesh.textureHeight * 4;
                newMesh.textureChannels = 4;
                newMesh.textureData.resize(imageSize);
                memcpy(newMesh.textureData.data(), imageData, imageSize);
                stbi_image_free(imageData);
            } else {
                LOGGER(LOGGER::ERR) << "Failed to load texture: " << meshLoadInfo.texturePath;
            }
        }

        // Add the new mesh to the list
        {

            if (newMesh.textureData.empty()) {
                // create temporary white texture
                newMesh.textureChannels = 4;
                newMesh.textureHeight = 1;
                newMesh.textureWidth = 1;
                newMesh.textureData.resize(newMesh.textureChannels * newMesh.textureHeight * newMesh.textureWidth, 255);
            }

            newMesh.name = meshLoadInfo.meshPath;
            newMesh.transform = meshLoadInfo.transform;
            std::lock_guard<std::mutex> lock(queueMutex);
            meshes[meshLoadInfo.localLoadingIndex] = newMesh;
            LOADING_STATUS_COUNTER++;
            LOGGER(LOGGER::INFO) << "Loaded mesh: " << meshLoadInfo.meshPath;
        }
    }

    if (LOADING_STATUS_COUNTER == meshes.size()) {
        EventSystem::TriggerEvent(Events::XRLIB_EVENT_MESHES_LOADING_FINISHED);
    }
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

        std::future<void> future = std::async(std::launch::async, &Scene::LoadMesh, this, loadInfo);
        futures.push_back(std::move(future));
    }
}

bool Scene::CheckTaskRunning() {
    std::lock_guard<std::mutex> lock(queueMutex);
    for (auto& future : futures) {
        if (future.wait_for(std::chrono::seconds(0)) != std::future_status::ready) {
            return true;
        }
    }
    return false;
}

glm::mat4 Scene::CameraProjection() {
    auto [width, height] = Graphics::WindowHandler::GetFrameBufferSize();
    auto proj = glm::perspective(glm::radians(45.0f), width / (float)height, 0.1f, 10.0f);
    proj[1][1] *= -1;
    return proj;
}

Scene& Scene::AttachLeftControllerPose() {
    auto currentLoadingIndex = loadingIndex;
    if (currentLoadingIndex < 0 || currentLoadingIndex > meshes.size()) {
        LOGGER(LOGGER::WARNING) << "Undefined behavior, ignored";
        return *this;
    }
    EventSystem::Callback<> tagCallback = [this, currentLoadingIndex]() {
        meshes[currentLoadingIndex].tags.push_back(TAG::MESH_LEFT_CONTROLLER);
    };
    EventSystem::RegisterListener(Events::XRLIB_EVENT_MESHES_LOADING_FINISHED, tagCallback);

    EventSystem::Callback<Transform> positionCallback = [this, currentLoadingIndex](Transform transform) {
        meshes[currentLoadingIndex].transform = transform;
    };
    EventSystem::RegisterListener<Transform>(Events::XRLIB_EVENT_LEFT_CONTROLLER_POSITION, positionCallback);
    return *this;
}

Scene& Scene::AttachRightControllerPose() {
    auto currentLoadingIndex = loadingIndex;
    if (currentLoadingIndex < 0 || currentLoadingIndex > meshes.size()) {
        LOGGER(LOGGER::WARNING) << "Undefined behavior, ignored";
        return *this;
    }
    EventSystem::Callback<> tagCallback = [this, currentLoadingIndex]() {
        meshes[currentLoadingIndex].tags.push_back(TAG::MESH_RIGHT_CONTROLLER);
    };
    EventSystem::RegisterListener(Events::XRLIB_EVENT_MESHES_LOADING_FINISHED, tagCallback);

    EventSystem::Callback<Transform> positionCallback = [this, currentLoadingIndex](Transform transform) {
        meshes[currentLoadingIndex].transform = transform;
    };
    EventSystem::RegisterListener<Transform>(Events::XRLIB_EVENT_RIGHT_CONTROLLER_POSITION, positionCallback);
    return *this;
}

Scene& Scene::BindToPointer(Mesh*& meshPtr) {
    auto currentLoadingIndex = loadingIndex;
    if (currentLoadingIndex < 0 || currentLoadingIndex > meshes.size()) {
        LOGGER(LOGGER::WARNING) << "Undefined behavior, ignored";
        return *this;
    }
    EventSystem::Callback<> callback = [this, &meshPtr, currentLoadingIndex]() {
        meshPtr = &meshes[currentLoadingIndex];
    };
    EventSystem::RegisterListener(Events::XRLIB_EVENT_MESHES_LOADING_FINISHED, callback);
    return *this;
}

Scene& Scene::AddLights(const Light& light) {
    lights.push_back(light);
    return *this;
}

}    // namespace XRLib
