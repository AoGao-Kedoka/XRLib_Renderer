#include "VkStandardRB.h"

namespace XRLib {
namespace Graphics {

VkStandardRB::VkStandardRB(VkCore& core, Scene& scene) : core{core}, scene{scene} {}

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
        fragNormal = inNormal;
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
        fragNormal = inNormal;
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

    layout(set = 0, binding = 2) uniform sampler2D texSamplers[];

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

        vec4 texColor = texture(texSamplers[modelIndex], fragTexCoord);
        vec3 result = vec3(0.0);

        for (int i = 0; i < lightsCount; i++) {
            vec3 lightPos = lights[i].transform[3].xyz;
            vec3 lightDir = normalize(lightPos - fragWorldPos);
            vec3 lightColor = lights[i].color.rgb;
            float lightIntensity = lights[i].intensity;

            result += calculatePhongLighting(normal, viewDir, lightDir, lightColor, lightIntensity, texColor.rgb);
        }

        outColor = vec4(result, texColor.a);
    }
)";

////////////////////////////////////////////////////
/// Default Buffers creation
////////////////////////////////////////////////////
std::shared_ptr<Buffer> CreateModelPositionBuffer(VkCore& core, Scene& scene) {
    std::vector<glm::mat4> modelPositions(scene.Meshes().size());
    for (int i = 0; i < modelPositions.size(); ++i) {
        modelPositions[i] = scene.Meshes()[i]->GetTransform().GetMatrix();
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
            modelPositions[i] = scene.Meshes()[i]->GetTransform().GetMatrix();
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
        viewProj.view = scene.CameraTransform().GetMatrix();
        buffer1.UpdateBuffer(sizeof(Primitives::ViewProjection), static_cast<void*>(&viewProj));
    };

    EventSystem::RegisterListener(Events::XRLIB_EVENT_KEY_PRESSED, bufferOnKeyShouldUpdateCallback);

    EventSystem::Callback<double, double> bufferOnMouseShouldUpdateCallback =
        [&scene, &viewProj, &buffer2 = *viewProjBuffer](double deltaX, double deltaY) {
            viewProj.view = scene.CameraTransform().GetMatrix();
            buffer2.UpdateBuffer(sizeof(Primitives::ViewProjection), static_cast<void*>(&viewProj));
        };

    EventSystem::RegisterListener(Events::XRLIB_EVENT_MOUSE_RIGHT_MOVEMENT_EVENT, bufferOnMouseShouldUpdateCallback);
    return viewProjBuffer;
}

std::vector<std::shared_ptr<Image>> CreateTextures(VkCore& core, Scene& scene) {
    std::vector<std::shared_ptr<Image>> textures(scene.Meshes().size());
    if (textures.empty()) {
        std::vector<uint8_t> textureData;
        textureData.resize(1 * 1 * 4, 255);
        textures.push_back(std::make_shared<Image>(core, textureData, 1, 1, 4, VK_FORMAT_R8G8B8A8_SRGB));
        return textures;
    }

    for (int i = 0; i < textures.size(); ++i) {
        textures[i] = std::make_shared<Image>(
            core, scene.Meshes()[i]->GetTextureData().data, scene.Meshes()[i]->GetTextureData().textureWidth,
            scene.Meshes()[i]->GetTextureData().textureHeight, scene.Meshes()[i]->GetTextureData().textureChannels,
            VK_FORMAT_R8G8B8A8_SRGB);
    }

    return textures;
}

std::pair<std::shared_ptr<Buffer>, std::shared_ptr<Buffer>> CreateLightBuffer(VkCore& core, Scene& scene) {
    VkBufferUsageFlags usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    auto lightsBuffer = std::make_unique<Buffer>(core, sizeof(Scene::Light) * scene.Lights().size() + sizeof(int),
                                                 usage, static_cast<void*>(scene.Lights().data()), false);
    usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    int lightsCount = scene.Lights().size();
    auto lightsCountBuffer =
        std::make_shared<Buffer>(core, sizeof(int), usage, static_cast<void*>(&lightsCount), false);

    return {std::move(lightsCountBuffer), std::move(lightsBuffer)};
}

////////////////////////////////////////////////////
// Default DescriptorLayout and Renderpasses binding
////////////////////////////////////////////////////
void VkStandardRB::PrepareDefaultRenderPasses(std::vector<std::vector<std::unique_ptr<Image>>>& swapchainImages,
                                              bool isStereo, std::shared_ptr<Buffer> viewProjBuffer) {
    auto modelPositionsBuffer = std::move(CreateModelPositionBuffer(core, scene));
    auto textures = std::move(CreateTextures(core, scene));
    auto [lightsCountBuffer, lightsBuffer] = std::move(CreateLightBuffer(core, scene));

    std::vector<std::unique_ptr<DescriptorSet>> descriptorSets;
    auto descriptorSet = std::make_unique<DescriptorSet>(core, viewProjBuffer, modelPositionsBuffer, textures);
    descriptorSet->AllocatePushConstant(sizeof(uint32_t));
    descriptorSets.push_back(std::move(descriptorSet));

    auto descriptorSet2 = std::make_unique<DescriptorSet>(core, lightsCountBuffer, lightsBuffer);
    descriptorSet2->AllocatePushConstant(sizeof(uint32_t));
    descriptorSets.push_back(std::move(descriptorSet2));

    std::unique_ptr<IGraphicsRenderpass> graphicsRenderPass =
        std::make_unique<VkGraphicsRenderpass>(core, isStereo, swapchainImages, std::move(descriptorSets));
    renderPasses.push_back(std::move(graphicsRenderPass));
}

void VkStandardRB::InitVerticesIndicesShader() {
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

void VkStandardRB::PrepareDefaultStereoRenderPasses(Primitives::ViewProjectionStereo& viewProj,
                                                    std::vector<std::unique_ptr<IGraphicsRenderpass>>& renderPasses) {
    PrepareDefaultRenderPasses(swapchain->GetSwapchainImages(), true,
                               std::move(CreateViewProjectionBuffer(core, viewProj)));
}

void VkStandardRB::PrepareDefaultFlatRenderPasses(Primitives::ViewProjection& viewProj,
                                                  std::vector<std::unique_ptr<IGraphicsRenderpass>>& renderPasses) {
    viewProj.view = scene.CameraTransform().GetMatrix();
    viewProj.proj = scene.CameraProjection();
    PrepareDefaultRenderPasses(swapchain->GetSwapchainImages(), false,
                               std::move(CreateViewProjectionBuffer(core, scene, viewProj)));
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
    auto currentPassIndex = 0;
    auto& currentPass = static_cast<VkGraphicsRenderpass&>(*renderPasses[currentPassIndex]);
    commandBuffer.StartRecord().StartPass(currentPass, imageIndex).BindDescriptorSets(currentPass, 0);
    for (uint32_t i = 0; i < scene.Meshes().size(); ++i) {
        commandBuffer.PushConstant(currentPass, sizeof(uint32_t), &i);
        if (!vertexBuffers.empty() && !indexBuffers.empty() && vertexBuffers[i] != nullptr &&
            indexBuffers[i] != nullptr) {
            commandBuffer.BindVertexBuffer(0, {vertexBuffers[i]->GetBuffer()}, {0})
                .BindIndexBuffer(indexBuffers[i]->GetBuffer(), 0);
        }

        commandBuffer.DrawIndexed(scene.Meshes()[i]->GetIndices().size(), 1, 0, 0, 0);
    }

    // represents how many passes left to draw
    EventSystem::TriggerEvent<int, CommandBuffer&>(Events::XRLIB_EVENT_RENDERER_PRE_SUBMITTING,
                                                   (renderPasses.size() - 1) - currentPassIndex, commandBuffer);

    commandBuffer.EndPass();

    // add barrier synchronization between render passes
    if (currentPassIndex != renderPasses.size() - 1) {
        commandBuffer.BarrierBetweenPasses(imageIndex, currentPass);
    }

    commandBuffer.EndRecord();
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
