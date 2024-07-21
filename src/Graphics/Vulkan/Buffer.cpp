#include "Buffer.h"

Buffer::Buffer(std::shared_ptr<VkCore> core, VkDeviceSize size,
               VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
    : core{core} {
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(core->GetRenderDevice(), &bufferInfo, nullptr,
                       &buffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(core->GetRenderDevice(), buffer,
                                  &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex =
        FindMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(core->GetRenderDevice(), &allocInfo, nullptr,
                         &bufferMemory) != VK_SUCCESS) {
        Util::ErrorPopup("Failed to allocate buffer memory!");
    }

    vkBindBufferMemory(core->GetRenderDevice(), buffer, bufferMemory, 0);
}

void Buffer::Cleanup() {
    Util::VkSafeClean(vkDestroyBuffer, core->GetRenderDevice(), buffer,
                      nullptr);
    Util::VkSafeClean(vkFreeMemory, core->GetRenderDevice(), bufferMemory,
                      nullptr);
}

uint32_t Buffer::FindMemoryType(uint32_t typeFilter,
                                VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(core->GetRenderPhysicalDevice(),
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
