#include "VkStandardRB.h"

namespace XRLib {
namespace Graphics {

VkStandardRB::VkStandardRB(VkCore& core, Scene& scene, std::vector<std::unique_ptr<IGraphicsRenderpass>>* renderPasses,
                           bool stereo)
    : core{core}, StandardRB{scene, renderPasses, stereo} {}

const std::string_view VkStandardRB::defaultVertFlat = R"(
    #version 450
    #extension GL_ARB_separate_shader_objects : enable
    layout(set = 0, binding = 0) uniform ViewProj{
        mat4 view;
        mat4 proj;
    } vp;

    layout(set = 0, binding = 1) readonly buffer ModelMatrices {
        mat4 models[];
    };

    layout(push_constant) uniform PushConstants {
        uint modelIndex;
    };

    layout(location = 0) in vec3 inPosition;
    layout(location = 1) in vec3 inNormal;
    layout(location = 2) in vec2 inTexCoord;

    layout(location = 0) out vec3 fragNormal;
    layout(location = 1) out vec2 fragTexCoord;
    layout(location = 2) out vec3 fragWorldPos;
    layout(location = 3) out vec3 cameraPos;

    void main() {
        vec4 worldPos = models[modelIndex] * vec4(inPosition, 1.0);
        gl_Position = vp.proj * vp.view * worldPos;
        mat3 normalMatrix = transpose(inverse(mat3(models[modelIndex])));
        fragNormal = normalize(normalMatrix * inNormal);
        fragTexCoord = inTexCoord;
        fragWorldPos = worldPos.xyz;
        cameraPos = -vec3(vp.view[3]);
    }
)";

const std::string_view VkStandardRB::defaultVertStereo = R"(
    #version 450
    #extension GL_EXT_multiview : enable
    #extension GL_ARB_separate_shader_objects : enable

    layout(set = 0,binding = 0) uniform ViewProj{
        mat4 view[2];
        mat4 proj[2];
    } vp;

    layout(set = 0, binding = 1) readonly buffer ModelMatrices {
        mat4 models[];
    };

    layout(push_constant) uniform PushConstants {
        uint modelIndex;
    };

    layout(location = 0) in vec3 inPosition;
    layout(location = 1) in vec3 inNormal;
    layout(location = 2) in vec2 inTexCoord;

    layout(location = 0) out vec3 fragNormal;
    layout(location = 1) out vec2 fragTexCoord;
    layout(location= 2) out vec3 fragWorldPos;
    layout(location = 3) out vec3 cameraPos;

    void main() {
        vec4 worldPos = models[modelIndex] * vec4(inPosition, 1.0);
        gl_Position = vp.proj[gl_ViewIndex] * vp.view[gl_ViewIndex] * worldPos;
        mat3 normalMatrix = transpose(inverse(mat3(models[modelIndex])));
        fragNormal = normalize(normalMatrix * inNormal);
        fragTexCoord = inTexCoord;
        fragWorldPos = worldPos.xyz;
        cameraPos = -vec3(vp.view[gl_ViewIndex][3]);
    }
)";

const std::string_view VkStandardRB::defaultPhongFrag = R"(
    #version 450
    #extension GL_ARB_separate_shader_objects : enable
    #extension GL_EXT_multiview : enable
    #extension GL_EXT_nonuniform_qualifier: enable

    struct Light {
        mat4 transform;
        vec4 color;
        float intensity;
    };

    layout(set = 0, binding = 2) uniform sampler2D diffuseSamplers[];
    layout(set = 0, binding = 3) uniform sampler2D normalSamplers[];

    layout(set = 1, binding = 0) uniform LightsCount{
        int lightsCount;
    };
    layout(set = 1, binding = 1) readonly buffer Lights{
        Light lights[];
    };

    layout(push_constant) uniform PushConstants {
        uint modelIndex;
    };

    layout(location = 0) in vec3 fragNormal;
    layout(location = 1) in vec2 fragTexCoord;
    layout(location = 2) in vec3 fragWorldPos;
    layout(location = 3) in vec3 cameraPos;

    layout(location = 0) out vec4 outColor;

    vec3 calculatePhongLighting(vec3 normal, vec3 viewDir, vec3 lightDir, vec3 lightColor, float lightIntensity, vec3 diffuseColor) {
        // Ambient
        float ambientStrength = 0.1;
        vec3 ambient = ambientStrength * lightColor;

        // Diffuse
        float diff = max(dot(normal, lightDir), 0.0);
        vec3 diffuse = diff * lightColor;

        // Specular
        float specularStrength = 0.5;
        vec3 reflectDir = reflect(-lightDir, normal);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
        vec3 specular = specularStrength * spec * lightColor;

        return (ambient + diffuse + specular) * diffuseColor * lightIntensity;
    }
    void main() {
        vec3 normal = normalize(fragNormal);
        vec3 viewDir = normalize(cameraPos - fragWorldPos);

        vec4 texColor = texture(diffuseSamplers[modelIndex], fragTexCoord);
        vec3 result = vec3(0.0);

        for (int i = 0; i < lightsCount; i++) {
            vec3 lightPos = lights[i].transform[3].xyz;
            vec3 lightDir = normalize(lightPos - fragWorldPos);
            vec3 lightColor = lights[i].color.rgb;
            float lightIntensity = lights[i].intensity;
            
            float distance = length(lightPos - fragWorldPos);
            float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * (distance * distance));
            result += calculatePhongLighting(normal, viewDir, lightDir, lightColor, lightIntensity, texColor.rgb) * attenuation;
        }

        result = pow(result, vec3(1.0/2.2));
        outColor = vec4(result, texColor.a);
    }
)";

// modified from: https://learnopengl.com/PBR/Lighting
const std::string_view VkStandardRB::defaultPBRFrag = R"(
    #version 450
    #extension GL_ARB_separate_shader_objects : enable
    #extension GL_EXT_multiview : enable
    #extension GL_EXT_nonuniform_qualifier: enable

    struct Light {
        mat4 transform;
        vec4 color;
        float intensity;
    };

    layout(set = 0, binding = 2) uniform sampler2D diffuseSamplers[];
    layout(set = 0, binding = 3) uniform sampler2D normalSamplers[];
    layout(set = 0, binding = 4) uniform sampler2D metallicRoughnessSampler[];
    layout(set = 0, binding = 5) uniform sampler2D emissiveSamplers[];

    layout(set = 1, binding = 0) uniform LightsCount {
        int lightsCount;
    };

    layout(set = 1, binding = 1) readonly buffer Lights {
        Light lights[];
    };

    layout(push_constant) uniform PushConstants {
        uint modelIndex;
    };

    layout(location = 0) in vec3 fragNormal;
    layout(location = 1) in vec2 fragTexCoord;
    layout(location = 2) in vec3 fragWorldPos;
    layout(location = 3) in vec3 cameraPos;

    layout(location = 0) out vec4 outColor;

    const float PI = 3.14159265359;

    float DistributionGGX(vec3 N, vec3 H, float roughness) {
        float a = roughness * roughness;
        float a2 = a * a;
        float NdotH = max(dot(N, H), 0.0);
        float NdotH2 = NdotH * NdotH;
        float denom = (NdotH2 * (a2 - 1.0) + 1.0);
        return a2 / (PI * denom * denom);
    }

    float GeometrySchlickGGX(float NdotV, float roughness) {
        float r = roughness + 1.0;
        float k = (r * r) / 8.0;
        return NdotV / (NdotV * (1.0 - k) + k);
    }

    float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
        float NdotV = max(dot(N, V), 0.0);
        float NdotL = max(dot(N, L), 0.0);
        float ggx1 = GeometrySchlickGGX(NdotV, roughness);
        float ggx2 = GeometrySchlickGGX(NdotL, roughness);
        return ggx1 * ggx2;
    }

    vec3 FresnelSchlick(float cosTheta, vec3 F0) {
        return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
    }

    void main() {
        int index = int(modelIndex);
        vec3 albedo = texture(diffuseSamplers[index], fragTexCoord).rgb;
        float ao = texture(metallicRoughnessSampler[index], fragTexCoord).r;
        float metallic = texture(metallicRoughnessSampler[index], fragTexCoord).b;
        float roughness = texture(metallicRoughnessSampler[index], fragTexCoord).g;
        vec3 emissive = texture(emissiveSamplers[index], fragTexCoord).rgb;
        vec3 normalMapSample = texture(normalSamplers[index], fragTexCoord).rgb;
        vec3 N = normalize(fragNormal); // TODO: normal map with TBN
        vec3 V = normalize(cameraPos - fragWorldPos);
        vec3 F0 = mix(vec3(0.04), albedo, metallic);
        vec3 Lo = vec3(0.0);

        for (int i = 0; i < lightsCount; ++i) {
            vec3 lightPos = vec3(lights[i].transform[3]);
            vec3 L = normalize(lightPos - fragWorldPos);
            vec3 H = normalize(V + L);
            float distance = length(lightPos - fragWorldPos);
            float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * (distance * distance));
            vec3 radiance = lights[i].color.rgb * lights[i].intensity * attenuation;
            float NDF = DistributionGGX(N, H, roughness);
            float G = GeometrySmith(N, V, L, roughness);
            vec3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);
            vec3 numerator = NDF * G * F;
            float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001;
            vec3 specular = numerator / denominator;
            vec3 kS = F;
            vec3 kD = vec3(1.0) - kS;
            kD *= 1.0 - metallic;
            float NdotL = max(dot(N, L), 0.0);
            Lo += (kD * albedo / PI + specular) * radiance * NdotL;
        }

        vec3 ambient = vec3(0.03) * albedo * ao;
        vec3 color = ambient + Lo + emissive;
        color = color / (color + vec3(1.0));
        color = pow(color, vec3(1.0/2.2));
        outColor = vec4(color, 1.0);
    }
)";

////////////////////////////////////////////////////
/// Default Buffers creation
////////////////////////////////////////////////////
std::shared_ptr<Buffer> CreateModelPositionBuffer(VkCore& core, Scene& scene) {
    std::vector<glm::mat4> modelPositions(scene.Meshes().size());
    for (int i = 0; i < modelPositions.size(); ++i) {
        modelPositions[i] = scene.Meshes()[i]->GetGlobalTransform().GetMatrix();
    }

    if (modelPositions.empty()) {
        Transform tempTransform;
        modelPositions.push_back(tempTransform.GetMatrix());
    }

    auto modelPositionsBuffer =
        std::make_shared<Buffer>(core, sizeof(glm::mat4) * modelPositions.size(),
                                 VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                 static_cast<void*>(modelPositions.data()), false);

    EventSystem::Callback<> modelPositionBufferCallback = [&scene, &buffer = *modelPositionsBuffer]() {
        std::vector<glm::mat4> modelPositions(scene.Meshes().size());
        for (int i = 0; i < modelPositions.size(); ++i) {
            modelPositions[i] = scene.Meshes()[i]->GetGlobalTransform().GetMatrix();
        }
        buffer.UpdateBuffer(sizeof(glm::mat4) * modelPositions.size(), static_cast<void*>(modelPositions.data()));
    };

    EventSystem::RegisterListener(Events::XRLIB_EVENT_APPLICATION_PRE_RENDERING, modelPositionBufferCallback);
    return modelPositionsBuffer;
}

std::shared_ptr<Buffer> CreateViewProjectionBuffer(VkCore& core, Primitives::ViewProjectionStereo& viewProj) {
    auto viewProjBuffer = std::make_shared<Buffer>(
        core, sizeof(viewProj), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        static_cast<void*>(&viewProj), false);

    EventSystem::Callback<std::vector<glm::mat4>, std::vector<glm::mat4>> bufferCamUpdateCallback =
        [&buffer = *viewProjBuffer, &viewProj](std::vector<glm::mat4> views, std::vector<glm::mat4> projs) {
            if (views.size() != 2 || projs.size() != 2) {
                Util::ErrorPopup("Unknown view size, please use custom shader");
                return;
            }

            for (int i = 0; i < 2; ++i) {
                viewProj.views[i] = views[i];
                viewProj.projs[i] = projs[i];
            }

            buffer.UpdateBuffer(sizeof(Primitives::ViewProjectionStereo), static_cast<void*>(&viewProj));
        };

    EventSystem::RegisterListener(Events::XRLIB_EVENT_HEAD_MOVEMENT, bufferCamUpdateCallback);
    return viewProjBuffer;
}

std::shared_ptr<Buffer> CreateViewProjectionBuffer(VkCore& core, Scene& scene, Primitives::ViewProjection& viewProj) {
    auto viewProjBuffer = std::make_shared<Buffer>(
        core, sizeof(viewProj), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        static_cast<void*>(&viewProj), false);
    EventSystem::Callback<int> bufferOnKeyShouldUpdateCallback = [&scene, &viewProj,
                                                                  &buffer1 = *viewProjBuffer](int keyCode) {
        viewProj.view = scene.MainCamera()->CameraView();
        buffer1.UpdateBuffer(sizeof(Primitives::ViewProjection), static_cast<void*>(&viewProj));
    };

    EventSystem::RegisterListener(Events::XRLIB_EVENT_KEY_PRESSED, bufferOnKeyShouldUpdateCallback);

    EventSystem::Callback<double, double> bufferOnMouseShouldUpdateCallback =
        [&scene, &viewProj, &buffer2 = *viewProjBuffer](double deltaX, double deltaY) {
            viewProj.view = scene.MainCamera()->CameraView();
            buffer2.UpdateBuffer(sizeof(Primitives::ViewProjection), static_cast<void*>(&viewProj));
        };

    EventSystem::RegisterListener(Events::XRLIB_EVENT_MOUSE_RIGHT_MOVEMENT_EVENT, bufferOnMouseShouldUpdateCallback);
    return viewProjBuffer;
}

std::vector<std::shared_ptr<Image>>
CreateTextures(VkCore& core, Scene& scene, const std::function<const Mesh::TextureData&(const Mesh&)>& getTexture) {
    std::vector<std::shared_ptr<Image>> textures(scene.Meshes().size());

    for (size_t i = 0; i < textures.size(); ++i) {
        const auto& mesh = *scene.Meshes()[i];
        const auto& texture = getTexture(mesh);

        textures[i] = std::make_shared<Image>(core, texture.textureData, texture.textureWidth, texture.textureHeight,
                                              texture.textureChannels, VK_FORMAT_R8G8B8A8_SRGB);
    }

    return textures;
}

std::pair<std::shared_ptr<Buffer>, std::shared_ptr<Buffer>> CreateLightBuffer(VkCore& core, Scene& scene) {
    VkBufferUsageFlags usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    struct PointLightData {
        Transform transform;
        glm::vec4 color;
        float intensity;
    };

    std::vector<PointLightData> pointLightDataBuffer(scene.PointLights().size());
    for (size_t i = 0; i < pointLightDataBuffer.size(); ++i) {
        pointLightDataBuffer[i].transform = scene.PointLights()[i]->GetGlobalTransform();
        pointLightDataBuffer[i].color = scene.PointLights()[i]->GetColor();
        pointLightDataBuffer[i].intensity = scene.PointLights()[i]->GetIntensity();
    }

    auto lightsBuffer = std::make_unique<Buffer>(core, sizeof(PointLightData) * scene.PointLights().size(), usage,
                                                 static_cast<void*>(pointLightDataBuffer.data()), false);
    usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    int lightsCount = scene.PointLights().size();
    auto lightsCountBuffer =
        std::make_shared<Buffer>(core, sizeof(int), usage, static_cast<void*>(&lightsCount), false);

    return {std::move(lightsCountBuffer), std::move(lightsBuffer)};
}

////////////////////////////////////////////////////
// Default DescriptorLayout and Renderpasses binding
////////////////////////////////////////////////////
void VkStandardRB::PrepareDefaultRenderPasses(std::vector<std::vector<Image*>>& swapchainImages,
                                              std::shared_ptr<Buffer> viewProjBuffer) {
    auto modelPositionsBuffer = std::move(CreateModelPositionBuffer(core, scene));

    auto diffuseTextures = std::move(
        CreateTextures(core, scene, [](const Mesh& mesh) -> const Mesh::TextureData& { return mesh.Diffuse; }));
    auto normalTextures = std::move(
        CreateTextures(core, scene, [](const Mesh& mesh) -> const Mesh::TextureData& { return mesh.Normal; }));
    auto metallicRoughness = std::move(CreateTextures(
        core, scene, [](const Mesh& mesh) -> const Mesh::TextureData& { return mesh.MetallicRoughness; }));
    auto emissiveTextures = std::move(
        CreateTextures(core, scene, [](const Mesh& mesh) -> const Mesh::TextureData& { return mesh.Emissive; }));

    auto [lightsCountBuffer, lightsBuffer] = std::move(CreateLightBuffer(core, scene));

    std::vector<std::unique_ptr<DescriptorSet>> descriptorSets;
    auto descriptorSet = std::make_unique<DescriptorSet>(core, viewProjBuffer, modelPositionsBuffer, diffuseTextures,
                                                         normalTextures, metallicRoughness, emissiveTextures);
    descriptorSet->AllocatePushConstant(sizeof(uint32_t));
    descriptorSets.push_back(std::move(descriptorSet));

    auto descriptorSet2 = std::make_unique<DescriptorSet>(core, lightsCountBuffer, lightsBuffer);
    descriptorSet2->AllocatePushConstant(sizeof(uint32_t));
    descriptorSets.push_back(std::move(descriptorSet2));

    std::unique_ptr<IGraphicsRenderpass> graphicsRenderPass =
        std::make_unique<VkGraphicsRenderpass>(core, stereo, swapchainImages, true, std::move(descriptorSets));
    renderPasses->push_back(std::move(graphicsRenderPass));
}

void VkStandardRB::InitVerticesIndicesBuffers() {
    if (scene.Meshes().empty()) {
        return;
    }
    // init vertex buffer and index buffer
    vertexBuffers.resize(scene.Meshes().size());
    indexBuffers.resize(scene.Meshes().size());
    for (int i = 0; i < scene.Meshes().size(); ++i) {
        auto& mesh = *scene.Meshes()[i];
        if (mesh.GetVerticies().empty() || mesh.GetIndices().empty()) {
            vertexBuffers[i] = nullptr;
            indexBuffers[i] = nullptr;
            continue;
        }

        void* verticesData = static_cast<void*>(mesh.GetVerticies().data());
        void* indicesData = static_cast<void*>(mesh.GetIndices().data());
        vertexBuffers[i] =
            std::make_unique<Buffer>(core, sizeof(mesh.GetVerticies()[0]) * mesh.GetVerticies().size(),
                                     VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, verticesData,
                                     true, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        indexBuffers[i] = std::make_unique<Buffer>(core, sizeof(mesh.GetIndices()[0]) * mesh.GetIndices().size(),
                                                   VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                                                   indicesData, true, VK_MEMORY_HEAP_DEVICE_LOCAL_BIT);
    }
}

void VkStandardRB::Prepare() {
    if (stereo) {
        PrepareDefaultRenderPasses(swapchain->GetSwapchainImages(),
                                   std::move(CreateViewProjectionBuffer(core, viewProjStereo)));
    } else {
        auto mainCamera = scene.MainCamera();
        viewProj.view = mainCamera->CameraView();
        viewProj.proj = mainCamera->CameraProjection();
        PrepareDefaultRenderPasses(swapchain->GetSwapchainImages(),
                                   std::move(CreateViewProjectionBuffer(core, scene, viewProj)));
    }
}

////////////////////////////////////////////////////
// Render Loop
////////////////////////////////////////////////////
bool VkStandardRB::StartFrame(uint32_t& imageIndex) {
    vkWaitForFences(core.GetRenderDevice(), 1, &core.GetInFlightFence(), VK_TRUE, UINT64_MAX);

    auto result = vkAcquireNextImageKHR(core.GetRenderDevice(), swapchain->GetSwaphcain(), UINT64_MAX,
                                        core.GetImageAvailableSemaphore(), VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        auto [width, height] = WindowHandler::GetFrameBufferSize();
        EventSystem::TriggerEvent(Events::XRLIB_EVENT_WINDOW_RESIZED, width, height);
        return false;
    } else if (result != VK_SUCCESS) {
        Util::ErrorPopup("Failed to acquire next image");
    }

    return true;
}

void VkStandardRB::RecordFrame(uint32_t& imageIndex) {
    vkResetFences(core.GetRenderDevice(), 1, &core.GetInFlightFence());
    CommandBuffer commandBuffer{core};
    vkResetCommandBuffer(commandBuffer.GetCommandBuffer(), 0);

    // default frame recording
    commandBuffer.StartRecord();
    for (int i = 0; i < renderPasses->size(); ++i) {
        auto currentPass = static_cast<VkGraphicsRenderpass*>(renderPasses->at(i).get());
        RecordPass(commandBuffer, currentPass, i, imageIndex);

        // add barrier synchronization between render passes
        if (i != renderPasses->size() - 1) {
            commandBuffer.BarrierBetweenPasses(imageIndex, *currentPass);
        }
    }

    if (!stereo)
        commandBuffer.EndRecord({core.GetImageAvailableSemaphore()}, {core.GetRenderFinishedSemaphore()},
                                core.GetInFlightFence());
    else {
        commandBuffer.EndRecord({}, {}, core.GetInFlightFence());
    }
}
void VkStandardRB::RecordPass(CommandBuffer& commandBuffer, VkGraphicsRenderpass* currentPass, uint8_t currentPassIndex,
                              uint32_t& imageIndex) {
    commandBuffer.StartPass(*currentPass, imageIndex).BindDescriptorSets(*currentPass, 0);
    for (uint32_t i = 0; i < scene.Meshes().size(); ++i) {
        commandBuffer.PushConstant(*currentPass, sizeof(uint32_t), &i);
        if (!vertexBuffers.empty() && !indexBuffers.empty() && vertexBuffers[i] != nullptr &&
            indexBuffers[i] != nullptr) {
            commandBuffer.BindVertexBuffer(0, {vertexBuffers[i]->GetBuffer()}, {0})
                .BindIndexBuffer(indexBuffers[i]->GetBuffer(), 0);
        }

        commandBuffer.DrawIndexed(scene.Meshes()[i]->GetIndices().size(), 1, 0, 0, 0);
    }

    // represents how many passes left to draw
    EventSystem::TriggerEvent<int, CommandBuffer&>(Events::XRLIB_EVENT_RENDERER_PRE_SUBMITTING,
                                                   (renderPasses->size() - 1) - currentPassIndex, commandBuffer);

    commandBuffer.EndPass();
}

void VkStandardRB::EndFrame(uint32_t& imageIndex) {
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &core.GetRenderFinishedSemaphore();

    VkSwapchainKHR swapChains[] = {swapchain->GetSwaphcain()};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;

    vkQueuePresentKHR(core.GetGraphicsQueue(), &presentInfo);
}

}    // namespace Graphics
}    // namespace XRLib
