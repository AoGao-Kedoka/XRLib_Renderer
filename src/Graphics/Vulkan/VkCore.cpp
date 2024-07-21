#include "VkCore.h"

void VkCore::CreateCommandBuffer() {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = GetCommandPool();
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(GetRenderDevice(), &allocInfo,
                                 &commandBuffer) != VK_SUCCESS) {
        Util::ErrorPopup("failed to allocate command buffers!");
    }
}

void VkCore::CreateSyncSemaphore(VkSemaphore& semaphore)
{
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    if (vkCreateSemaphore(GetRenderDevice(), &semaphoreInfo, nullptr,
                          &semaphore) !=
        VK_SUCCESS) {

        Util::ErrorPopup("Failed to create semaphore");
    }
}

void VkCore::CreateFence(VkFence& fence) {
    VkFenceCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    if (vkCreateFence(GetRenderDevice(), &info, nullptr, &fence)) {
        Util::ErrorPopup("Failed to create fence!");
	}
}

void VkCore::BeginSingleTimeCommands() {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    if (vkBeginCommandBuffer(GetCommandBuffer(), &beginInfo) != VK_SUCCESS) {
        Util::ErrorPopup("Failed to begin command buffer");
    }
}

void VkCore::EndSingleTimeCommands() {
    vkEndCommandBuffer(GetCommandBuffer());

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(GetGraphicsQueue(), 1, &submitInfo,
                  VK_NULL_HANDLE);
    vkQueueWaitIdle(GetGraphicsQueue());
}

void VkCore::CreateCommandPool() {
    auto graphicsFamilyIndex = GetGraphicsQueueFamilyIndex();
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = graphicsFamilyIndex;
    if (vkCreateCommandPool(GetRenderDevice(), &poolInfo, nullptr,
                            &commandPool) != VK_SUCCESS) {
        Util::ErrorPopup("failed to create command pool!");
    }
}
