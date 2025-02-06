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

        loadingStatusCounter = -1;
        loadingRegistrationCounter = -1;
    }
}

void MeshManager::LoadMeshAsync(Mesh::MeshLoadConfig loadConfig, Entity*& bindPtr, Entity* parent) {
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        meshQueue.push({loadConfig, bindPtr});
    }
    cv.notify_all();
    IncreaseLoadingRegistrationcounter(loadingRegistrationCounter, queueMutex, cv);
}

void CreateTempTexture(XRLib::Mesh& newMesh, uint8_t color) {
    Mesh::TextureData textureData;
    textureData.textureChannels = 4;
    textureData.textureHeight = 1;
    textureData.textureWidth = 1;
    textureData.textureData.resize(textureData.textureChannels * textureData.textureHeight * textureData.textureWidth, color);
    newMesh.Diffuse = textureData;
}

void MeshManager::LoadMesh(const Mesh::MeshLoadConfig& meshLoadConfig, Entity*& bindPtr) {
    Assimp::Importer importer;

    const aiScene* scene =
        importer.ReadFile(meshLoadConfig.meshPath, aiProcess_Triangulate | aiProcess_FlipUVs |
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
        LoadMeshVerticesIndices(meshLoadConfig, mesh, aiMesh);
        LoadMeshTextures(meshLoadConfig, mesh, aiMesh, scene);
        mesh->Rename(aiMesh->mName.C_Str());
        mesh->GetLocalTransform() = meshLoadConfig.transform;
        LOGGER(LOGGER::INFO) << "Loaded mesh: " << aiMesh->mName.C_Str();
    };

    if (meshPathValid()) {
        LOGGER(LOGGER::ERR) << importer.GetErrorString();
        auto newMesh = createMeshPlaceHolder();
        HandleInvalidMesh(meshLoadConfig, newMesh);
        IncreaseLoadingStatusCounter(loadingStatusCounter, queueMutex, cv);
        return;
    }

    if (scene->mNumMeshes > 0) {
        auto entityParent = (scene->mNumMeshes > 1)
                                ? std::make_unique<Entity>(Util::GetFileNameWithoutExtension(meshLoadConfig.meshPath))
                                : nullptr;

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

void MeshManager::LoadMeshVerticesIndices(const Mesh::MeshLoadConfig& meshLoadConfig, Mesh* newMesh, aiMesh* aiMesh) {
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

void MeshManager::LoadMeshTextures(const Mesh::MeshLoadConfig& meshLoadConfig, Mesh* newMesh, aiMesh* aiMesh,
                                   const aiScene* scene) {

    // get embedded textures
    LoadEmbeddedTextures(newMesh, aiMesh, scene);

    // get meshloadinfo specified texture
    LoadSpecifiedTextures(newMesh->Diffuse, meshLoadConfig.diffuseTexturePath);
    LoadSpecifiedTextures(newMesh->Normal, meshLoadConfig.normalTexturePath);
    LoadSpecifiedTextures(newMesh->Roughness, meshLoadConfig.roughnessTexturePath);
    LoadSpecifiedTextures(newMesh->Emissive, meshLoadConfig.emissiveTexturePath);

    // last fallback, create temporary white texture
    if (newMesh->Diffuse.textureData.empty()) {
        CreateTempTexture(*newMesh, 255);
    }
}

void MeshManager::LoadEmbeddedTextures(Mesh* newMesh, aiMesh* aiMesh, const aiScene* scene) {
    if (aiMesh->mMaterialIndex < 0) {
        return;
    }
    aiMaterial* material = scene->mMaterials[aiMesh->mMaterialIndex];
    aiString texturePath;
    auto loadTextureFromEmbedding = [](const aiTexture* texture, Mesh::TextureData& textureData) {
            if (texture->mHeight == 0) {
                // Compressed texture data
                textureData.textureData.resize(texture->mWidth);
                memcpy(textureData.textureData.data(), texture->pcData, texture->mWidth);

                int width, height, channels;
                unsigned char* decodedData =
                    stbi_load_from_memory(reinterpret_cast<const unsigned char*>(texture->pcData),
                                          texture->mWidth, &width, &height, &channels, STBI_rgb_alpha);
                if (decodedData) {
                    textureData.textureWidth = width;
                    textureData.textureHeight = height;
                    textureData.textureChannels = 4;
                    textureData.textureData.resize(width * height * 4);
                    memcpy(textureData.textureData.data(), decodedData, width * height * 4);
                    stbi_image_free(decodedData);
                } else {
                    textureData.textureWidth = texture->mWidth;
                    textureData.textureHeight = texture->mHeight;
                    textureData.textureChannels = 4;
                    textureData.textureData.resize(textureData.textureWidth * textureData.textureHeight * 4);
                    memcpy(textureData.textureData.data(), texture->pcData, textureData.textureData.size());
                }
            } else {
                // Raw texture data
                textureData.textureWidth = texture->mWidth;
                textureData.textureHeight = texture->mHeight;
                textureData.textureChannels = 4;
                textureData.textureData.resize(textureData.textureWidth * textureData.textureHeight * 4);
                memcpy(textureData.textureData.data(), texture->pcData, textureData.textureData.size());
            }
    };

    // Diffuse
    if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == AI_SUCCESS) {
        const aiTexture* embeddedTexture = scene->GetEmbeddedTexture(texturePath.C_Str());
        if (embeddedTexture) {
            loadTextureFromEmbedding(embeddedTexture, newMesh->Diffuse);
        }
    }

    // Normal Map
    if (material->GetTexture(aiTextureType_NORMALS, 0, &texturePath) == AI_SUCCESS ||
        material->GetTexture(aiTextureType_HEIGHT, 0, &texturePath) == AI_SUCCESS) {
        const aiTexture* embeddedTexture = scene->GetEmbeddedTexture(texturePath.C_Str());
        if (embeddedTexture) {
            loadTextureFromEmbedding(embeddedTexture, newMesh->Normal);
        }
    }

    // Roughness
    if (material->GetTexture(aiTextureType_DIFFUSE_ROUGHNESS, 0, &texturePath) == AI_SUCCESS) {
        const aiTexture* embeddedTexture = scene->GetEmbeddedTexture(texturePath.C_Str());
        if (embeddedTexture) {
            loadTextureFromEmbedding(embeddedTexture, newMesh->Roughness);
        }
    }

    // Emissive
    if (material->GetTexture(aiTextureType_EMISSIVE, 0, &texturePath) == AI_SUCCESS) {
        const aiTexture* embeddedTexture = scene->GetEmbeddedTexture(texturePath.C_Str());
        if (embeddedTexture) {
            loadTextureFromEmbedding(embeddedTexture, newMesh->Emissive);
        }
    }
}

void MeshManager::LoadSpecifiedTextures(Mesh::TextureData& texture, const std::string& path) {
    if (texture.textureData.empty() && !path.empty()) {
        unsigned char* imageData = stbi_load(path.c_str(), &texture.textureWidth, &texture.textureHeight,
                      &texture.textureChannels, STBI_rgb_alpha);

        if (imageData) {
            size_t imageSize = texture.textureWidth * texture.textureHeight * 4;
            texture.textureChannels = 4;
            texture.textureData.resize(imageSize);
            memcpy(texture.textureData.data(), imageData, imageSize);
            stbi_image_free(imageData);
        } else {
            LOGGER(LOGGER::ERR) << "Failed to load texture: " << path;
        }
    }
}

void MeshManager::MeshLoadingThread() {
    while (true) {
        std::unique_lock<std::mutex> lock(queueMutex);
        cv.wait(lock, [this] { return !meshQueue.empty() || stop; });
        if (stop && meshQueue.empty()) {
            return;
        }

        auto [loadConfig, entityPtr] = meshQueue.front();
        meshQueue.pop();
        std::future<void> future =
            std::async(std::launch::async, &MeshManager::LoadMesh, this, loadConfig, std::ref(entityPtr));
        futures.push_back(std::move(future));
    }
}

void MeshManager::HandleInvalidMesh(const Mesh::MeshLoadConfig& meshLoadConfig, Mesh* newMesh) {
    Transform transform;
    newMesh->GetLocalTransform() = transform;
    newMesh->Rename(meshLoadConfig.meshPath);
    CreateTempTexture(*newMesh, 255);
}

}    // namespace XRLib
