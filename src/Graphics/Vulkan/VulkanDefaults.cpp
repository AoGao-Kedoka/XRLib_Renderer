#include "VulkanDefaults.h"

namespace XRLib {
namespace Graphics {

const std::string VulkanDefaults::defaultVertFlat = R"(
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

const std::string VulkanDefaults::defaultVertStereo = R"(
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

const std::string VulkanDefaults::defaultPhongFrag = R"(
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

std::shared_ptr<Buffer> CreateModelPositionsBuffer(std::shared_ptr<VkCore> core, std::shared_ptr<Scene> scene) {

    std::vector<glm::mat4> modelPositions(scene->Meshes().size());
    for (int i = 0; i < modelPositions.size(); ++i) {
        modelPositions[i] = scene->Meshes()[i].transform.GetMatrix();
    }

    auto modelPositionsBuffer =
        std::make_shared<Buffer>(core, sizeof(glm::mat4) * modelPositions.size(),
                                 VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                 static_cast<void*>(modelPositions.data()), false);

    EventSystem::Callback<> modelPositionBufferCallback = [scene, modelPositionsBuffer]() {
        std::vector<glm::mat4> modelPositions(scene->Meshes().size());
        for (int i = 0; i < modelPositions.size(); ++i) {
            modelPositions[i] = scene->Meshes()[i].transform.GetMatrix();
        }
        modelPositionsBuffer->UpdateBuffer(sizeof(glm::mat4) * modelPositions.size(),
                                           static_cast<void*>(modelPositions.data()));
    };

    EventSystem::RegisterListener(Events::XRLIB_EVENT_APPLICATION_PRE_RENDERING, modelPositionBufferCallback);

    return modelPositionsBuffer;
}

std::pair<std::shared_ptr<Buffer>, std::shared_ptr<Buffer>> CreateLightBuffer(std::shared_ptr<VkCore> core,
                                                                              std::shared_ptr<Scene> scene) {
    VkBufferUsageFlags usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    auto lightsBuffer = std::make_shared<Buffer>(core, sizeof(Scene::Light) * scene->Lights().size() + sizeof(int),
                                                 usage, static_cast<void*>(scene->Lights().data()), false);
    usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    int lightsCount = scene->Lights().size();
    auto lightsCountBuffer =
        std::make_shared<Buffer>(core, sizeof(int), usage, static_cast<void*>(&lightsCount), false);

    return {lightsCountBuffer, lightsBuffer};
}

std::vector<std::shared_ptr<Image>> CreateTextures(std::shared_ptr<VkCore> core, std::shared_ptr<Scene> scene) {

    std::vector<std::shared_ptr<Image>> textures(scene->Meshes().size());
    for (int i = 0; i < textures.size(); ++i) {
        textures[i] = std::make_shared<Image>(core, scene->Meshes()[i].textureData, scene->Meshes()[i].textureWidth,
                                              scene->Meshes()[i].textureHeight, scene->Meshes()[i].textureChannels,
                                              VK_FORMAT_R8G8B8A8_SRGB);
    }

    return textures;
}

template <typename T>
std::shared_ptr<Buffer> CreateViewProjBuffer(std::shared_ptr<VkCore> core, T& viewProj) {

    return std::make_shared<Buffer>(core, sizeof(T),
                                    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                    static_cast<void*>(&viewProj), false);
}

void RegisterViewProjUpdateCallback(std::shared_ptr<Buffer> viewProjBuffer,
                                    Primitives::ViewProjectionStereo& viewProj) {

    EventSystem::Callback<std::vector<glm::mat4>, std::vector<glm::mat4>> bufferCamUpdateCallback =
        [viewProjBuffer, &viewProj](std::vector<glm::mat4> views, std::vector<glm::mat4> projs) {
            if (views.size() != 2 || projs.size() != 2) {
                Util::ErrorPopup("Unknown view size, please use custom shader");
                return;
            }

            for (int i = 0; i < 2; ++i) {
                viewProj.views[i] = views[i];
                viewProj.projs[i] = projs[i];
            }

            viewProjBuffer->UpdateBuffer(sizeof(Primitives::ViewProjectionStereo), static_cast<void*>(&viewProj));
        };

    EventSystem::RegisterListener(Events::XRLIB_EVENT_HEAD_MOVEMENT, bufferCamUpdateCallback);
}

void PrepareRenderPassesCommon(std::shared_ptr<VkCore> core, std::shared_ptr<Scene> scene,
                               std::vector<std::unique_ptr<GraphicsRenderPass>>& renderPasses,
                               const std::vector<std::vector<DescriptorLayoutElement>>& layoutElementsVec,
                               bool isStereo) {

    std::vector<std::shared_ptr<DescriptorSet>> descriptorSets;
    for (auto& layoutElements : layoutElementsVec) {
        auto descriptorSet = std::make_shared<DescriptorSet>(core, layoutElements);
        descriptorSet->AllocatePushConstant(sizeof(uint32_t));
        descriptorSets.push_back(descriptorSet);
    }

    auto graphicsRenderPass = std::make_unique<GraphicsRenderPass>(core, isStereo, descriptorSets);
    renderPasses.push_back(std::move(graphicsRenderPass));
}

void VulkanDefaults::PrepareDefaultStereoRenderPasses(std::shared_ptr<VkCore> core, std::shared_ptr<Scene> scene,
                                                      Primitives::ViewProjectionStereo& viewProj,
                                                      std::vector<std::unique_ptr<GraphicsRenderPass>>& renderPasses) {

    auto viewProjBuffer = CreateViewProjBuffer(core, viewProj);
    auto modelPositionsBuffer = CreateModelPositionsBuffer(core, scene);
    auto [lightsCountBuffer, lightsBuffer] = CreateLightBuffer(core, scene);
    auto textures = CreateTextures(core, scene);
    RegisterViewProjUpdateCallback(viewProjBuffer, viewProj);

    std::vector<DescriptorLayoutElement> modelLayoutElements{{viewProjBuffer}, {modelPositionsBuffer}, {textures}};
    std::vector<DescriptorLayoutElement> lightsLayoutElements{{lightsCountBuffer}, {lightsBuffer}};
    PrepareRenderPassesCommon(core, scene, renderPasses, {modelLayoutElements, lightsLayoutElements}, true);
}

void VulkanDefaults::PrepareDefaultFlatRenderPasses(std::shared_ptr<VkCore> core, std::shared_ptr<Scene> scene,
                                                    Primitives::ViewProjection& viewProj,
                                                    std::vector<std::unique_ptr<GraphicsRenderPass>>& renderPasses) {

    viewProj.view = scene->CameraTransform().GetMatrix();
    viewProj.proj = scene->CameraProjection();

    auto viewProjBuffer = CreateViewProjBuffer(core, viewProj);
    auto modelPositionsBuffer = CreateModelPositionsBuffer(core, scene);
    auto [lightsCountBuffer, lightsBuffer] = CreateLightBuffer(core, scene);
    auto textures = CreateTextures(core, scene);

    std::vector<DescriptorLayoutElement> modelLayoutElements{{viewProjBuffer}, {modelPositionsBuffer}, {textures}};
    std::vector<DescriptorLayoutElement> lightsLayoutElements{{lightsCountBuffer}, {lightsBuffer}};
    PrepareRenderPassesCommon(core, scene, renderPasses, {modelLayoutElements, lightsLayoutElements}, false);

    EventSystem::Callback<int> bufferOnKeyShouldUpdateCallback = [scene, &viewProj, viewProjBuffer](int keyCode) {
        viewProj.view = scene->CameraTransform().GetMatrix();
        viewProjBuffer->UpdateBuffer(sizeof(Primitives::ViewProjection), static_cast<void*>(&viewProj));
    };

    EventSystem::RegisterListener(Events::XRLIB_EVENT_KEY_PRESSED, bufferOnKeyShouldUpdateCallback);

    EventSystem::Callback<double, double> bufferOnMouseShouldUpdateCallback =
        [scene, &viewProj, viewProjBuffer](double deltaX, double deltaY) {
            viewProj.view = scene->CameraTransform().GetMatrix();
            viewProjBuffer->UpdateBuffer(sizeof(Primitives::ViewProjection), static_cast<void*>(&viewProj));
        };

    EventSystem::RegisterListener(Events::XRLIB_EVENT_MOUSE_RIGHT_MOVEMENT_EVENT, bufferOnMouseShouldUpdateCallback);
}
}    // namespace Graphics
}    // namespace XRLib
