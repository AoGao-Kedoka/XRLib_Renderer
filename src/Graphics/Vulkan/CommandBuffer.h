#pragma once

#include "VkGraphicsRenderpass.h"

namespace XRLib {
namespace Graphics {
class CommandBuffer {
   public:
    static std::unique_ptr<CommandBuffer> BeginSingleTimeCommands(VkCore& core);
    static void EndSingleTimeCommands(std::unique_ptr<CommandBuffer> commandBuffer);

   public:
    CommandBuffer(VkCore& core);
    ~CommandBuffer();

    CommandBuffer& BindVertexBuffer(int firstBinding, std::vector<VkBuffer> buffers, std::vector<VkDeviceSize> offsets);
    CommandBuffer& BindIndexBuffer(VkBuffer indexBuffer, VkDeviceSize offset,
                                   VkIndexType indexType = VK_INDEX_TYPE_UINT16);

    CommandBuffer& BindDescriptorSets(VkGraphicsRenderpass& pass, uint32_t firstSet, uint32_t dynamicOffsetCount = 0,
                                      const uint32_t* pDynamicOffsets = nullptr);
    CommandBuffer& StartRecord();
    CommandBuffer& StartPass(VkGraphicsRenderpass& pass, uint32_t imageIndex);
    CommandBuffer& EndPass();
    CommandBuffer& DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset,
                               uint32_t firstInstance);
    CommandBuffer& Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);
    CommandBuffer& PushConstant(VkGraphicsRenderpass& pass, uint32_t size, const void* ptr);
    void EndRecord(VkSubmitInfo* submitInfo, VkFence fence);
    void EndRecord();

    void BarrierBetweenPasses(uint32_t imageIndex, VkGraphicsRenderpass& pass);

    void PipelineBarrier(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
                         VkDependencyFlags dependencyFlags, uint32_t memoryBarrierCount,
                         const VkMemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount,
                         const VkBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount,
                         const VkImageMemoryBarrier* pImageMemoryBarriers);

    VkCommandBuffer& GetCommandBuffer() { return commandBuffer; }

   private:
    VkCore& core;
    VkGraphicsRenderpass* currentPass{nullptr};
    VkCommandBuffer commandBuffer{VK_NULL_HANDLE};
};
}    // namespace Graphics
}    // namespace XRLib
