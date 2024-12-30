#include "MeshManager.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace XRLib {

MeshManager::MeshManager(std::vector<Mesh*>& meshesContainer, std::vector<std::unique_ptr<Entity>>& hiearchyRoot)
    : stop{false}, meshes{meshesContainer}, hiearchyRoot{hiearchyRoot} {
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

void IncreaseLoadingStatusCounter(int& loadingStatusCounter, std::mutex& queueMutex, std::condition_variable& cv) {
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        loadingStatusCounter++;
        cv.notify_all();
    }
}

void IncreaseLoadingRegistrationcounter(int& loadingRegistrationCounter, std::mutex& queueMutex,
                                        std::condition_variable& cv) {
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        loadingRegistrationCounter++;
        cv.notify_all();
    }
}

void MeshManager::WaitForAllMeshesToLoad() {
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        cv.wait(lock, [this] { return loadingStatusCounter == meshes.size() - 1; });
    }

    for (auto& future : futures) {
        if (future.valid()) {
            future.get();
        }
    }
    futures.clear();
}

void MeshManager::LoadMeshAsync(Mesh::MeshLoadInfo loadInfo, Entity* parent) {
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        meshQueue.push(loadInfo);
    }
    cv.notify_all();
    IncreaseLoadingRegistrationcounter(loadingRegistrationCounter, queueMutex, cv);
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

    IncreaseLoadingStatusCounter(loadingStatusCounter, queueMutex, cv);

    auto& newMesh = meshLoadInfo.destPtr;
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        LOGGER(LOGGER::ERR) << importer.GetErrorString();
        HandleInvalidMesh(meshLoadInfo, newMesh);
        return;
    }

    if (scene->mNumMeshes != 1) {
        Util::ErrorPopup(
            "Loading models with multiple meshes is currently not supported.\nThis will be worked on very soon!");
    }

    for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
        aiMesh* aiMesh = scene->mMeshes[i];

        LoadMeshVerticesIndices(meshLoadInfo, newMesh, aiMesh);
        LoadMeshTextures(meshLoadInfo, newMesh, aiMesh, scene);
        newMesh->Rename(meshLoadInfo.meshPath);
        newMesh->GetTransform() = meshLoadInfo.transform;
        LOGGER(LOGGER::INFO) << "Loaded mesh: " << aiMesh->mName.C_Str();
    }

    if (loadingStatusCounter == meshes.size() - 1) {
        EventSystem::TriggerEvent(Events::XRLIB_EVENT_MESHES_LOADING_FINISHED);
    }
}

void MeshManager::LoadMeshVerticesIndices(const Mesh::MeshLoadInfo& meshLoadInfo, Mesh* newMesh, aiMesh* aiMesh) {
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
        newMesh->GetVerticies().push_back(vertex);
    }

    // Process indices
    for (unsigned int j = 0; j < aiMesh->mNumFaces; j++) {
        aiFace face = aiMesh->mFaces[j];
        for (unsigned int k = 0; k < face.mNumIndices; k++) {
            newMesh->GetIndices().push_back(face.mIndices[k]);
        }
    }
}

void MeshManager::LoadMeshTextures(const Mesh::MeshLoadInfo& meshLoadInfo, Mesh* newMesh, aiMesh* aiMesh,
                                   const aiScene* scene) {

    // get embedded textures
    if (aiMesh->mMaterialIndex >= 0) {
        aiMaterial* material = scene->mMaterials[aiMesh->mMaterialIndex];
        aiString texturePath;

        if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == AI_SUCCESS) {
            const aiTexture* embeddedTexture = scene->GetEmbeddedTexture(texturePath.C_Str());
            if (embeddedTexture) {
                if (embeddedTexture->mHeight == 0) {
                    // Compressed texture data
                    newMesh->GetTextureData().data.resize(embeddedTexture->mWidth);
                    memcpy(newMesh->GetTextureData().data.data(), embeddedTexture->pcData, embeddedTexture->mWidth);

                    int width, height, channels;
                    unsigned char* decodedData =
                        stbi_load_from_memory(reinterpret_cast<const unsigned char*>(embeddedTexture->pcData),
                                              embeddedTexture->mWidth, &width, &height, &channels, STBI_rgb_alpha);
                    if (decodedData) {
                        newMesh->GetTextureData().textureWidth = width;
                        newMesh->GetTextureData().textureHeight = height;
                        newMesh->GetTextureData().textureChannels = 4;
                        newMesh->GetTextureData().data.resize(width * height * 4);
                        memcpy(newMesh->GetTextureData().data.data(), decodedData, width * height * 4);
                        stbi_image_free(decodedData);
                    } else {
                        newMesh->GetTextureData().textureWidth = embeddedTexture->mWidth;
                        newMesh->GetTextureData().textureHeight = embeddedTexture->mHeight;
                        newMesh->GetTextureData().textureChannels = 4;
                        newMesh->GetTextureData().data.resize(newMesh->GetTextureData().textureWidth *
                                                              newMesh->GetTextureData().textureHeight * 4);
                        memcpy(newMesh->GetTextureData().data.data(), embeddedTexture->pcData,
                               newMesh->GetTextureData().data.size());
                    }
                } else {
                    // Raw texture data
                    newMesh->GetTextureData().textureWidth = embeddedTexture->mWidth;
                    newMesh->GetTextureData().textureHeight = embeddedTexture->mHeight;
                    newMesh->GetTextureData().textureChannels = 4;
                    newMesh->GetTextureData().data.resize(newMesh->GetTextureData().textureWidth *
                                                          newMesh->GetTextureData().textureHeight * 4);
                    memcpy(newMesh->GetTextureData().data.data(), embeddedTexture->pcData,
                           newMesh->GetTextureData().data.size());
                }
            }
        }
    }

    // get meshloadinfo specified texture
    if (newMesh->GetTextureData().data.empty() && !meshLoadInfo.texturePath.empty()) {
        unsigned char* imageData = stbi_load(meshLoadInfo.texturePath.c_str(), &newMesh->GetTextureData().textureWidth,
                                             &newMesh->GetTextureData().textureHeight,
                                             &newMesh->GetTextureData().textureChannels, STBI_rgb_alpha);

        if (imageData) {
            size_t imageSize = newMesh->GetTextureData().textureWidth * newMesh->GetTextureData().textureHeight * 4;
            newMesh->GetTextureData().textureChannels = 4;
            newMesh->GetTextureData().data.resize(imageSize);
            memcpy(newMesh->GetTextureData().data.data(), imageData, imageSize);
            stbi_image_free(imageData);
        } else {
            LOGGER(LOGGER::ERR) << "Failed to load texture: " << meshLoadInfo.texturePath;
        }
    }

    // last fallback, create temporary white texture
    if (newMesh->GetTextureData().data.empty()) {
        CreateTempTexture(*newMesh, 255);
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

void MeshManager::HandleInvalidMesh(const Mesh::MeshLoadInfo& meshLoadInfo, Mesh* newMesh) {
    Transform transform;
    newMesh->GetTransform() = transform;
    newMesh->Rename(meshLoadInfo.meshPath);
    CreateTempTexture(*newMesh, 255);
}

void MeshManager::AttachLeftControllerPose() {
    auto currentLoadingIndex = loadingRegistrationCounter;
    if (currentLoadingIndex < 0 || currentLoadingIndex > meshes.size()) {
        LOGGER(LOGGER::WARNING) << "Undefined behavior, ignored";
        return;
    }
    meshes[currentLoadingIndex]->Tags().push_back(Mesh::TAG::MESH_LEFT_CONTROLLER);

    EventSystem::Callback<Transform> positionCallback = [this, currentLoadingIndex](Transform transform) {
        meshes[currentLoadingIndex]->GetTransform() = transform;
    };
    EventSystem::RegisterListener<Transform>(Events::XRLIB_EVENT_LEFT_CONTROLLER_POSITION, positionCallback);
}

void MeshManager::AttachRightControllerPose() {
    auto currentLoadingIndex = loadingRegistrationCounter;
    if (currentLoadingIndex < 0 || currentLoadingIndex > meshes.size()) {
        LOGGER(LOGGER::WARNING) << "Undefined behavior, ignored";
        return;
    }
    meshes[currentLoadingIndex]->Tags().push_back(Mesh::TAG::MESH_RIGHT_CONTROLLER);

    EventSystem::Callback<Transform> positionCallback = [this, currentLoadingIndex](Transform transform) {
        meshes[currentLoadingIndex]->GetTransform() = transform;
    };
    EventSystem::RegisterListener<Transform>(Events::XRLIB_EVENT_RIGHT_CONTROLLER_POSITION, positionCallback);
}

void MeshManager::BindToPointer(Mesh*& meshPtr) {
    auto currentLoadingIndex = loadingRegistrationCounter;
    if (currentLoadingIndex < 0 || currentLoadingIndex > meshes.size() - 1) {
        LOGGER(LOGGER::WARNING) << "Undefined behavior, ignored";
        return;
    }
    meshPtr = meshes[currentLoadingIndex];
}
}    // namespace XRLib
