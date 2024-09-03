#pragma once

#include "CommandBuffer.h"
#include "VkCore.h"
#include "Logger.h"

namespace XRLib {
namespace Graphics {
class Buffer {
   public:
    Buffer(std::shared_ptr<VkCore> core, VkDeviceSize size,
           VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
    Buffer(std::shared_ptr<VkCore> core, VkDeviceSize size,
           VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
           void* data);
    Buffer() = default;
    ~Buffer();

    Buffer& operator=(const Buffer& other) {
        this->core = other.core;
        this->buffer = other.buffer;
        this->bufferMemory = other.bufferMemory;
        return *this;
    }

    VkBuffer& GetBuffer() { return buffer; }
    VkDeviceMemory GetDeviceMemory() { return bufferMemory; }

   private:
    uint32_t FindMemoryType(uint32_t typeFilter,
                            VkMemoryPropertyFlags properties);

    void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                      VkMemoryPropertyFlags properties);

    /*
     * Create a staging buffer and send the current buffer to GPU
     */
    void MapMemory(void* dataInput);

    std::shared_ptr<VkCore> core{nullptr};
    VkBuffer buffer{VK_NULL_HANDLE};
    VkDeviceMemory bufferMemory{VK_NULL_HANDLE};
    VkDeviceSize bufferSize{0};
};
}    // namespace Graphics
}    // namespace XRLib
