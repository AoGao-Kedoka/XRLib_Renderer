#include "DescriptorSet.h"

namespace XRLib {
namespace Graphics {
DescriptorSet::DescriptorSet(
    std::shared_ptr<VkCore> core,
    std::vector<DescriptorLayoutElement> layoutBindings)
    : core{core} {
    bindings.resize(layoutBindings.size());
    for (int i = 0; i < layoutBindings.size(); ++i) {
        bindings[i].binding = i;
        bindings[i].descriptorType = layoutBindings[i].GetType();
        bindings[i].descriptorCount = 1;
        bindings[i].stageFlags = layoutBindings[i].stage;
        bindings[i].pImmutableSamplers = nullptr;
    }

    VkResult result;
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = bindings.size();
    layoutInfo.pBindings = bindings.data();
    if ((result = vkCreateDescriptorSetLayout(
             core->GetRenderDevice(), &layoutInfo, nullptr,
             &descriptorSetLayout)) != VK_SUCCESS) {
        Util::ErrorPopup("Error create descriptor set layout, Error: " +
                         result);
    }

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = core->GetDescriptorPool();
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &descriptorSetLayout;
    if ((result = vkAllocateDescriptorSets(core->GetRenderDevice(), &allocInfo,
                                           &descriptorSet)) != VK_SUCCESS) {
        Util::ErrorPopup("Failed to allocate descriptor set, Error: " + result);
    }

    std::vector<VkWriteDescriptorSet> descriptorWrites(layoutBindings.size());
    for (int i = 0; i < layoutBindings.size(); ++i) {
        descriptorWrites[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[i].dstSet = descriptorSet;
        descriptorWrites[i].dstBinding = i;
        descriptorWrites[i].dstArrayElement = 0;
        descriptorWrites[i].descriptorCount = 1;

        if (std::holds_alternative<VkBuffer>(layoutBindings[i].data)) {
            // write buffer
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = std::get<VkBuffer>(layoutBindings[i].data);
            bufferInfo.offset = 0;
            bufferInfo.range = layoutBindings[i].size;

            descriptorWrites[i].descriptorType = layoutBindings[i].GetType();
            descriptorWrites[i].pBufferInfo = &bufferInfo;

        } else if (std::holds_alternative<Image>(
                       layoutBindings[i].data)) {
            // write sampler
            Image image = std::get<Image>(layoutBindings[i].data);
            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageView = image.GetImageView();
            imageInfo.sampler = image.GetSampler();

            descriptorWrites[i].descriptorType = layoutBindings[i].GetType();
            descriptorWrites[i].pImageInfo = &imageInfo;
        }
    }

    vkUpdateDescriptorSets(core->GetRenderDevice(), descriptorWrites.size(),
                           descriptorWrites.data(), 0, nullptr);
}

DescriptorSet::~DescriptorSet() {
    vkDestroyDescriptorSetLayout(core->GetRenderDevice(), descriptorSetLayout,
                                 nullptr);
}

}    // namespace Graphics
}    // namespace XRLib
