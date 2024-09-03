#include "Buffer.h"

namespace XRLib {
namespace Graphics {
Buffer::Buffer(std::shared_ptr<VkCore> core, VkDeviceSize size,
               VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
               void* data)
    : core{core}, bufferSize{size} {
    CreateBuffer(size, usage, properties);
    MapMemory(data);
    if (!usage & VK_BUFFER_USAGE_TRANSFER_DST_BIT) {
        LOGGER(LOGGER::WARNING)
            << "You are not sending a transfer destnation bit with memory mapping, "
               "this may result error!";
    }
}
Buffer::Buffer(std::shared_ptr<VkCore> core, VkDeviceSize size,
               VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
    : core{core}, bufferSize{size} {
    CreateBuffer(size, usage, properties);
}

Buffer::~Buffer() {
    VkUtil::VkSafeClean(vkDestroyBuffer, core->GetRenderDevice(), buffer,
                        nullptr);
    VkUtil::VkSafeClean(vkFreeMemory, core->GetRenderDevice(), bufferMemory,
                        nullptr);
}

void Buffer::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                          VkMemoryPropertyFlags properties) {
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

    vkBindBufferMemory(this->core->GetRenderDevice(), buffer, bufferMemory, 0);
}

void Buffer::MapMemory(void* dataInput) {
    if (this->buffer == VK_NULL_HANDLE || bufferSize == 0) {
        LOGGER(LOGGER::ERR) << "Invalid buffer or buffer size for memory mapping";
        return;
    }
    Buffer stagingBuffer{
        this->core,
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
    };
    void* data;
    vkMapMemory(core->GetRenderDevice(), stagingBuffer.GetDeviceMemory(), 0,
                bufferSize, 0, &data);
    std::memcpy(data, dataInput, (size_t)bufferSize);
    vkUnmapMemory(core->GetRenderDevice(), stagingBuffer.GetDeviceMemory());

    auto cb = CommandBuffer::BeginSingleTimeCommands(core);

    VkBufferCopy copyRegion{};
    copyRegion.size = bufferSize;
    vkCmdCopyBuffer(cb->GetCommandBuffer(), stagingBuffer.GetBuffer(),
                    buffer,
                    1, &copyRegion);

    CommandBuffer::EndSingleTimeCommands(cb);
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
}    // namespace Graphics
}    // namespace XRLib
