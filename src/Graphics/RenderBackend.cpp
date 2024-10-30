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
}

RenderBackend::~RenderBackend() {}

void RenderBackend::Prepare(std::vector<std::pair<const std::string&, const std::string&>> passesToAdd) {
    GetSwapchainInfo();
    InitVertexIndexBuffers();

    auto stereoExtent = vkCore->GetswapchainExtentStereo();
    depthImage = std::make_unique<Image>(
        vkCore, stereoExtent.width, stereoExtent.height, VkUtil::FindDepthFormat(vkCore->GetRenderPhysicalDevice()),
        VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 2);

    if (passesToAdd.empty()) {
        VulkanDefaults::PrepareDefaultStereoRenderPasses(vkCore, scene, viewProj, RenderPasses);
    } else {
        LOGGER(LOGGER::INFO) << "Using custom render pass";
        // TODO: custom render pass
        //std::vector<std::shared_ptr<DescriptorSet>> sets;
        //for (auto& pass : passesToAdd) {
        //    auto graphicsRenderPass = std::make_unique<GraphicsRenderPass>(vkCore, true, sets, pass.first, pass.second);
        //    RenderPasses.push_back(std::move(graphicsRenderPass));
        //}
    }
    InitFrameBuffer();
}

void RenderBackend::GetSwapchainInfo() {
    uint8_t swapchainImageCount = xrCore->GetSwapchainImages().size();
    vkCore->GetStereoSwapchainImages().resize(swapchainImageCount);
    vkCore->GetStereoSwapchainImageViews().resize(swapchainImageCount);
    for (uint32_t i = 0; i < xrCore->GetSwapchainImages().size(); ++i) {
        // create image view
        vkCore->GetStereoSwapchainImages()[i] = xrCore->GetSwapchainImages()[i].image;
        VkImageViewCreateInfo imageViewCreateInfo{};
        imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.image = vkCore->GetStereoSwapchainImages()[i];
        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
        imageViewCreateInfo.format = vkCore->GetStereoSwapchainImageFormat();
        imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
        imageViewCreateInfo.subresourceRange.levelCount = 1;
        imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        imageViewCreateInfo.subresourceRange.layerCount = 2;
        if (vkCreateImageView(vkCore->GetRenderDevice(), &imageViewCreateInfo, nullptr,
                              &vkCore->GetStereoSwapchainImageViews()[i]) != VK_SUCCESS) {
            Util::ErrorPopup("Failed to create image view");
        }
    }

    vkCore->SetStereoSwapchainExtent2D({xrCore->GetXRViewConfigurationView()[0].recommendedImageRectWidth,
                                        xrCore->GetXRViewConfigurationView()[0].recommendedImageRectHeight});
}

void RenderBackend::InitVertexIndexBuffers() {
    // init vertex buffer and index buffer
    for (int i = 0; i < scene->Meshes().size(); ++i) {
        auto mesh = scene->Meshes()[i];
        void* verticesData = static_cast<void*>(mesh.vertices.data());
        void* indicesData = static_cast<void*>(mesh.indices.data());
        vertexBuffers.push_back(
            std::make_unique<Buffer>(vkCore, sizeof(mesh.vertices[0]) * mesh.vertices.size(),
                                     VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, verticesData,
                                     true, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT));

        indexBuffers.push_back(
            std::make_unique<Buffer>(vkCore, sizeof(mesh.indices[0]) * mesh.indices.size(),
                                     VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, indicesData,
                                     true, VK_MEMORY_HEAP_DEVICE_LOCAL_BIT));
    }
}

void RenderBackend::InitFrameBuffer() {
    // create frame buffer
    if (xrCore->IsXRValid()) {
        vkCore->GetSwapchainFrameBuffer().resize(vkCore->GetStereoSwapchainImageViews().size());
    } 

    for (size_t i = 0; i < vkCore->GetSwapchainFrameBuffer().size(); i++) {
        std::vector<VkImageView> attachments;

        if (xrCore->IsXRValid()) {
            attachments.push_back(vkCore->GetStereoSwapchainImageViews()[i]);
        } 
        attachments.push_back(depthImage->GetImageView(VK_IMAGE_ASPECT_DEPTH_BIT));

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = RenderPasses[RenderPasses.size() - 1]->GetRenderPass().GetVkRenderPass();
        framebufferInfo.attachmentCount = attachments.size();
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = vkCore->GetSwapchainExtent(xrCore->IsXRValid()).width;
        framebufferInfo.height = vkCore->GetSwapchainExtent(xrCore->IsXRValid()).height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(vkCore->GetRenderDevice(), &framebufferInfo, nullptr,
                                &vkCore->GetSwapchainFrameBuffer()[i]) != VK_SUCCESS) {
            Util::ErrorPopup("Failed to create frame buffer");
        }
    }
}
void RenderBackend::Run(uint32_t& imageIndex) {
    CommandBuffer commandBuffer{vkCore};
    vkWaitForFences(vkCore->GetRenderDevice(), 1, &vkCore->GetInFlightFence(), VK_TRUE, UINT64_MAX);
    vkResetCommandBuffer(commandBuffer.GetCommandBuffer(), 0);

    if (!xrCore->IsXRValid()) {
        auto result = vkAcquireNextImageKHR(vkCore->GetRenderDevice(), swapchain->GetSwaphcain(), UINT64_MAX,
                                            vkCore->GetImageAvailableSemaphore(), VK_NULL_HANDLE, &imageIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            auto [width, height] = WindowHandler::GetFrameBufferSize();
            EventSystem::TriggerEvent(Events::XRLIB_EVENT_WINDOW_RESIZED, width, height);
            return;
        } else if (result != VK_SUCCESS) {
            Util::ErrorPopup("Failed to acquire next image");
        }
    }

    vkResetFences(vkCore->GetRenderDevice(), 1, &vkCore->GetInFlightFence());

    auto currentPassIndex = 0;
    auto currentPass = *RenderPasses[currentPassIndex];
    commandBuffer.StartRecord().StartPass(currentPass, imageIndex).BindDescriptorSets(currentPass, 0);
    for (uint32_t i = 0; i < scene->Meshes().size(); ++i) {
        commandBuffer.PushConstant(currentPass, sizeof(uint32_t), &i)
            .BindVertexBuffer(0, {vertexBuffers[i]->GetBuffer()}, {0})
            .BindIndexBuffer(indexBuffers[i]->GetBuffer(), 0)
            .DrawIndexed(scene->Meshes()[i].indices.size(), 1, 0, 0, 0);
    }

    if (currentPassIndex == RenderPasses.size() - 1) {
        EventSystem::TriggerEvent<CommandBuffer&>(Events::XRLIB_EVENT_RENDERER_PRE_SUBMITTING, commandBuffer);
    }

    commandBuffer.EndPass();

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

    VkSemaphore signalSemaphores[] = {vkCore->GetRenderFinishedSemaphore()};
    if (!xrCore->IsXRValid()) {
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;
    }

    commandBuffer.EndRecord(&submitInfo, vkCore->GetInFlightFence());

    if (!xrCore->IsXRValid()) {
        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;
        VkSwapchainKHR swapChains[] = {swapchain->GetSwaphcain()};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;

        vkQueuePresentKHR(vkCore->GetGraphicsQueue(), &presentInfo);
    }
}
}    // namespace Graphics
}    // namespace XRLib
