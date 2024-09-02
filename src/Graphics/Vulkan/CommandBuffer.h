#pragma once

#include "GraphicsRenderPass.h"
#include "VkCore.h"

namespace XRLib {
namespace Graphics {
class CommandBuffer {
   public:
    static void BeginSingleTimeCommands(std::shared_ptr<VkCore> core);
    static void EndSingleTimeCommands(std::shared_ptr<VkCore> core);
    static void Record(std::shared_ptr<VkCore> core,
                       std::shared_ptr<GraphicsRenderPass> pass,
                       uint32_t imageIndex);
};
}    // namespace Graphics
}    // namespace XRLib
