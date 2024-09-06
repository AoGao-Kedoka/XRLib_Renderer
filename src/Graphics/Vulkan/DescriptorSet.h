#pragma once

#include "Buffer.h"
#include "Image.h"
#include "Logger.h"
#include "VkCore.h"

namespace XRLib {
namespace Graphics {

struct DescriptorLayoutElement {

    std::variant<std::shared_ptr<Buffer>, std::shared_ptr<Image>> data;
    VkShaderStageFlags stage =
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorType GetType() {
        if (std::holds_alternative<std::shared_ptr<Buffer>>(data)) {
            return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        }
        if (std::holds_alternative<std::shared_ptr<Image>>(data)) {
            return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        }
        return VK_DESCRIPTOR_TYPE_MAX_ENUM;
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

    VkDescriptorSet& GetVkDescriptorSet() { return descriptorSet; }

   private:
    std::shared_ptr<VkCore> core;
    std::vector<VkDescriptorSetLayoutBinding> bindings;
    std::vector<DescriptorLayoutElement> elements;
    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorSet descriptorSet;
};
}    // namespace Graphics
}    // namespace XRLib
