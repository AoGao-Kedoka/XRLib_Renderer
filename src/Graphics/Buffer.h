#pragma once

#include <stdexcept>
#include <vulkan/vulkan.h>

#include "VkCore.h"

class Buffer {
   public:
    Buffer(std::shared_ptr<VkCore> core, VkDeviceSize size,
           VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
    Buffer() {};

    Buffer& operator=(const Buffer& other) {
        this->core = other.core;
        this->buffer = other.buffer;
        this->bufferMemory = other.bufferMemory;
        return *this;
    }

    VkBuffer GetBuffer() { return buffer; }
    VkDeviceMemory GetDeviceMemory() { return bufferMemory; }
    void Cleanup();

   private:
    void CheckValue() {
        if (buffer == VK_NULL_HANDLE || bufferMemory == VK_NULL_HANDLE) {
            LOGGER(LOGGER::ERR) << "Something went wrong with buffer";
            exit(-1);
        }
    }

    uint32_t FindMemoryType(uint32_t typeFilter,
                            VkMemoryPropertyFlags properties);

    std::shared_ptr<VkCore> core{nullptr};
    VkBuffer buffer{VK_NULL_HANDLE};
    VkDeviceMemory bufferMemory{VK_NULL_HANDLE};
};
