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
        cv.wait(lock, [this] { return loadingStatusCounter == loadingRegistrationCounter; });
    }

    for (auto& future : futures) {
        if (future.valid()) {
            future.get();
        }
    }
    futures.clear();

    loadingStatusCounter = -1;
    loadingRegistrationCounter = -1;
}

void MeshManager::LoadMeshAsync(Mesh::MeshLoadInfo loadInfo, Entity*& bindPtr, Entity* parent) {
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        meshQueue.push({loadInfo, bindPtr});
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

void MeshManager::LoadMesh(const Mesh::MeshLoadInfo& meshLoadInfo, Entity*& bindPtr) {
    Assimp::Importer importer;

    const aiScene* scene =
        importer.ReadFile(meshLoadInfo.meshPath, aiProcess_Triangulate | aiProcess_FlipUVs |
                                                     aiProcess_JoinIdenticalVertices | aiProcess_PreTransformVertices);

    auto meshPathValid = [&]() -> bool {
        return !scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode;
    };

    auto createMeshPlaceHolder = [&]() -> Mesh* {
        auto meshPlaceHolder = std::make_unique<Mesh>();
        Mesh* meshPtr = meshPlaceHolder.get();
        bindPtr = meshPtr;
        meshes.push_back(meshPtr);
        Entity::AddEntity(meshPlaceHolder, hiearchyRoot, &meshes);
        return meshPtr;
    };

    auto loadMeshData = [&](Mesh* mesh, aiMesh* aiMesh) {
        LoadMeshVerticesIndices(meshLoadInfo, mesh, aiMesh);
        LoadMeshTextures(meshLoadInfo, mesh, aiMesh, scene);
        mesh->Rename(aiMesh->mName.C_Str());
        mesh->GetTransform() = meshLoadInfo.transform;
        LOGGER(LOGGER::INFO) << "Loaded mesh: " << aiMesh->mName.C_Str();
    };


    if (meshPathValid()) {
        LOGGER(LOGGER::ERR) << importer.GetErrorString();
        auto newMesh = createMeshPlaceHolder();
        HandleInvalidMesh(meshLoadInfo, newMesh);
        IncreaseLoadingStatusCounter(loadingStatusCounter, queueMutex, cv);
        return;
    }

    if (scene->mNumMeshes > 0) {
        auto entityParent = (scene->mNumMeshes > 1) ? std::make_unique<Entity>() : nullptr;

        for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
            aiMesh* aiMesh = scene->mMeshes[i];
            auto meshChild = std::make_unique<Mesh>();
            auto newMesh = meshChild.get();

            if (entityParent) {
                bindPtr = entityParent.get();
                Entity::AddEntity(meshChild, entityParent.get(), &meshes);
            } else {
                bindPtr = newMesh;
                Entity::AddEntity(meshChild, hiearchyRoot, &meshes);
            }

            loadMeshData(newMesh, aiMesh);
        }

        if (entityParent) {
            Entity::AddEntity(entityParent, hiearchyRoot);
        }
    }

    IncreaseLoadingStatusCounter(loadingStatusCounter, queueMutex, cv);

    if (loadingStatusCounter == loadingRegistrationCounter) {
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
        std::unique_lock<std::mutex> lock(queueMutex);
        cv.wait(lock, [this] { return !meshQueue.empty() || stop; });
        if (stop && meshQueue.empty()) {
            return;
        }

        auto [loadInfo, entityPtr] = meshQueue.front();
        meshQueue.pop();
        std::future<void> future = std::async(std::launch::async, &MeshManager::LoadMesh, this, loadInfo, std::ref(entityPtr));
        futures.push_back(std::move(future));
    }
}

void MeshManager::HandleInvalidMesh(const Mesh::MeshLoadInfo& meshLoadInfo, Mesh* newMesh) {
    Transform transform;
    newMesh->GetTransform() = transform;
    newMesh->Rename(meshLoadInfo.meshPath);
    CreateTempTexture(*newMesh, 255);
}

}    // namespace XRLib
