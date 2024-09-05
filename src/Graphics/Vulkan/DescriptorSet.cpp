#include "DescriptorSet.h"

namespace XRLib {
namespace Graphics {
DescriptorSet::DescriptorSet(
    std::shared_ptr<VkCore> core,
    std::vector<DescriptorLayoutElement> layoutBindings)
    : core{core}, elements{layoutBindings} {
    bindings.resize(elements.size());
    for (int i = 0; i < elements.size(); ++i) {
        bindings[i].binding = i;
        bindings[i].descriptorType = elements[i].GetType();
        bindings[i].descriptorCount = 1;
        bindings[i].stageFlags = elements[i].stage;
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

    std::vector<VkWriteDescriptorSet> descriptorWrites(elements.size());
    for (int i = 0; i < elements.size(); ++i) {
        descriptorWrites[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[i].dstSet = descriptorSet;
        descriptorWrites[i].dstBinding = i;
        descriptorWrites[i].dstArrayElement = 0;
        descriptorWrites[i].descriptorCount = 1;

        if (auto bufferPtr =
                std::get_if<std::shared_ptr<Buffer>>(&elements[i].data)) {
            // write buffer
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = (*bufferPtr)->GetBuffer();
            bufferInfo.offset = 0;
            bufferInfo.range = elements[i].size;

            descriptorWrites[i].descriptorType = elements[i].GetType();
            descriptorWrites[i].pBufferInfo = &bufferInfo;

        } else if (auto imagePtr =
                       std::get_if<std::shared_ptr<Image>>(&elements[i].data)) {
            // write sampler
            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageView = (*imagePtr)->GetImageView();
            imageInfo.sampler = (*imagePtr)->GetSampler();
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            descriptorWrites[i].descriptorType = elements[i].GetType();
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
