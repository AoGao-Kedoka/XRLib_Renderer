#include "VkCore.h"

namespace XRLib {
namespace Graphics {
VkCore::~VkCore() {
    for (auto framebuffer : swapChainFrameBuffers) {
        VkUtil::VkSafeClean(vkDestroyFramebuffer, vkDevice, framebuffer,
                            nullptr);
    }
    VkUtil::VkSafeClean(vkDestroyCommandPool, vkDevice, commandPool, nullptr);

    VkUtil::VkSafeClean(vkDestroyDevice, vkDevice, nullptr);
    VkUtil::VkSafeClean(vkDestroyInstance, vkInstance, nullptr);
}

uint32_t VkCore::GetMemoryType(uint32_t typeFilter,
                               VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(GetRenderPhysicalDevice(),
                                        &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) &&
            (memProperties.memoryTypes[i].propertyFlags & properties) ==
                properties) {
            return i;
        }
    }

    Util::ErrorPopup("Failed to find suitable memory type!");
    return -1;
}

void VkCore::CreateSyncSemaphore(VkSemaphore& semaphore) {
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    if (vkCreateSemaphore(GetRenderDevice(), &semaphoreInfo, nullptr,
                          &semaphore) != VK_SUCCESS) {

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
void VkCore::CreateDescriptorPool() {
    VkDescriptorPoolSize poolSizes{};
    poolSizes.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes.descriptorCount = 1;
    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSizes;
    poolInfo.maxSets = 1;
    if (vkCreateDescriptorPool(GetRenderDevice(), &poolInfo, nullptr,
                               &descriptorPool) != VK_SUCCESS) {}
}
}    // namespace Graphics
}    // namespace XRLib
