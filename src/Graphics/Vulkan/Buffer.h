#pragma once

#include "Logger.h"
#include "VkCore.h"

namespace XRLib {
namespace Graphics {

class Buffer {
   public:
    Buffer(VkCore& core, VkDeviceSize size, VkBufferUsageFlags usage,
           VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                              VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    Buffer(VkCore& core, VkDeviceSize size, VkBufferUsageFlags usage, void* data, bool deviceBuffer,
           VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                              VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    ~Buffer();

    VkBuffer& GetBuffer() { return buffer; }
    VkDeviceMemory GetDeviceMemory() { return bufferMemory; }
    void* GetMappedData() { return data; }
    VkDeviceSize GetSize() { return bufferSize; }
    bool IsUniformBuffer() { return usage & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT; }
    bool IsStorageBuffer() { return usage & VK_BUFFER_USAGE_STORAGE_BUFFER_BIT; }

    void UpdateBuffer(VkDeviceSize size, void* data);

   private:
    void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);

    void MapHostMemory(void* dataInput);

    /*
     * Create a staging buffer and send the current buffer to GPU, if it's a device buffer
     */
    void MapDeviceMemory(void* dataInput);

    VkCore& core;
    VkBuffer buffer{VK_NULL_HANDLE};
    VkDeviceMemory bufferMemory{VK_NULL_HANDLE};
    VkDeviceSize bufferSize{0};
    VkBufferUsageFlags usage;
    void* data;
};
}    // namespace Graphics
}    // namespace XRLib
