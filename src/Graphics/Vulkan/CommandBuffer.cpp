#include "CommandBuffer.h"

namespace XRLib {
namespace Graphics {
void CommandBuffer::BeginSingleTimeCommands(std::shared_ptr<VkCore> core) {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    if (vkBeginCommandBuffer(core->GetCommandBuffer(), &beginInfo) !=
        VK_SUCCESS) {
        Util::ErrorPopup("Failed to begin command buffer");
    }
}

void CommandBuffer::EndSingleTimeCommands(std::shared_ptr<VkCore> core) {
    vkEndCommandBuffer(core->GetCommandBuffer());

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &core->GetCommandBuffer();

    vkQueueSubmit(core->GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(core->GetGraphicsQueue());
}

void CommandBuffer::Record(std::shared_ptr<VkCore> core,
                           std::shared_ptr<GraphicsRenderPass> pass,
                           uint32_t imageIndex) {

    if (pass->GetPipeline().GetVkPipeline() == VK_NULL_HANDLE) {
        Util::ErrorPopup("Graphics pipeline not initialized");
    }

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;                     // Optional
    beginInfo.pInheritanceInfo = nullptr;    // Optional

    if (vkBeginCommandBuffer(core->GetCommandBuffer(), &beginInfo) !=
        VK_SUCCESS) {
        Util::ErrorPopup("Failed to begin recording command buffer");
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

    vkCmdBeginRenderPass(core->GetCommandBuffer(), &renderPassInfo,
                         VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(core->GetCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS,
                      pass->GetPipeline().GetVkPipeline());

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = renderPassInfo.renderArea.extent.width;
    viewport.height = renderPassInfo.renderArea.extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(core->GetCommandBuffer(), 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = renderPassInfo.renderArea.extent;
    vkCmdSetScissor(core->GetCommandBuffer(), 0, 1, &scissor);

    // TODO Bind vertex buffer,index buffer and descriptor sets

    vkCmdDraw(core->GetCommandBuffer(), 3, 1, 0, 0);

    vkCmdEndRenderPass(core->GetCommandBuffer());

    if (vkEndCommandBuffer(core->GetCommandBuffer()) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }
}
}    // namespace Graphics
}    // namespace XRLib
