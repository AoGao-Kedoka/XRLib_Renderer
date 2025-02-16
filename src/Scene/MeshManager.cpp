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

glm::mat4 ConvertMatrixToGLM(const aiMatrix4x4& from) {
    return glm::mat4(from.a1, from.b1, from.c1, from.d1, from.a2, from.b2, from.c2, from.d2, from.a3, from.b3, from.c3,
                     from.d3, from.a4, from.b4, from.c4, from.d4);
}

void MeshManager::WaitForAllMeshesToLoad() {
    if (meshQueue.empty()) {
        loadingStatusCounter = -1;
        loadingRegistrationCounter = -1;
        return;
    }
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        cv.wait(lock, [this] { return loadingStatusCounter == loadingRegistrationCounter; });

        loadingStatusCounter = -1;
        loadingRegistrationCounter = -1;
        cv.notify_all();
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
    textureData.textureData.resize(textureData.textureChannels * textureData.textureHeight * textureData.textureWidth,
                                   color);
    newMesh.Diffuse = textureData;
}

void MeshManager::LoadMesh(const Mesh::MeshLoadConfig& meshLoadConfig, Entity*& bindPtr) {
    LOGGER(LOGGER::INFO) << "Loading: " << meshLoadConfig.meshPath;
    Assimp::Importer importer;

    const aiScene* scene =
        importer.ReadFile(meshLoadConfig.meshPath, aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_FlipUVs);

    auto meshPathInValid = [&]() -> bool {
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

    if (meshPathInValid()) {
        LOGGER(LOGGER::ERR) << importer.GetErrorString();
        auto newMesh = createMeshPlaceHolder();
        HandleInvalidMesh(meshLoadConfig, newMesh);
        IncreaseLoadingStatusCounter(loadingStatusCounter, queueMutex, cv);
        return;
    }

    if (scene->mNumMeshes > 0) {
        auto entityParent = std::make_unique<Entity>(Util::GetFileNameWithoutExtension(meshLoadConfig.meshPath));
        bindPtr = entityParent.get();

        std::vector<std::future<void>> loadFutures;
        ProcessNode(scene->mRootNode, scene, meshLoadConfig, entityParent.get(), loadFutures);
        for (auto& future : loadFutures) {
            future.wait();
        }

        entityParent->GetLocalTransform() =
            meshLoadConfig.transform.GetMatrix() * entityParent->GetLocalTransform().GetMatrix();
        Entity::AddEntity(entityParent, hiearchyRoot);
    }

    IncreaseLoadingStatusCounter(loadingStatusCounter, queueMutex, cv);

    if (loadingStatusCounter == loadingRegistrationCounter) {
        EventSystem::TriggerEvent(Events::XRLIB_EVENT_MESHES_LOADING_FINISHED);
    }
}
void MeshManager::ProcessNode(aiNode* node, const aiScene* scene, const Mesh::MeshLoadConfig& meshLoadConfig,
                              Entity* parent, std::vector<std::future<void>>& loadFutures) {
    parent->GetLocalTransform() = ConvertMatrixToGLM(node->mTransformation);

    // handles meshes
    for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        loadFutures.push_back(
            std::async(std::launch::async, &MeshManager::ProcessMesh, this, mesh, scene, meshLoadConfig, parent));
    }

    // handles node, transfer to an entity
    for (unsigned int i = 0; i < node->mNumChildren; ++i) {
        auto entity = std::make_unique<Entity>(Util::GetFileNameWithoutExtension(meshLoadConfig.meshPath));
        ProcessNode(node->mChildren[i], scene, meshLoadConfig, entity.get(), loadFutures);
        Entity::AddEntity(entity, parent);
    }
}
void MeshManager::ProcessMesh(aiMesh* aiMesh, const aiScene* scene, const Mesh::MeshLoadConfig& meshLoadConfig,
                              Entity* parent) {
    auto mesh = std::make_unique<Mesh>();
    LoadMeshVerticesIndices(meshLoadConfig, mesh.get(), aiMesh);
    LoadMeshTextures(meshLoadConfig, mesh.get(), aiMesh, scene);
    mesh->Rename(aiMesh->mName.C_Str());
    Entity::AddEntity(mesh, parent, &meshes);
    LOGGER(LOGGER::DEBUG) << "Loaded mesh: " << aiMesh->mName.C_Str();
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
    LoadEmbeddedTextures(meshLoadConfig, newMesh, aiMesh, scene);

    // get meshloadinfo specified texture
    LoadSpecifiedTextures(newMesh->Diffuse, meshLoadConfig.diffuseTexturePath);
    LoadSpecifiedTextures(newMesh->Normal, meshLoadConfig.normalTexturePath);
    LoadSpecifiedTextures(newMesh->MetallicRoughness, meshLoadConfig.metallicRoughnessTexturePath);
    LoadSpecifiedTextures(newMesh->Emissive, meshLoadConfig.emissiveTexturePath);

    // last fallback, create temporary white texture
    if (newMesh->Diffuse.textureData.empty()) {
        CreateTempTexture(*newMesh, 255);
    }
}

void MeshManager::LoadEmbeddedTextures(const Mesh::MeshLoadConfig& meshLoadConfig, Mesh* newMesh, aiMesh* aiMesh,
                                       const aiScene* scene) {
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
                stbi_load_from_memory(reinterpret_cast<const unsigned char*>(texture->pcData), texture->mWidth, &width,
                                      &height, &channels, STBI_rgb_alpha);
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
            newMesh->Diffuse.textureData.clear();
            loadTextureFromEmbedding(embeddedTexture, newMesh->Diffuse);
        } else {
            auto path = std::filesystem::path(meshLoadConfig.meshPath).parent_path() /
                        std::filesystem::path(texturePath.C_Str());
            if (std::filesystem::is_regular_file(path)) {
                newMesh->Diffuse.textureData.clear();
                LoadSpecifiedTextures(newMesh->Diffuse, path.generic_string());
            }
        }
    }

    // Normal Map
    if (material->GetTexture(aiTextureType_NORMALS, 0, &texturePath) == AI_SUCCESS ||
        material->GetTexture(aiTextureType_HEIGHT, 0, &texturePath) == AI_SUCCESS) {
        const aiTexture* embeddedTexture = scene->GetEmbeddedTexture(texturePath.C_Str());
        if (embeddedTexture) {
            newMesh->Normal.textureData.clear();
            loadTextureFromEmbedding(embeddedTexture, newMesh->Normal);
        } else {
            auto path = std::filesystem::path(meshLoadConfig.meshPath).parent_path() /
                        std::filesystem::path(texturePath.C_Str());
            if (std::filesystem::is_regular_file(path)) {
                newMesh->Normal.textureData.clear();
                LoadSpecifiedTextures(newMesh->Normal, path.generic_string());
            }
        }
    }

    // Emissive
    if (material->GetTexture(aiTextureType_EMISSIVE, 0, &texturePath) == AI_SUCCESS) {
        const aiTexture* embeddedTexture = scene->GetEmbeddedTexture(texturePath.C_Str());
        if (embeddedTexture) {
            newMesh->Emissive.textureData.clear();
            loadTextureFromEmbedding(embeddedTexture, newMesh->Emissive);
        } else {
            auto path = std::filesystem::path(meshLoadConfig.meshPath).parent_path() /
                        std::filesystem::path(texturePath.C_Str());
            if (std::filesystem::is_regular_file(path)) {
                newMesh->Emissive.textureData.clear();
                LoadSpecifiedTextures(newMesh->Emissive, path.generic_string());
            }
        }
    }

    // Metallic and roughness
    if (material->GetTexture(aiTextureType_UNKNOWN, 0, &texturePath) == AI_SUCCESS) {
        const aiTexture* embeddedTexture = scene->GetEmbeddedTexture(texturePath.C_Str());
        if (embeddedTexture) {
            newMesh->MetallicRoughness.textureData.clear();
            loadTextureFromEmbedding(embeddedTexture, newMesh->MetallicRoughness);
        } else {
            auto path = std::filesystem::path(meshLoadConfig.meshPath).parent_path() /
                        std::filesystem::path(texturePath.C_Str());
            if (std::filesystem::is_regular_file(path)) {
                newMesh->MetallicRoughness.textureData.clear();
                LoadSpecifiedTextures(newMesh->MetallicRoughness, path.generic_string());
            }
        }
    }
}

void MeshManager::LoadSpecifiedTextures(Mesh::TextureData& texture, const std::string& path) {
    if (path.empty()) {
        return;
    }

    // check fallback texture
    if (texture.textureChannels == 4 && texture.textureWidth == 1 && texture.textureHeight == 1) {
        unsigned char* imageData = stbi_load(path.c_str(), &texture.textureWidth, &texture.textureHeight,
                                             &texture.textureChannels, STBI_rgb_alpha);

        if (imageData) {
            texture.textureData.clear();
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
        lock.unlock();

        std::future<void> future =
            std::async(std::launch::async, &MeshManager::LoadMesh, this, loadConfig, std::ref(entityPtr));

        {
            std::lock_guard<std::mutex> lock(queueMutex);
            futures.push_back(std::move(future));
        }
    }
}

void MeshManager::HandleInvalidMesh(const Mesh::MeshLoadConfig& meshLoadConfig, Mesh* newMesh) {
    Transform transform;
    newMesh->GetLocalTransform() = transform;
    newMesh->Rename(meshLoadConfig.meshPath);
    CreateTempTexture(*newMesh, 255);
}

}    // namespace XRLib
