#pragma once

#include "Buffer.h"
#include "Logger.h"
#include "VkCore.h"

namespace XRLib {
namespace Graphics {
class Image {
   public:
    Image() = default;
    Image(std::shared_ptr<VkCore> core, const std::string& path,
          VkFormat format);
    Image(std::shared_ptr<VkCore> core, std::pair<int, int> frameSize,
          VkFormat format);
    ~Image();

    VkSampler& GetSampler();
    VkImageView& GetImageView();
    VkDeviceSize GetSize() { return size; }

   private:
    void CreateImage(uint32_t width, uint32_t height, VkFormat format,
                     VkImageTiling tiling, VkImageUsageFlags usage,
                     VkMemoryPropertyFlags properties, VkImage& image,
                     VkDeviceMemory& imageMemory);
    void TransitionImageLayout(VkImage image, VkFormat format,
                               VkImageLayout oldLayout,
                               VkImageLayout newLayout);
    void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width,
                           uint32_t height);

   private:
    std::shared_ptr<VkCore> core;
    std::unique_ptr<Buffer> imageBuffer;
    VkImage image{VK_NULL_HANDLE};
    VkDeviceMemory imageMemorry{VK_NULL_HANDLE};
    VkImageView imageView{VK_NULL_HANDLE};
    VkFormat format{VK_FORMAT_UNDEFINED};
    VkSampler sampler{VK_NULL_HANDLE};
    VkDeviceSize size{0};
};
}    // namespace Graphics
}    // namespace XRLib
