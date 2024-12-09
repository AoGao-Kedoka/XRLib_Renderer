#include "Image.h"

#include "CommandBuffer.h"

namespace XRLib {
namespace Graphics {
Image::Image(VkCore& core, std::vector<uint8_t> textureData, const unsigned int width, const unsigned int height, const unsigned int channels,
             VkFormat format)
    : core{core}, format{format}, width(width), height{height}{
    size = width * height * channels;

    std::unique_ptr<Buffer> imageBuffer = std::make_unique<Buffer>(core, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                                                   static_cast<void*>(textureData.data()), false);

    CreateImage(width, height, format, VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    // command buffer: copy buffer to the image
    TransitionImageLayout(image, format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    CopyBufferToImage(imageBuffer->GetBuffer(), image, width, height);

    TransitionImageLayout(image, format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                          VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

Image::Image(VkCore& core, const unsigned int width, const unsigned int height, VkFormat format, VkImageTiling tiling,
             VkImageUsageFlags usage, VkMemoryPropertyFlags properties, uint32_t layerCount)
    : core{core}, format{format}, layerCount{layerCount} {
    CreateImage(width, height, format, tiling, usage, properties);
    resizable = true;
}

Image::Image(VkCore& core, VkImage image, VkFormat format, const unsigned int width, const unsigned int height, uint32_t layerCount)
    : core{core}, image{image}, format{format}, width{width}, height{height}, layerCount{layerCount} {}

void Image::Resize(unsigned int width, unsigned int height) {
    if (!resizable) {
        LOGGER(LOGGER::INFO) << "Image doesn't support resize";
        return;
    }

    ResetImage();
    CreateImage(width, height, format, tiling, usageFlags, propertyFlags);
}

Image::~Image() {
    VkUtil::VkSafeClean(vkFreeMemory, core.GetRenderDevice(), imageMemory, nullptr);
    VkUtil::VkSafeClean(vkDestroyImageView, core.GetRenderDevice(), imageView, nullptr);
    VkUtil::VkSafeClean(vkDestroySampler, core.GetRenderDevice(), sampler, nullptr);
}

void Image::ResetImage() {
    VkUtil::VkSafeClean(vkFreeMemory, core.GetRenderDevice(), imageMemory, nullptr);
    VkUtil::VkSafeClean(vkDestroyImageView, core.GetRenderDevice(), imageView, nullptr);
    VkUtil::VkSafeClean(vkDestroySampler, core.GetRenderDevice(), sampler, nullptr);
    imageView = VK_NULL_HANDLE;
    sampler = VK_NULL_HANDLE;
    imageMemory = VK_NULL_HANDLE;
}

VkImageView& Image::GetImageView(VkImageAspectFlags aspectFlags) {
    if (imageView == VK_NULL_HANDLE) {
        VkImageViewCreateInfo imageViewInfo{};
        imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewInfo.image = image;
        imageViewInfo.viewType = (layerCount == 1 ? VK_IMAGE_VIEW_TYPE_2D : VK_IMAGE_VIEW_TYPE_2D_ARRAY);
        imageViewInfo.format = format;
        imageViewInfo.subresourceRange.aspectMask = aspectFlags;
        imageViewInfo.subresourceRange.baseMipLevel = 0;
        imageViewInfo.subresourceRange.levelCount = 1;
        imageViewInfo.subresourceRange.baseArrayLayer = 0;
        imageViewInfo.subresourceRange.layerCount = layerCount;

        if (vkCreateImageView(core.GetRenderDevice(), &imageViewInfo, nullptr, &imageView) != VK_SUCCESS) {
            throw std::runtime_error("Faild to create image view");
        }
    }
    return imageView;
}

VkSampler& Image::GetSampler() {
    if (sampler == VK_NULL_HANDLE) {
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_TRUE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 0.0f;

        if (vkCreateSampler(core.GetRenderDevice(), &samplerInfo, nullptr, &sampler) != VK_SUCCESS) {
            Util::ErrorPopup("Failed to create sampler");
        }
    }
    return sampler;
}
void Image::CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
                        VkMemoryPropertyFlags properties) {
    StoreImageProperties(format, tiling, usage, properties);
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = layerCount;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;    // we don't use another queue family for the
                                                          // compute shader

    if (vkCreateImage(core.GetRenderDevice(), &imageInfo, nullptr, &image) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image!");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(core.GetRenderDevice(), image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = core.GetMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(core.GetRenderDevice(), &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate image memory!");
    }

    vkBindImageMemory(core.GetRenderDevice(), image, imageMemory, 0);

    this->height = height;
    this->width = width;
}

bool hasStencilComponent(VkFormat format) {
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

void Image::TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
    auto commandBuffer = CommandBuffer::BeginSingleTimeCommands(core);
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    }

    if (hasStencilComponent(format)) {
        barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
    } else {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_GENERAL) {
        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
               newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
               newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask =
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    } else {
        throw std::invalid_argument("Transition not found");
    }

    vkCmdPipelineBarrier(commandBuffer->GetCommandBuffer(), sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1,
                         &barrier);
    CommandBuffer::EndSingleTimeCommands(std::move(commandBuffer));
}
void Image::CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
    auto commandBuffer = CommandBuffer::BeginSingleTimeCommands(core);
    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = {0, 0, 0};
    region.imageExtent = {width, height, 1};

    vkCmdCopyBufferToImage(commandBuffer->GetCommandBuffer(), buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                           &region);

    CommandBuffer::EndSingleTimeCommands(std::move(commandBuffer));
}

void Image::StoreImageProperties(VkFormat format, VkImageTiling tiling, VkImageUsageFlags usageFlags,
                                 VkMemoryPropertyFlags property) {
    this->tiling = tiling;
    this->usageFlags = usageFlags;
    this->propertyFlags = property;
    this->layerCount = layerCount;
    this->format = format;
}

}    // namespace Graphics
}    // namespace XRLib
