#pragma once

#include "Logger.h"
#include "VkCore.h"

namespace XRLib {
namespace Graphics {

class Buffer {
   public:
    Buffer(std::shared_ptr<VkCore> core, VkDeviceSize size,
           VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
    Buffer(std::shared_ptr<VkCore> core, VkDeviceSize size,
           VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
           void* data, bool deviceBuffer);
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
    void* GetMappedData() { return data; }

   private:
    uint32_t FindMemoryType(uint32_t typeFilter,
                            VkMemoryPropertyFlags properties);

    void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                      VkMemoryPropertyFlags properties);

    void MapHostMemory(void* dataInput);
    /*
     * Create a staging buffer and send the current buffer to GPU, if it's a device buffer
     */
    void MapDeviceMemory(void* dataInput);

    std::shared_ptr<VkCore> core{nullptr};
    VkBuffer buffer{VK_NULL_HANDLE};
    VkDeviceMemory bufferMemory{VK_NULL_HANDLE};
    VkDeviceSize bufferSize{0};
    void* data;
};
}    // namespace Graphics
}    // namespace XRLib
