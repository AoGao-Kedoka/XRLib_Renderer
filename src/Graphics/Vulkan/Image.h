#pragma once

#include "Logger.h"
#include "VkCore.h"

namespace XRLib {
namespace Graphics {
class Image {
   public:
    Image() = default;
    Image(VkCore* core, VkImage image, VkFormat format);

    VkSampler& GetSampler() { return sampler; }
    VkImageView& GetImageView() { return imageView; }

   private:
    VkCore* core;
    VkImage image{VK_NULL_HANDLE};
    VkImageView imageView{VK_NULL_HANDLE};
    VkFormat format{VK_FORMAT_UNDEFINED};
    VkSampler sampler{VK_NULL_HANDLE};
};
}    // namespace Graphics
}    // namespace XRLib
