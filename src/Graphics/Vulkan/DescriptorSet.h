#pragma once

#include "Buffer.h"
#include "Image.h"

namespace XRLib {
namespace Graphics {

struct DescriptorLayoutElement {

    std::variant<std::shared_ptr<Buffer>, std::vector<std::shared_ptr<Image>>>
        data;    //buffer or images can be shared to multiple descriptors
    VkShaderStageFlags stage = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorType GetType() const {
        if (auto buffer = std::get_if<std::shared_ptr<Buffer>>(&data)) {
            if ((*buffer)->IsUniformBuffer())
                return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            if ((*buffer)->IsStorageBuffer())
                return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        }

        if (auto image = std::get_if<std::vector<std::shared_ptr<Image>>>(&data)) {
            return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        }
        return VK_DESCRIPTOR_TYPE_MAX_ENUM;
    }
};

class DescriptorSet {
   public:
    DescriptorSet(VkCore& core, std::vector<DescriptorLayoutElement>& layoutBindings);

    template <typename... Args>
    DescriptorSet(VkCore& core, Args&&... args) : core{core} {
        (PushElement(std::forward<Args>(args)), ...);
        Init();
    }

    ~DescriptorSet();

    VkDescriptorSetLayout& GetDescriptorSetLayout() { return descriptorSetLayout; }

    VkDescriptorSet& GetVkDescriptorSet() { return descriptorSet; }

    void AllocatePushConstant(uint32_t size) { pushConstantSize = size; };
    uint32_t GetPushConstantSize() { return pushConstantSize; }

   private:
    void Init();

    template <typename T>
    void PushElement(T&& arg) {
        if constexpr (std::is_same_v<std::remove_reference_t<T>, std::shared_ptr<Buffer>>) {
            elements.push_back(DescriptorLayoutElement{arg});
        } else if constexpr (std::is_same_v<std::remove_reference_t<T>, std::vector<std::shared_ptr<Image>>>) {
            elements.push_back(DescriptorLayoutElement{arg});
        } else {
            static_assert(always_false<T>::value, "Invalid argument type for DescriptorSet constructor");
        }
    }

    // needed for cl
    template <typename T>
    struct always_false : std::false_type {};

   private:
    VkCore& core;
    std::vector<VkDescriptorSetLayoutBinding> bindings;
    std::vector<DescriptorLayoutElement> elements;
    uint32_t pushConstantSize{0};
    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorSet descriptorSet;
};
}    // namespace Graphics
}    // namespace XRLib
