#pragma once

#include "GraphicsRenderPass.h"

namespace XRLib {
namespace Graphics {
class CommandBuffer {
   public:
    static std::unique_ptr<CommandBuffer> BeginSingleTimeCommands(std::shared_ptr<VkCore> core);
    static void EndSingleTimeCommands(std::unique_ptr<CommandBuffer> commandBuffer);

   public:
    CommandBuffer(std::shared_ptr<VkCore> core);
    ~CommandBuffer();

    CommandBuffer& BindVertexBuffer(int firstBinding, std::vector<VkBuffer> buffers, std::vector<VkDeviceSize> offsets);
    CommandBuffer& BindIndexBuffer(VkBuffer indexBuffer, VkDeviceSize offset,
                                   VkIndexType indexType = VK_INDEX_TYPE_UINT16);

    CommandBuffer& BindDescriptorSets(GraphicsRenderPass& pass, uint32_t firstSet,
                                      uint32_t dynamicOffsetCount = 0, const uint32_t* pDynamicOffsets = nullptr);
    CommandBuffer& StartRecord();
    CommandBuffer& StartPass(GraphicsRenderPass& pass, uint32_t imageIndex);
    CommandBuffer& EndPass();
    CommandBuffer& DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset,
                               uint32_t firstInstance);
    CommandBuffer& Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);
    CommandBuffer& PushConstant(GraphicsRenderPass& pass, uint32_t size,
                                               const void* ptr);
    void EndRecord(VkSubmitInfo* submitInfo, VkFence fence);

    VkCommandBuffer& GetCommandBuffer() { return commandBuffer; }

   private:
    std::shared_ptr<VkCore> core;
    GraphicsRenderPass* currentPass{nullptr};
    VkCommandBuffer commandBuffer{VK_NULL_HANDLE};
};
}    // namespace Graphics
}    // namespace XRLib
