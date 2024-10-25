#pragma once

#include "Buffer.h"
#include "Logger.h"
#include "VkCore.h"

namespace XRLib {
namespace Graphics {
class Image {
   public:
    Image() = default;

    // image with data
    Image(std::shared_ptr<VkCore> core, std::vector<uint8_t> textureData, int width, int height, int channels,
          VkFormat format);

    // raw image
    Image(std::shared_ptr<VkCore> core, int width, int height, VkFormat format,
          VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL,
          VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT |
                                    VK_IMAGE_USAGE_STORAGE_BIT,
          VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, uint32_t layerCount = 1);

    // image predefined from other apis
    Image(std::shared_ptr<VkCore> core, VkImage image, VkFormat format, int width, int height, uint32_t layerCount = 1);

    ~Image();

    VkSampler& GetSampler();
    VkImageView& GetImageView(VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT);
    VkDeviceSize GetSize() { return size; }
    VkFormat GetFormat() { return format; }
    int Width() { return width; };
    int Height() { return height; }

   private:
    void CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
                     VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
    void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
    void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

   private:
    std::shared_ptr<VkCore> core;
    std::unique_ptr<Buffer> imageBuffer;
    VkImage image{VK_NULL_HANDLE};
    VkDeviceMemory imageMemorry{VK_NULL_HANDLE};
    VkImageView imageView{VK_NULL_HANDLE};
    VkFormat format{VK_FORMAT_UNDEFINED};
    VkSampler sampler{VK_NULL_HANDLE};
    VkDeviceSize size{0};

    uint32_t layerCount{1};
    int width, height;
};
}    // namespace Graphics
}    // namespace XRLib
