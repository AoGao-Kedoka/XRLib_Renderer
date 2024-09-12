#include "DescriptorSet.h"

namespace XRLib {
namespace Graphics {
DescriptorSet::DescriptorSet(std::shared_ptr<VkCore> core, std::vector<DescriptorLayoutElement> layoutBindings)
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
    if ((result = vkCreateDescriptorSetLayout(core->GetRenderDevice(), &layoutInfo, nullptr, &descriptorSetLayout)) !=
        VK_SUCCESS) {
        Util::ErrorPopup("Error create descriptor set layout");
    }

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = core->GetDescriptorPool();
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &descriptorSetLayout;
    if ((result = vkAllocateDescriptorSets(core->GetRenderDevice(), &allocInfo, &descriptorSet)) != VK_SUCCESS) {
        Util::ErrorPopup("Failed to allocate descriptor set");
    }

    // Create storage for buffer and image infos
    std::vector<VkDescriptorBufferInfo> bufferInfos(elements.size());
    std::vector<VkDescriptorImageInfo> imageInfos(elements.size());

    std::vector<VkWriteDescriptorSet> descriptorWrites(elements.size());
    for (int i = 0; i < elements.size(); ++i) {
        descriptorWrites[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[i].dstSet = descriptorSet;
        descriptorWrites[i].dstBinding = i;
        descriptorWrites[i].dstArrayElement = 0;
        descriptorWrites[i].descriptorCount = 1;
        descriptorWrites[i].descriptorType = elements[i].GetType();

        if (auto bufferPtr = std::get_if<std::shared_ptr<Buffer>>(&elements[i].data)) {
            // Store buffer info in bufferInfos vector
            bufferInfos[i].buffer = (*bufferPtr)->GetBuffer();
            bufferInfos[i].offset = 0;
            bufferInfos[i].range = (*bufferPtr)->GetSize();

            descriptorWrites[i].pBufferInfo = &bufferInfos[i];

        } else if (auto imagePtr = std::get_if<std::shared_ptr<Image>>(&elements[i].data)) {
            // Store image info in imageInfos vector
            imageInfos[i].imageView = (*imagePtr)->GetImageView();
            imageInfos[i].sampler = (*imagePtr)->GetSampler();
            imageInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            descriptorWrites[i].pImageInfo = &imageInfos[i];
        }
    }

    vkUpdateDescriptorSets(core->GetRenderDevice(), descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
}

DescriptorSet::~DescriptorSet() {
    vkDestroyDescriptorSetLayout(core->GetRenderDevice(), descriptorSetLayout, nullptr);
}
}    // namespace Graphics
}    // namespace XRLib
