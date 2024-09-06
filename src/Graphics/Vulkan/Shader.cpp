#include "Shader.h"

namespace XRLib {
namespace Graphics {
inline static const std::string defaultVert = R"(
#version 450
#extension GL_EXT_multiview : enable
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_debug_printf : enable

layout(set = 0,binding = 0) uniform ViewProj{
    mat4 view;
    mat4 proj;
} vp;

layout(set = 0,binding = 1) uniform ModelPos{
    mat4 model;
} m;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 fragNormal;
layout(location = 1) out vec2 fragTexCoord;

void main() {
    gl_Position = vp.proj * vp.view * m.model * vec4(inPosition, 1.0);
    fragNormal = inNormal;
    fragTexCoord = inTexCoord;
}
)";

inline static const std::string triangleVert = R"(
#version 450
#extension GL_EXT_multiview : enable
#extension GL_EXT_debug_printf : enable

vec2 positions[3] = vec2[](
    vec2(0.0, -0.5),
    vec2(0.5, 0.5),
    vec2(-0.5, 0.5)
);

void main() {
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
    debugPrintfEXT("gl_Position is %1.2v4f", gl_Position);
}
)";

inline static const std::string defaultFrag = R"(
#version 450
#extension GL_EXT_multiview : enable
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 2) uniform sampler2D texSampler;

layout(location = 0) in vec3 fragNormal;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = texture(texSampler, fragTexCoord);
}
)";

Shader::Shader(std::shared_ptr<VkCore> core,
               const std::filesystem::path& filePath, ShaderStage shaderStage)
    : core{core}, stage{shaderStage} {
    std::string rawCode;
    if (filePath.empty()) {
        switch (stage) {
            case ShaderStage::VERTEX_SHADER:
                rawCode = defaultVert;
                break;
            case ShaderStage::FRAGMENT_SHADER:
                rawCode = defaultFrag;
                break;
        }
    } else {
        rawCode = Util::ReadFile(filePath.generic_string());
    }

    if (rawCode == "") {
        Util::ErrorPopup("Shader file content is empty");
    }

    auto spirvFuture = std::async(
        std::launch::async, &Shader::Compile, this, rawCode,
        filePath.empty() ? "main" : filePath.filename().generic_string());
    Init(spirvFuture.get());
}

Shader::~Shader() {
    if (!core)
        return;
    VkUtil::VkSafeClean(vkDestroyShaderModule, core->GetRenderDevice(),
                        this->shaderModule, nullptr);
}

void Shader::Init(std::vector<uint32_t> spirv) {
    //create shader module
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = spirv.size() * sizeof(unsigned int);
    createInfo.pCode = spirv.data();
    if (vkCreateShaderModule(core->GetRenderDevice(), &createInfo, nullptr,
                             &this->shaderModule)) {
        Util::ErrorPopup("Failed to create shader module");
    }

    // create pipeline shader stage
    this->shaderStageInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    this->shaderStageInfo.stage =
        static_cast<VkShaderStageFlagBits>(this->stage);
    this->shaderStageInfo.module = this->shaderModule;
    this->shaderStageInfo.pName = "main";
}

std::vector<uint32_t> Shader::Compile(std::string content, std::string name) {
    shaderc::Compiler compiler;
    shaderc::CompileOptions options;

    options.SetOptimizationLevel(shaderc_optimization_level_performance);

    shaderc_shader_kind shader_kind{shaderc_glsl_vertex_shader};
    switch (this->stage) {
        case ShaderStage::VERTEX_SHADER:
            shader_kind = shaderc_glsl_vertex_shader;
            break;
        case ShaderStage::FRAGMENT_SHADER:
            shader_kind = shaderc_glsl_fragment_shader;
            break;
    }
    shaderc::SpvCompilationResult module =
        compiler.CompileGlslToSpv(content, shader_kind, name.c_str(), options);

    if (module.GetCompilationStatus() != shaderc_compilation_status_success) {
        Util::ErrorPopup("failed to compile shader");
    }

    LOGGER(LOGGER::INFO) << "Shader compiled";

    return {module.cbegin(), module.cend()};
}
}    // namespace Graphics
}    // namespace XRLib
