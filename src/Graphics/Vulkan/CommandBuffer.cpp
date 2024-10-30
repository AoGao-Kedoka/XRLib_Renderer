#include "CommandBuffer.h"

namespace XRLib {
namespace Graphics {

CommandBuffer::CommandBuffer(std::shared_ptr<VkCore> core) : core{core} {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = core->GetCommandPool();
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(core->GetRenderDevice(), &allocInfo, &commandBuffer) != VK_SUCCESS) {
        Util::ErrorPopup("failed to allocate command buffers!");
    }
}

CommandBuffer::~CommandBuffer() {
    vkFreeCommandBuffers(core->GetRenderDevice(), core->GetCommandPool(), 1, &commandBuffer);
}

std::unique_ptr<CommandBuffer> CommandBuffer::BeginSingleTimeCommands(std::shared_ptr<VkCore> core) {
    auto commandBuffer = std::make_unique<CommandBuffer>(core);
    commandBuffer->StartRecord();
    return commandBuffer;
}

void CommandBuffer::EndSingleTimeCommands(std::unique_ptr<CommandBuffer> commandBuffer) {
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer->GetCommandBuffer();
    commandBuffer->EndRecord(&submitInfo, VK_NULL_HANDLE);
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

CommandBuffer& CommandBuffer::BindDescriptorSets(GraphicsRenderPass& pass, uint32_t firstSet,
                                                 uint32_t dynamicOffsetCount, const uint32_t* pDynamicOffsets) {
    if (pass.GetDescriptorSets().empty()) {
        LOGGER(LOGGER::WARNING) << "No descriptor set available, skipping";
        return *this;
    }

    std::vector<VkDescriptorSet> descriptorSets;
    descriptorSets.reserve(pass.GetDescriptorSets().size());
    for (const auto descriptorSet : pass.GetDescriptorSets()) {
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

CommandBuffer& CommandBuffer::StartPass(GraphicsRenderPass& pass, uint32_t imageIndex) {
    if (pass.GetPipeline().GetVkPipeline() == VK_NULL_HANDLE) {
        Util::ErrorPopup("Graphics pipeline not initialized");
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = pass.GetRenderPass().GetVkRenderPass();
    renderPassInfo.framebuffer = pass.GetRenderPass().GetFrameBuffers()[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = {static_cast<uint32_t>(pass.GetRenderPass().GetRenderTargets()[imageIndex]->Width()),
                                        static_cast<uint32_t>(pass.GetRenderPass().GetRenderTargets()[imageIndex]->Height())};

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
CommandBuffer& CommandBuffer::PushConstant(GraphicsRenderPass& pass, uint32_t size, const void* ptr) {
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
