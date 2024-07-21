#pragma once

#include <array>
#include <vulkan/vulkan.h>

#include "../Primitives.h"

class VkUtil {
   public:
    static VkVertexInputBindingDescription GetVertexBindingDescription();

    static std::array<VkVertexInputAttributeDescription, 3>
    GetVertexAttributeDescription();
};