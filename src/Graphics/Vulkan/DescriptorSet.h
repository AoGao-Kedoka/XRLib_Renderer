#pragma once

#include "Logger.h"
#include "VkCore.h"
#include "Image.h"

namespace XRLib {
namespace Graphics {

struct DescriptorLayoutElement {

    std::variant<VkBuffer, Image> data;
    VkShaderStageFlags stage;
    VkDeviceSize size;
    VkDescriptorType GetType() {
        if (std::holds_alternative<VkBuffer>(data))
            return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        if (std::holds_alternative<Image>(data))
            return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    }
};

class DescriptorSet {
   public:
    DescriptorSet(std::shared_ptr<VkCore> core,
                  std::vector<DescriptorLayoutElement> layoutBindings);
    ~DescriptorSet();

    VkDescriptorSetLayout& GetDescriptorSetLayout() {
        return descriptorSetLayout;
    }

   private:
    std::shared_ptr<VkCore> core;
    std::vector<VkDescriptorSetLayoutBinding> bindings;
    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorSet descriptorSet;
};
}    // namespace Graphics
}    // namespace XRLib
