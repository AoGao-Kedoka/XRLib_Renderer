#include "RenderBackend.h"
#include "Utils/Util.h"

namespace XRLib {
namespace Graphics {
RenderBackend::RenderBackend(std::shared_ptr<Info> info, std::shared_ptr<VkCore> vkCore,
                             std::shared_ptr<XRLib::XR::XrCore> xrCore, std::shared_ptr<XRLib::Scene> scene)
    : info{info}, vkCore{vkCore}, xrCore{xrCore}, scene{scene} {
    // initialize vulkan
    vkCore->CreateVkInstance(*info, xrCore->VkAdditionalInstanceExts());
    if (xrCore->IsXRValid())
        xrCore->VkSetPhysicalDevice(vkCore->GetRenderInstance(), &vkCore->VkPhysicalDeviceRef());
    else
        vkCore->SelectPhysicalDevice();
    vkCore->CreateVkDevice(*info, xrCore->VkAdditionalDeviceExts(), xrCore->IsXRValid());

    EventSystem::TriggerEvent(Events::XRLIB_EVENT_RENDERBACKEND_INIT_FINISHED);


    EventSystem::Callback<> preRenderSynchronizaionCallback = [&vkCore]() {
    };

    EventSystem::RegisterListener(Events::XRLIB_EVENT_APPLICATION_PRE_RENDERING, preRenderSynchronizaionCallback);
}

RenderBackend::~RenderBackend() {}

void RenderBackend::Prepare(std::vector<std::unique_ptr<GraphicsRenderPass>>& passes) {
    GetSwapchainInfo();
    InitVertexIndexBuffers();

    if (passes.empty()) {
        VulkanDefaults::PrepareDefaultStereoRenderPasses(vkCore, scene, viewProj, RenderPasses,
                                                         swapchain->GetSwapchainImages());
    } else {
        LOGGER(LOGGER::INFO) << "Using custom render pass";
        this->RenderPasses = std::move(passes);
    }
}

void RenderBackend::GetSwapchainInfo() {
    swapchain = std::make_unique<Swapchain>();
    std::vector<std::vector<std::unique_ptr<Image>>>& swapchainImages = swapchain->GetSwapchainImages(true);
    uint8_t swapchainImageCount = xrCore->GetSwapchainImages().size();
    swapchainImages.resize(swapchainImageCount);
    for (uint32_t i = 0; i < swapchainImageCount; ++i) {
        auto [width, height] = xrCore->SwapchainExtent();
        swapchainImages[i].push_back(
            std::make_unique<Image>(vkCore, xrCore->GetSwapchainImages()[i].image,
                                    static_cast<VkFormat>(xrCore->SwapchainFormats()[0]), width, height, 2));
    }
}

void RenderBackend::InitVertexIndexBuffers() {
    if (scene->Meshes().empty()) {
        return;
    }
    // init vertex buffer and index buffer
    vertexBuffers.resize(scene->Meshes().size());
    indexBuffers.resize(scene->Meshes().size());
    for (int i = 0; i < scene->Meshes().size(); ++i) {
        auto mesh = scene->Meshes()[i];
        if (mesh.vertices.empty() || mesh.indices.empty()) {
            vertexBuffers[i] = nullptr;
            indexBuffers[i] = nullptr;
            continue;
        }

        void* verticesData = static_cast<void*>(mesh.vertices.data());
        void* indicesData = static_cast<void*>(mesh.indices.data());
        vertexBuffers[i] =
            std::make_unique<Buffer>(vkCore, sizeof(mesh.vertices[0]) * mesh.vertices.size(),
                                     VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, verticesData,
                                     true, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        indexBuffers[i] = std::make_unique<Buffer>(vkCore, sizeof(mesh.indices[0]) * mesh.indices.size(),
                                                   VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                                                   indicesData, true, VK_MEMORY_HEAP_DEVICE_LOCAL_BIT);
    }
}

bool RenderBackend::StartFrame(uint32_t& imageIndex) {
    vkWaitForFences(vkCore->GetRenderDevice(), 1, &vkCore->GetInFlightFence(), VK_TRUE, UINT64_MAX);
    auto result = vkAcquireNextImageKHR(vkCore->GetRenderDevice(), swapchain->GetSwaphcain(), UINT64_MAX,
                                        vkCore->GetImageAvailableSemaphore(), VK_NULL_HANDLE, &imageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        auto [width, height] = WindowHandler::GetFrameBufferSize();
        EventSystem::TriggerEvent(Events::XRLIB_EVENT_WINDOW_RESIZED, width, height);
        return false;
    } else if (result != VK_SUCCESS) {
        Util::ErrorPopup("Failed to acquire next image");
    }

    return true;
}

void RenderBackend::RecordFrame(uint32_t& imageIndex) {
    vkResetFences(vkCore->GetRenderDevice(), 1, &vkCore->GetInFlightFence());
    CommandBuffer commandBuffer{vkCore};
    vkResetCommandBuffer(commandBuffer.GetCommandBuffer(), 0);

    auto currentPassIndex = 0;
    auto& currentPass = *RenderPasses[currentPassIndex];
    commandBuffer.StartRecord().StartPass(currentPass, imageIndex).BindDescriptorSets(currentPass, 0);
    for (uint32_t i = 0; i < scene->Meshes().size(); ++i) {
        commandBuffer.PushConstant(currentPass, sizeof(uint32_t), &i);
        if (!vertexBuffers.empty() && !indexBuffers.empty() && vertexBuffers[i] != nullptr &&
            indexBuffers[i] != nullptr) {
            commandBuffer.BindVertexBuffer(0, {vertexBuffers[i]->GetBuffer()}, {0})
                .BindIndexBuffer(indexBuffers[i]->GetBuffer(), 0);
        }

        commandBuffer.DrawIndexed(scene->Meshes()[i].indices.size(), 1, 0, 0, 0);
    }

    // represents how many passes left to draw
    EventSystem::TriggerEvent<int, CommandBuffer&>(Events::XRLIB_EVENT_RENDERER_PRE_SUBMITTING,
                                                   (RenderPasses.size() - 1) - currentPassIndex, commandBuffer);

    commandBuffer.EndPass();

    // add barrier synchronization between render passes
    if (currentPassIndex != RenderPasses.size() - 1) {
        commandBuffer.BarrierBetweenPasses(imageIndex, currentPass);
    }

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {vkCore->GetImageAvailableSemaphore()};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 0;
    if (!xrCore->IsXRValid()) {
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
    }

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer.GetCommandBuffer();

    if (!xrCore->IsXRValid()) {
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &vkCore->GetRenderFinishedSemaphore();
    }

    commandBuffer.EndRecord(&submitInfo, vkCore->GetInFlightFence());
}

void RenderBackend::EndFrame(uint32_t& imageIndex) {
        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &vkCore->GetRenderFinishedSemaphore();
        VkSwapchainKHR swapChains[] = {swapchain->GetSwaphcain()};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;

        vkQueuePresentKHR(vkCore->GetGraphicsQueue(), &presentInfo);
}
}    // namespace Graphics
}    // namespace XRLib
