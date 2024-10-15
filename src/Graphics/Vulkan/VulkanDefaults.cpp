#pragma once
#include "VulkanDefaults.h"

namespace XRLib {
namespace Graphics {
std::shared_ptr<Buffer> CreateModelPositionsBuffer(std::shared_ptr<VkCore> core, std::shared_ptr<Scene> scene) {

    std::vector<glm::mat4> modelPositions(scene->Meshes().size());
    for (int i = 0; i < modelPositions.size(); ++i) {
        modelPositions[i] = scene->Meshes()[i].transform.GetMatrix();
    }

    auto modelPositionsBuffer =
        std::make_shared<Buffer>(core, sizeof(glm::mat4) * modelPositions.size(),
                                 VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
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
                                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
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
                               const std::vector<DescriptorLayoutElement>& layoutElements, bool isStereo) {

    auto descriptorSet = std::make_shared<DescriptorSet>(core, layoutElements);
    descriptorSet->AllocatePushConstant(sizeof(uint32_t));

    auto graphicsRenderPass = std::make_unique<GraphicsRenderPass>(core, isStereo, descriptorSet);
    renderPasses.push_back(std::move(graphicsRenderPass));
}

void VulkanDefaults::PrepareDefaultStereoRenderPasses(std::shared_ptr<VkCore> core, std::shared_ptr<Scene> scene,
                                                      Primitives::ViewProjectionStereo& viewProj,
                                                      std::vector<std::unique_ptr<GraphicsRenderPass>>& renderPasses) {

    auto viewProjBuffer = CreateViewProjBuffer(core, viewProj);
    RegisterViewProjUpdateCallback(viewProjBuffer, viewProj);

    auto modelPositionsBuffer = CreateModelPositionsBuffer(core, scene);
    auto textures = CreateTextures(core, scene);

    std::vector<DescriptorLayoutElement> layoutElements{{viewProjBuffer}, {modelPositionsBuffer}, {textures}};
    PrepareRenderPassesCommon(core, scene, renderPasses, layoutElements, true);
}

void VulkanDefaults::PrepareDefaultFlatRenderPasses(std::shared_ptr<VkCore> core, std::shared_ptr<Scene> scene,
                                                    Primitives::ViewProjection& viewProj,
                                                    std::vector<std::unique_ptr<GraphicsRenderPass>>& renderPasses) {

    viewProj.view = scene->CameraTransform().GetMatrix();
    viewProj.proj = scene->CameraProjection();

    auto viewProjBuffer = CreateViewProjBuffer(core, viewProj);
    auto modelPositionsBuffer = CreateModelPositionsBuffer(core, scene);
    auto textures = CreateTextures(core, scene);

    std::vector<DescriptorLayoutElement> layoutElements{{viewProjBuffer}, {modelPositionsBuffer}, {textures}};
    PrepareRenderPassesCommon(core, scene, renderPasses, layoutElements, false);

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
