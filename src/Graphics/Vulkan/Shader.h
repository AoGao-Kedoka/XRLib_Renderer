#pragma once

#include "Logger.h"
#include "Utils/Info.h"
#include "Utils/Util.h"
#include "VkCore.h"

namespace XRLib {
namespace Graphics {
class Shader {
   public:
    enum ShaderStage {
        VERTEX_SHADER = VK_SHADER_STAGE_VERTEX_BIT,
        FRAGMENT_SHADER = VK_SHADER_STAGE_FRAGMENT_BIT,
        // possibly more
    };
    Shader(std::shared_ptr<VkCore> core, const std::filesystem::path& file_path,
           ShaderStage stage);
    ~Shader();

    Shader(Shader&& other) noexcept
        : core(std::exchange(other.core, nullptr)),
          stage(std::exchange(other.stage, static_cast<ShaderStage>(0))),
          shaderModule(std::exchange(other.shaderModule, VK_NULL_HANDLE)),
          shaderStageInfo(std::exchange(other.shaderStageInfo, {})) {}

    Shader& operator=(Shader&& other) noexcept {
        if (this != &other) {
            core = std::exchange(other.core, nullptr);
            stage = std::exchange(other.stage, static_cast<ShaderStage>(0));
            shaderModule = std::exchange(other.shaderModule, VK_NULL_HANDLE);
            shaderStageInfo = std::exchange(other.shaderStageInfo, {});
        }
        return *this;
    }

    VkShaderModule GetShaderModule() const { return shaderModule; };
    VkPipelineShaderStageCreateInfo GetShaderStageInfo() const {
        return shaderStageInfo;
    }

   private:
    std::shared_ptr<VkCore> core{nullptr};
    ShaderStage stage;
    VkShaderModule shaderModule{VK_NULL_HANDLE};
    VkPipelineShaderStageCreateInfo shaderStageInfo{};

    void Init(std::vector<uint32_t> spirv);
    std::vector<uint32_t> Compile(std::string content, std::string name);

    inline static std::string defaultVert{
        "#version 450\n"
        "#extension GL_EXT_multiview : enable\n"
        "#extension GL_ARB_separate_shader_objects : enable\n"
        "\n"
        "layout(set=0,binding = 0) uniform UniformBufferObject {\n"
        "        mat4 model;\n"
        "        mat4 view;\n"
        "        mat4 proj;\n"
        "} ubo;\n"
        "\n"
        "layout(location = 0) in vec3 inPosition;\n"
        "layout(location = 1) in vec3 inNormal;\n"
        "layout(location = 2) in vec2 inTexCoord;\n"
        "\n"
        "layout(location = 0) out vec3 fragNormal;\n"
        "layout(location = 1) out vec2 fragTexCoord;\n"
        "\n"
        "void main() {\n"
        "    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, "
        "1.0);\n"
        "    fragNormal = inNormal;\n"
        "    fragTexCoord = inTexCoord;\n"
        "}\n"};

    inline static std::string defaultFrag{
        "#version 450\n"
        "#extension GL_ARB_separate_shader_objects : enable\n"
        "\n"
        "layout(location = 0) out vec4 outColor;\n"
        "\n"
        "void main() {\n"
        "    outColor = vec4(1.0, 0.0, 0.0, 1.0);\n"
        "}\n"};
};
}    // namespace Graphics
}    // namespace XRLib
