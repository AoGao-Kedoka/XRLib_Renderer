#include "RenderBackend.h"
#include "Utils/Util.h"

namespace XRLib {
namespace Graphics {
RenderBackend::RenderBackend(Info& info, VkCore& vkCore, XR::XrCore& xrCore, XRLib::Scene& scene)
    : info{info}, vkCore{vkCore}, xrCore{xrCore}, scene{scene} {
    vkCore.CreateVkInstance(info, xrCore.VkAdditionalInstanceExts());
    if (xrCore.IsXRValid())
        xrCore.VkSetPhysicalDevice(vkCore.GetRenderInstance(), &vkCore.VkPhysicalDeviceRef());
    else
        vkCore.SelectPhysicalDevice();
    vkCore.CreateVkDevice(info, xrCore.VkAdditionalDeviceExts(), xrCore.IsXRValid());

    EventSystem::TriggerEvent(Events::XRLIB_EVENT_RENDERBACKEND_INIT_FINISHED);

    if (xrCore.IsXRValid()) {
        GetSwapchainInfo();
    }
}

RenderBackend::~RenderBackend() {}

void RenderBackend::Prepare(std::vector<std::unique_ptr<IGraphicsRenderpass>>& passes) {
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
    std::vector<std::unique_ptr<Image>> swapchainImages;
    auto [width, height] = xrCore.SwapchainExtent();
    for (int i = 0; i < xrCore.GetSwapchainImages().size(); ++i) {
        swapchainImages.push_back(std::make_unique<Image>(vkCore, xrCore.GetSwapchainImages()[i].image,
                                                          static_cast<VkFormat>(xrCore.SwapchainFormats()[0]), width,
                                                          height, 2));
    }
    swapchain = std::make_unique<Swapchain>(vkCore, swapchainImages);
}

void RenderBackend::InitVertexIndexBuffers() {
    if (scene.Meshes().empty()) {
        return;
    }
    // init vertex buffer and index buffer
    vertexBuffers.resize(scene.Meshes().size());
    indexBuffers.resize(scene.Meshes().size());
    for (int i = 0; i < scene.Meshes().size(); ++i) {
        auto mesh = scene.Meshes()[i];
        if (mesh.GetVerticies().empty() || mesh.GetIndices().empty()) {
            vertexBuffers[i] = nullptr;
            indexBuffers[i] = nullptr;
            continue;
        }

        void* verticesData = static_cast<void*>(mesh.GetVerticies().data());
        void* indicesData = static_cast<void*>(mesh.GetIndices().data());
        vertexBuffers[i] =
            std::make_unique<Buffer>(vkCore, sizeof(mesh.GetVerticies()[0]) * mesh.GetVerticies().size(),
                                     VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, verticesData,
                                     true, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        indexBuffers[i] = std::make_unique<Buffer>(vkCore, sizeof(mesh.GetIndices()[0]) * mesh.GetIndices().size(),
                                                   VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                                                   indicesData, true, VK_MEMORY_HEAP_DEVICE_LOCAL_BIT);
    }
}

bool RenderBackend::StartFrame(uint32_t& imageIndex) {
    vkWaitForFences(vkCore.GetRenderDevice(), 1, &vkCore.GetInFlightFence(), VK_TRUE, UINT64_MAX);
    auto result = vkAcquireNextImageKHR(vkCore.GetRenderDevice(), swapchain->GetSwaphcain(), UINT64_MAX,
                                        vkCore.GetImageAvailableSemaphore(), VK_NULL_HANDLE, &imageIndex);
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
    vkResetFences(vkCore.GetRenderDevice(), 1, &vkCore.GetInFlightFence());
    CommandBuffer commandBuffer{vkCore};
    vkResetCommandBuffer(commandBuffer.GetCommandBuffer(), 0);

    // default frame recording
    auto currentPassIndex = 0;
    auto& currentPass = static_cast<VkGraphicsRenderpass&>(*RenderPasses[currentPassIndex]);
    commandBuffer.StartRecord().StartPass(currentPass, imageIndex).BindDescriptorSets(currentPass, 0);
    for (uint32_t i = 0; i < scene.Meshes().size(); ++i) {
        commandBuffer.PushConstant(currentPass, sizeof(uint32_t), &i);
        if (!vertexBuffers.empty() && !indexBuffers.empty() && vertexBuffers[i] != nullptr &&
            indexBuffers[i] != nullptr) {
            commandBuffer.BindVertexBuffer(0, {vertexBuffers[i]->GetBuffer()}, {0})
                .BindIndexBuffer(indexBuffers[i]->GetBuffer(), 0);
        }

        commandBuffer.DrawIndexed(scene.Meshes()[i].GetIndices().size(), 1, 0, 0, 0);
    }

    // represents how many passes left to draw
    EventSystem::TriggerEvent<int, CommandBuffer&>(Events::XRLIB_EVENT_RENDERER_PRE_SUBMITTING,
                                                   (RenderPasses.size() - 1) - currentPassIndex, commandBuffer);

    commandBuffer.EndPass();

    // add barrier synchronization between render passes
    if (currentPassIndex != RenderPasses.size() - 1) {
        commandBuffer.BarrierBetweenPasses(imageIndex, currentPass);
    }

    commandBuffer.EndRecord();
}

void RenderBackend::RecordFrame(uint32_t& imageIndex,
                                std::function<void(uint32_t&, CommandBuffer&)> recordingFunction) {
    vkResetFences(vkCore.GetRenderDevice(), 1, &vkCore.GetInFlightFence());
    CommandBuffer commandBuffer{vkCore};
    vkResetCommandBuffer(commandBuffer.GetCommandBuffer(), 0);

    // custom frame recording
    recordingFunction(imageIndex, commandBuffer);
}

void RenderBackend::EndFrame(uint32_t& imageIndex) {
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &vkCore.GetRenderFinishedSemaphore();
    VkSwapchainKHR swapChains[] = {swapchain->GetSwaphcain()};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;

    vkQueuePresentKHR(vkCore.GetGraphicsQueue(), &presentInfo);
}
}    // namespace Graphics
}    // namespace XRLib
