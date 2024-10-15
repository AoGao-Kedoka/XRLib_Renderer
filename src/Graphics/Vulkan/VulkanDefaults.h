#pragma once
#include "Graphics/Vulkan/Buffer.h"
#include "Graphics/Vulkan/GraphicsRenderPass.h"
#include "Graphics/Vulkan/VkCore.h"
#include "Scene.h"

namespace XRLib {
namespace Graphics {
class VulkanDefaults {
   public:
    ////////////////////////////////////////////////////
    // Shaders
    ////////////////////////////////////////////////////
    inline static const std::string defaultVertFlat = R"(
    #version 450
    #extension GL_ARB_separate_shader_objects : enable
    layout(set = 0, binding = 0) uniform ViewProj{
        mat4 view;
        mat4 proj;
    } vp;

    layout(set = 0, binding = 1) readonly buffer ModelMatrices {
        mat4 models[];
    };

    layout(push_constant) uniform PushConstants {
        uint modelIndex;
    };

    layout(location = 0) in vec3 inPosition;
    layout(location = 1) in vec3 inNormal;
    layout(location = 2) in vec2 inTexCoord;

    layout(location = 0) out vec3 fragNormal;
    layout(location = 1) out vec2 fragTexCoord;

    void main() {
        gl_Position = vp.proj * vp.view * models[modelIndex] * vec4(inPosition, 1.0);
        fragNormal = inNormal;
        fragTexCoord = inTexCoord;
    }
    )";

    inline static const std::string defaultFrag = R"(
    #version 450
    #extension GL_ARB_separate_shader_objects : enable
    #extension GL_EXT_multiview : enable
    #extension GL_EXT_nonuniform_qualifier: enable

    layout(set = 0, binding = 2) uniform sampler2D texSamplers[];

    layout(push_constant) uniform PushConstants {
        uint modelIndex;
    };

    layout(location = 0) in vec3 fragNormal;
    layout(location = 1) in vec2 fragTexCoord;

    layout(location = 0) out vec4 outColor;

    void main() {
        outColor = texture(texSamplers[modelIndex], fragTexCoord);
    }
    )";

    inline static const std::string defaultVertStereo = R"(
    #version 450
    #extension GL_EXT_multiview : enable
    #extension GL_ARB_separate_shader_objects : enable

    layout(set = 0,binding = 0) uniform ViewProj{
        mat4 view[2];
        mat4 proj[2];
    } vp;

    layout(set = 0, binding = 1) readonly buffer ModelMatrices {
        mat4 models[];
    };

    layout(push_constant) uniform PushConstants {
        uint modelIndex;
    };

    layout(location = 0) in vec3 inPosition;
    layout(location = 1) in vec3 inNormal;
    layout(location = 2) in vec2 inTexCoord;

    layout(location = 0) out vec3 fragNormal;
    layout(location = 1) out vec2 fragTexCoord;

    void main() {
        gl_Position = vp.proj[gl_ViewIndex] * vp.view[gl_ViewIndex] * models[modelIndex] * vec4(inPosition, 1.0);
        fragNormal = inNormal;
        fragTexCoord = inTexCoord;
    }
    )";

    ////////////////////////////////////////////////////
    // Default render passes
    ////////////////////////////////////////////////////
    static void PrepareDefaultStereoRenderPasses(std::shared_ptr<VkCore> core, std::shared_ptr<Scene> scene,
                                                 Primitives::ViewProjectionStereo& viewProj,
                                                 std::vector<std::unique_ptr<GraphicsRenderPass>>& renderPasses);
    static void PrepareDefaultFlatRenderPasses(std::shared_ptr<VkCore> core, std::shared_ptr<Scene> scene,
                                                 Primitives::ViewProjection& viewProj,
                                                 std::vector<std::unique_ptr<GraphicsRenderPass>>& renderPasses);

};
}    // namespace Graphics
}    // namespace XRLib
