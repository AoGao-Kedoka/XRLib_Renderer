#include "Buffer.h"
#include "CommandBuffer.h"

namespace XRLib {
namespace Graphics {

void ValidateBufferUsage(VkBufferUsageFlags usage) {
    if (usage & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT && usage & VK_BUFFER_USAGE_STORAGE_BUFFER_BIT) {
        LOGGER(LOGGER::WARNING) << "Not defined buffer usage";
    }
}

Buffer::Buffer(std::shared_ptr<VkCore> core, VkDeviceSize size, VkBufferUsageFlags usage, void* data,
               bool deviceBuffer, VkMemoryPropertyFlags properties)
    : core{core}, bufferSize{size}, usage{usage} {
    ValidateBufferUsage(usage);
    CreateBuffer(size, usage, properties);
    deviceBuffer ? MapDeviceMemory(data) : MapHostMemory(data);
    if (!(usage & VK_BUFFER_USAGE_TRANSFER_DST_BIT) && deviceBuffer == true) {
        LOGGER(LOGGER::WARNING)
            << "You are not sending a transfer destination bit with memory mapping, this may result error!";
    }
}
Buffer::Buffer(std::shared_ptr<VkCore> core, VkDeviceSize size, VkBufferUsageFlags usage,
               VkMemoryPropertyFlags properties)
    : core{core}, bufferSize{size}, usage{usage} {
    ValidateBufferUsage(usage);
    CreateBuffer(size, usage, properties);
}

Buffer::~Buffer() {
    VkUtil::VkSafeClean(vkDestroyBuffer, core->GetRenderDevice(), buffer, nullptr);
    VkUtil::VkSafeClean(vkFreeMemory, core->GetRenderDevice(), bufferMemory, nullptr);
}

void Buffer::UpdateBuffer(VkDeviceSize size, void* data) {
    auto cb = CommandBuffer::BeginSingleTimeCommands(core);
    vkCmdUpdateBuffer(cb->GetCommandBuffer(), buffer, 0, size, data);
    CommandBuffer::EndSingleTimeCommands(std::move(cb));
}

void Buffer::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) {
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(core->GetRenderDevice(), &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(core->GetRenderDevice(), buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = core->GetMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(core->GetRenderDevice(), &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
        Util::ErrorPopup("Failed to allocate buffer memory!");
    }

    vkBindBufferMemory(this->core->GetRenderDevice(), buffer, bufferMemory, 0);
}

void Buffer::MapHostMemory(void* dataInput) {
    vkMapMemory(core->GetRenderDevice(), bufferMemory, 0, bufferSize, 0, &data);
    std::memcpy(data, dataInput, (size_t)bufferSize);
}

void Buffer::MapDeviceMemory(void* dataInput) {
    if (this->buffer == VK_NULL_HANDLE || bufferSize == 0) {
        LOGGER(LOGGER::ERR) << "Invalid buffer or buffer size for memory mapping";
        return;
    }
    Buffer stagingBuffer{this->core, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};

    vkMapMemory(core->GetRenderDevice(), stagingBuffer.GetDeviceMemory(), 0, bufferSize, 0, &data);
    std::memcpy(data, dataInput, (size_t)bufferSize);
    vkUnmapMemory(core->GetRenderDevice(), stagingBuffer.GetDeviceMemory());

    auto cb = CommandBuffer::BeginSingleTimeCommands(core);

    VkBufferCopy copyRegion{};
    copyRegion.size = bufferSize;
    vkCmdCopyBuffer(cb->GetCommandBuffer(), stagingBuffer.GetBuffer(), buffer, 1, &copyRegion);

    CommandBuffer::EndSingleTimeCommands(std::move(cb));
}

}    // namespace Graphics
}    // namespace XRLib
