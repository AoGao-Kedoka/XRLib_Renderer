#pragma once

#include "Buffer.h"

namespace XRLib {
namespace Graphics {
class Image {
   public:
    // image with data
    Image(VkCore& core, std::vector<uint8_t> textureData, const unsigned int width, const unsigned int height, const unsigned int channels,
          VkFormat format);

    // raw image
    Image(VkCore& core, const unsigned int width, const unsigned int height, VkFormat format,
          VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL,
          VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT |
                                    VK_IMAGE_USAGE_STORAGE_BIT,
          VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, uint32_t layerCount = 1);

    // image predefined from other apis
    Image(VkCore& core, VkImage image, VkFormat format, const unsigned int width, const unsigned int height, uint32_t layerCount = 1);

    ~Image();

    VkImage& GetImage() { return image; }
    VkSampler& GetSampler();
    VkImageView& GetImageView(VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT);
    VkDeviceSize GetSize() { return size; }
    VkFormat GetFormat() { return format; }
    unsigned int Width() { return width; };
    unsigned int Height() { return height; }
    void Resize(unsigned int width, unsigned int height);
    void ResetImage();

   private:
    void CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
                     VkMemoryPropertyFlags properties);
    void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
    void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
    void StoreImageProperties(VkFormat format, VkImageTiling tiling, VkImageUsageFlags usageFlags, VkMemoryPropertyFlags propertyFlags);

   private:
    VkCore& core;
    std::unique_ptr<Buffer> imageBuffer{nullptr};
    VkImage image{VK_NULL_HANDLE};
    VkDeviceMemory imageMemory{VK_NULL_HANDLE};
    VkImageView imageView{VK_NULL_HANDLE};
    VkFormat format{VK_FORMAT_UNDEFINED};
    VkSampler sampler{VK_NULL_HANDLE};
    VkDeviceSize size{0};
    VkImageTiling tiling;
    VkImageUsageFlags usageFlags;
    VkMemoryPropertyFlags propertyFlags;
    uint32_t layerCount{1};
    unsigned int width, height;
    bool resizable = false;
};
}    // namespace Graphics
}    // namespace XRLib
