#include "CommandBuffer.h"

namespace XRLib {
namespace Graphics {

CommandBuffer::CommandBuffer(std::shared_ptr<VkCore> core) : core{core} {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = core->GetCommandPool();
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(core->GetRenderDevice(), &allocInfo,
                                 &commandBuffer) != VK_SUCCESS) {
        Util::ErrorPopup("failed to allocate command buffers!");
    }
}

CommandBuffer::~CommandBuffer() {
    vkFreeCommandBuffers(core->GetRenderDevice(), core->GetCommandPool(), 1,
                         &commandBuffer);
}

std::shared_ptr<CommandBuffer>
CommandBuffer::BeginSingleTimeCommands(std::shared_ptr<VkCore> core) {
    std::shared_ptr<CommandBuffer> commandBuffer = std::make_shared<CommandBuffer>(core);
    commandBuffer->StartRecord();
    return commandBuffer;
}

void CommandBuffer::EndSingleTimeCommands(std::shared_ptr<CommandBuffer> commandBuffer) {
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer->GetCommandBuffer();
    commandBuffer->EndRecord(&submitInfo, VK_NULL_HANDLE);
}

CommandBuffer& CommandBuffer::BindVertexBuffer(int firstBinding,
                                               std::vector<VkBuffer> buffers,
                                               std::vector<VkDeviceSize> offsets) {
    vkCmdBindVertexBuffers(commandBuffer, 0, buffers.size(), buffers.data(),
                           offsets.data());
    return *this;
}

CommandBuffer& CommandBuffer::BindIndexBuffer(VkBuffer indexBuffer,
                                              VkDeviceSize offset,
                                              VkIndexType indexType) {
    vkCmdBindIndexBuffer(commandBuffer, indexBuffer, offset, indexType);
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

CommandBuffer& CommandBuffer::StartPass(std::shared_ptr<GraphicsRenderPass> pass,
                                      uint32_t imageIndex) {

    if (pass->GetPipeline().GetVkPipeline() == VK_NULL_HANDLE) {
        Util::ErrorPopup("Graphics pipeline not initialized");
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = pass->GetRenderPass().GetVkRenderPass();
    renderPassInfo.framebuffer = core->GetSwapchainFrameBuffer()[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = core->GetSwapchainExtent(pass->Stereo());

    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo,
                         VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      pass->GetPipeline().GetVkPipeline());

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
    currentPass = pass;
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

CommandBuffer& CommandBuffer::DrawIndexed(uint32_t indexCount,
                                          uint32_t instanceCount,
                                          uint32_t firstIndex,
                                          uint32_t vertexOffset,
                                          uint32_t firstInstance) {
    vkCmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex,
                     vertexOffset, firstInstance);
    return *this;
}

CommandBuffer& CommandBuffer::Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) {
    vkCmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex,
              firstInstance);
    return *this;
}

void CommandBuffer::EndRecord(VkSubmitInfo* submitInfo, VkFence fence) {
    if (currentPass != nullptr) {
        vkCmdEndRenderPass(commandBuffer);
    }
    vkEndCommandBuffer(commandBuffer);
    vkQueueSubmit(core->GetGraphicsQueue(), 1, submitInfo, fence);
    vkQueueWaitIdle(core->GetGraphicsQueue());
}

}    // namespace Graphics
}    // namespace XRLib
