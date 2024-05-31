#include "VkCore.h"

void VkCore::CreateCommandBuffer() {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = GetCommandPool();
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(GetRenderDevice(), &allocInfo,
                                 &commandBuffer) != VK_SUCCESS) {
        LOGGER(LOGGER::ERR) << "failed to allocate command buffers!";
        exit(-1);
    }
}

void VkCore::CreateSemaphore(VkSemaphore& semaphore) {
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    if (vkCreateSemaphore(vkDevice, &semaphoreInfo, nullptr, &semaphore) !=
        VK_SUCCESS) {
        LOGGER(LOGGER::ERR) << "Failed to create semaphore";
        exit(-1);
    }
}

void VkCore::CreateFence(VkFence& fence) {
    VkFenceCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_EXPORT_FENCE_CREATE_INFO;
    info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    if (vkCreateFence(GetRenderDevice(), &info, nullptr, &fence)) {
        LOGGER(LOGGER::ERR) << "Failed to create fence!";
        exit(-1);
	}
}

void VkCore::CreateCommandPool() {
    auto graphicsFamilyIndex = GetGraphicsQueueFamilyIndex();
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = graphicsFamilyIndex;
    if (vkCreateCommandPool(GetRenderDevice(), &poolInfo, nullptr,
                            &commandPool) != VK_SUCCESS) {
        LOGGER(LOGGER::ERR) << "failed to create command pool!";
        exit(-1);
    }
}
