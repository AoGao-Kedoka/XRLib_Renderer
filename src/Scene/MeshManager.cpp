#include "MeshManager.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace XRLib {

MeshManager::MeshManager() : done{false}, stop{false} {
    workerThread = std::thread(&MeshManager::MeshLoadingThread, this);
}
MeshManager::~MeshManager() {
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        stop = true;
    }
    cv.notify_all();

    if (workerThread.joinable()) {
        workerThread.join();
    }
}
void MeshManager::WaitForAllMeshesToLoad() {
    std::unique_lock<std::mutex> lock(queueMutex);
    done = true;
    cv.notify_all();
    lock.unlock();

    for (auto& future : futures) {
        future.wait();
    }
}
void MeshManager::LoadMeshAsync(Mesh::MeshLoadInfo loadInfo) {
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        Mesh meshPlaceHolder;
        meshes.push_back(std::move(meshPlaceHolder));

        loadingIndex++;
        loadInfo.localLoadingIndex = loadingIndex;
        meshQueue.push(loadInfo);
    }
    cv.notify_all();
}

void CreateTempTexture(XRLib::Mesh& newMesh, uint8_t color) {
    Mesh::TextureData textureData;
    textureData.textureChannels = 4;
    textureData.textureHeight = 1;
    textureData.textureWidth = 1;
    textureData.data.resize(textureData.textureChannels * textureData.textureHeight * textureData.textureWidth, color);
    newMesh.GetTextureData() = textureData;
}
void MeshManager::LoadMesh(const Mesh::MeshLoadInfo& meshLoadInfo) {
    Assimp::Importer importer;

    const aiScene* scene =
        importer.ReadFile(meshLoadInfo.meshPath, aiProcess_Triangulate | aiProcess_FlipUVs |
                                                     aiProcess_JoinIdenticalVertices | aiProcess_PreTransformVertices);

    Mesh newMesh;
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        LOGGER(LOGGER::ERR) << importer.GetErrorString();
        Transform transform;
        newMesh.GetTransform() = transform;
        newMesh.GetName() = meshLoadInfo.meshPath;
        CreateTempTexture(newMesh, 255);
        AddNewMesh(newMesh, meshLoadInfo);
        return;
    }

    if (scene->mNumMeshes != 1) {
        Util::ErrorPopup("Loading models with multiple meshes is currently not supported.\nThis will be worked on very soon!");
    }

    for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
        aiMesh* aiMesh = scene->mMeshes[i];

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
            newMesh.GetVerticies().push_back(vertex);
        }

        // Process indices
        for (unsigned int j = 0; j < aiMesh->mNumFaces; j++) {
            aiFace face = aiMesh->mFaces[j];
            for (unsigned int k = 0; k < face.mNumIndices; k++) {
                newMesh.GetIndices().push_back(face.mIndices[k]);
            }
        }

        // Load texture data
        if (aiMesh->mMaterialIndex >= 0) {
            aiMaterial* material = scene->mMaterials[aiMesh->mMaterialIndex];
            aiString texturePath;

            if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == AI_SUCCESS) {
                const aiTexture* embeddedTexture = scene->GetEmbeddedTexture(texturePath.C_Str());
                if (embeddedTexture) {
                    if (embeddedTexture) {
                        if (embeddedTexture->mHeight == 0) {
                            // Compressed texture data
                            newMesh.GetTextureData().data.resize(embeddedTexture->mWidth);
                            memcpy(newMesh.GetTextureData().data.data(), embeddedTexture->pcData,
                                   embeddedTexture->mWidth);

                            int width, height, channels;
                            unsigned char* decodedData = stbi_load_from_memory(
                                reinterpret_cast<const unsigned char*>(embeddedTexture->pcData),
                                embeddedTexture->mWidth, &width, &height, &channels, STBI_rgb_alpha);
                            if (decodedData) {
                                newMesh.GetTextureData().textureWidth = width;
                                newMesh.GetTextureData().textureHeight = height;
                                newMesh.GetTextureData().textureChannels = 4;
                                newMesh.GetTextureData().data.resize(width * height * 4);
                                memcpy(newMesh.GetTextureData().data.data(), decodedData, width * height * 4);
                                stbi_image_free(decodedData);
                            } else {
                                newMesh.GetTextureData().textureWidth = embeddedTexture->mWidth;
                                newMesh.GetTextureData().textureHeight = embeddedTexture->mHeight;
                                newMesh.GetTextureData().textureChannels = 4;
                                newMesh.GetTextureData().data.resize(newMesh.GetTextureData().textureWidth *
                                                                     newMesh.GetTextureData().textureHeight * 4);
                                memcpy(newMesh.GetTextureData().data.data(), embeddedTexture->pcData,
                                       newMesh.GetTextureData().data.size());
                            }
                        } else {
                            // Raw texture data
                            newMesh.GetTextureData().textureWidth = embeddedTexture->mWidth;
                            newMesh.GetTextureData().textureHeight = embeddedTexture->mHeight;
                            newMesh.GetTextureData().textureChannels = 4;
                            newMesh.GetTextureData().data.resize(newMesh.GetTextureData().textureWidth *
                                                                 newMesh.GetTextureData().textureHeight * 4);
                            memcpy(newMesh.GetTextureData().data.data(), embeddedTexture->pcData,
                                   newMesh.GetTextureData().data.size());
                        }
                    }
                }
            }
        }

        if (newMesh.GetTextureData().data.empty() && !meshLoadInfo.texturePath.empty()) {
            unsigned char* imageData = stbi_load(
                meshLoadInfo.texturePath.c_str(), &newMesh.GetTextureData().textureWidth,
                &newMesh.GetTextureData().textureHeight, &newMesh.GetTextureData().textureChannels, STBI_rgb_alpha);

            if (imageData) {
                size_t imageSize = newMesh.GetTextureData().textureWidth * newMesh.GetTextureData().textureHeight * 4;
                newMesh.GetTextureData().textureChannels = 4;
                newMesh.GetTextureData().data.resize(imageSize);
                memcpy(newMesh.GetTextureData().data.data(), imageData, imageSize);
                stbi_image_free(imageData);
            } else {
                LOGGER(LOGGER::ERR) << "Failed to load texture: " << meshLoadInfo.texturePath;
            }
        }

        // Add the new mesh to the list
        {

            if (newMesh.GetTextureData().data.empty()) {
                CreateTempTexture(newMesh, 255);
            }

            newMesh.GetName() = meshLoadInfo.meshPath;
            newMesh.GetTransform() = meshLoadInfo.transform;
            AddNewMesh(newMesh, meshLoadInfo);
            LOGGER(LOGGER::INFO) << "Loaded mesh: " << aiMesh->mName.C_Str();
        }
    }

    if (loadingStatuscounter == meshes.size()) {
        EventSystem::TriggerEvent(Events::XRLIB_EVENT_MESHES_LOADING_FINISHED);
    }
}
void MeshManager::MeshLoadingThread() {
    while (true) {
        Mesh::MeshLoadInfo loadInfo;
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            cv.wait(lock, [this] { return !meshQueue.empty() || stop; });
            if (stop && meshQueue.empty()) {
                return;
            }
            loadInfo = meshQueue.front();
            meshQueue.pop();
        }

        std::future<void> future = std::async(std::launch::async, &MeshManager::LoadMesh, this, loadInfo);
        futures.push_back(std::move(future));
    }
}
void MeshManager::AddNewMesh(const Mesh& newMesh, const Mesh::MeshLoadInfo& meshLoadInfo) {
    std::lock_guard<std::mutex> lock(queueMutex);
    meshes[meshLoadInfo.localLoadingIndex] = newMesh;
    loadingStatuscounter++;
}

void MeshManager::AttachLeftControllerPose() {
    auto currentLoadingIndex = loadingIndex;
    if (currentLoadingIndex < 0 || currentLoadingIndex > meshes.size()) {
        LOGGER(LOGGER::WARNING) << "Undefined behavior, ignored";
        return;
    }
    EventSystem::Callback<> tagCallback = [this, currentLoadingIndex]() {
        meshes[currentLoadingIndex].Tags().push_back(Mesh::MESH_TAG::MESH_LEFT_CONTROLLER);
    };
    EventSystem::RegisterListener(Events::XRLIB_EVENT_MESHES_LOADING_FINISHED, tagCallback);

    EventSystem::Callback<Transform> positionCallback = [this, currentLoadingIndex](Transform transform) {
        meshes[currentLoadingIndex].GetTransform() = transform;
    };
    EventSystem::RegisterListener<Transform>(Events::XRLIB_EVENT_LEFT_CONTROLLER_POSITION, positionCallback);
}
void MeshManager::AttachRightControllerPose() {
    auto currentLoadingIndex = loadingIndex;
    if (currentLoadingIndex < 0 || currentLoadingIndex > meshes.size()) {
        LOGGER(LOGGER::WARNING) << "Undefined behavior, ignored";
        return;
    }
    EventSystem::Callback<> tagCallback = [this, currentLoadingIndex]() {
        meshes[currentLoadingIndex].Tags().push_back(Mesh::MESH_TAG::MESH_RIGHT_CONTROLLER);
    };
    EventSystem::RegisterListener(Events::XRLIB_EVENT_MESHES_LOADING_FINISHED, tagCallback);

    EventSystem::Callback<Transform> positionCallback = [this, currentLoadingIndex](Transform transform) {
        meshes[currentLoadingIndex].GetTransform() = transform;
    };
    EventSystem::RegisterListener<Transform>(Events::XRLIB_EVENT_RIGHT_CONTROLLER_POSITION, positionCallback);
}

void MeshManager::BindToPointer(Mesh*& meshPtr) {
    auto currentLoadingIndex = loadingIndex;
    if (currentLoadingIndex < 0 || currentLoadingIndex > meshes.size()) {
        LOGGER(LOGGER::WARNING) << "Undefined behavior, ignored";
        return;
    }
    EventSystem::Callback<> callback = [this, &meshPtr, currentLoadingIndex]() {
        meshPtr = &meshes[currentLoadingIndex];
    };
    EventSystem::RegisterListener(Events::XRLIB_EVENT_MESHES_LOADING_FINISHED, callback);
}
}    // namespace XRLib
