#include "CommandBuffer.h"

namespace XRLib {
namespace Graphics {

CommandBuffer::CommandBuffer(VkCore& core) : core{core} {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = core.GetCommandPool();
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(core.GetRenderDevice(), &allocInfo, &commandBuffer) != VK_SUCCESS) {
        Util::ErrorPopup("failed to allocate command buffers!");
    }
}

CommandBuffer::~CommandBuffer() {
    vkFreeCommandBuffers(core.GetRenderDevice(), core.GetCommandPool(), 1, &commandBuffer);
}

CommandBuffer CommandBuffer::BeginSingleTimeCommands(VkCore& core) {
    CommandBuffer commandBuffer{core};
    commandBuffer.StartRecord();
    return commandBuffer;
}

void CommandBuffer::EndSingleTimeCommands(CommandBuffer& commandBuffer) {
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer.GetCommandBuffer();
    commandBuffer.EndRecord(&submitInfo, VK_NULL_HANDLE);
}

CommandBuffer& CommandBuffer::BindVertexBuffer(int firstBinding, std::vector<VkBuffer> buffers,
                                               std::vector<VkDeviceSize> offsets) {
    vkCmdBindVertexBuffers(commandBuffer, 0, buffers.size(), buffers.data(), offsets.data());
    return *this;
}

CommandBuffer& CommandBuffer::BindIndexBuffer(VkBuffer indexBuffer, VkDeviceSize offset, VkIndexType indexType) {
    vkCmdBindIndexBuffer(commandBuffer, indexBuffer, offset, indexType);
    return *this;
}

CommandBuffer& CommandBuffer::BindDescriptorSets(VkGraphicsRenderpass& pass, uint32_t firstSet,
                                                 uint32_t dynamicOffsetCount, const uint32_t* pDynamicOffsets) {
    if (pass.GetDescriptorSets().empty()) {
        LOGGER(LOGGER::WARNING) << "No descriptor set available, skipping";
        return *this;
    }

    std::vector<VkDescriptorSet> descriptorSets;
    descriptorSets.reserve(pass.GetDescriptorSets().size());
    for (const auto& descriptorSet : pass.GetDescriptorSets()) {
        if (descriptorSet != nullptr) {
            descriptorSets.push_back(descriptorSet->GetVkDescriptorSet());
        }
    }
    VkPipelineLayout layout = pass.GetPipeline().GetVkPipelineLayout();
    VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    vkCmdBindDescriptorSets(commandBuffer, bindPoint, layout, firstSet, descriptorSets.size(), descriptorSets.data(),
                            dynamicOffsetCount, pDynamicOffsets);
    return *this;
}

CommandBuffer& CommandBuffer::StartRecord() {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;                     // Optional
    beginInfo.pInheritanceInfo = nullptr;    // Optional

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        Util::ErrorPopup("Failed to begin recording command buffer");
    }
    return *this;
}

CommandBuffer& CommandBuffer::StartPass(VkGraphicsRenderpass& pass, uint32_t imageIndex) {
    if (pass.GetPipeline().GetVkPipeline() == VK_NULL_HANDLE) {
        Util::ErrorPopup("Graphics pipeline not initialized");
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = pass.GetRenderpass().GetVkRenderpass();
    renderPassInfo.framebuffer = pass.GetRenderpass().GetFrameBuffers()[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = {
        static_cast<uint32_t>(pass.GetRenderpass().GetRenderTargets()[imageIndex][0]->Width()),
        static_cast<uint32_t>(pass.GetRenderpass().GetRenderTargets()[imageIndex][0]->Height())};

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
    clearValues[1].depthStencil = {1.0f, 0};
    renderPassInfo.clearValueCount = clearValues.size();
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pass.GetPipeline().GetVkPipeline());

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = renderPassInfo.renderArea.extent.width;
    viewport.height = renderPassInfo.renderArea.extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = renderPassInfo.renderArea.extent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
    currentPass = &pass;
    return *this;
}

CommandBuffer& CommandBuffer::PushConstant(VkGraphicsRenderpass& pass, uint32_t size, const void* ptr) {
    vkCmdPushConstants(this->commandBuffer, pass.GetPipeline().GetVkPipelineLayout(),
                       VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, size, ptr);
    return *this;
}

CommandBuffer& CommandBuffer::EndPass() {
    if (currentPass == nullptr) {
        LOGGER(LOGGER::DEBUG) << "No pass started, ignoring end pass";
        return *this;
    }
    vkCmdEndRenderPass(commandBuffer);
    currentPass = nullptr;
    return *this;
}

CommandBuffer& CommandBuffer::DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex,
                                          uint32_t vertexOffset, uint32_t firstInstance) {
    vkCmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
    return *this;
}

CommandBuffer& CommandBuffer::Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex,
                                   uint32_t firstInstance) {
    vkCmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
    return *this;
}

void CommandBuffer::EndRecord() {
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {core.GetImageAvailableSemaphore()};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &GetCommandBuffer();
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &core.GetRenderFinishedSemaphore();

    EndRecord(&submitInfo, core.GetInFlightFence());
}

void CommandBuffer::EndRecord(VkSubmitInfo* submitInfo, VkFence fence) {
    if (currentPass != nullptr) {
        vkCmdEndRenderPass(commandBuffer);
    }
    vkEndCommandBuffer(commandBuffer);
    vkQueueSubmit(core.GetGraphicsQueue(), 1, submitInfo, fence);
    vkQueueWaitIdle(core.GetGraphicsQueue());
}

void CommandBuffer::BarrierBetweenPasses(uint32_t imageIndex, VkGraphicsRenderpass& pass) {
        for (const auto& renderTarget : pass.GetRenderpass().GetRenderTargets()[imageIndex]) {
            VkImageMemoryBarrier barrier = {};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.image = renderTarget->GetImage();
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier.subresourceRange.baseMipLevel = 0;
            barrier.subresourceRange.levelCount = 1;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = 1;
            PipelineBarrier(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                          VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
        }
}

void CommandBuffer::PipelineBarrier(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
                                    VkDependencyFlags dependencyFlags, uint32_t memoryBarrierCount,
                                    const VkMemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount,
                                    const VkBufferMemoryBarrier* pBufferMemoryBarriers,
                                    uint32_t imageMemoryBarrierCount,
                                    const VkImageMemoryBarrier* pImageMemoryBarriers) {
    vkCmdPipelineBarrier(commandBuffer, srcStageMask, dstStageMask, dependencyFlags, memoryBarrierCount,
                         pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount,
                         pImageMemoryBarriers);
}

}    // namespace Graphics
}    // namespace XRLib
