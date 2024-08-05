#pragma once

#include "GraphicsRenderPass.h"
#include "VkCore.h"

class CommandBuffer {
public:
  static void BeginSingleTimeCommands(std::shared_ptr<VkCore> core);
  static void EndSingleTimeCommands(std::shared_ptr<VkCore> core);
  static void Record(std::shared_ptr<VkCore> core,
                     std::shared_ptr<GraphicsRenderPass> pass,
                     uint32_t imageIndex);
};