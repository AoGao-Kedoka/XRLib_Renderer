#include "Shader.h"

Shader::Shader(std::shared_ptr<VkCore> core,
               const std::filesystem::path& filePath, ShaderStage shaderStage)
    : core{core}, stage{shaderStage} {
    std::string rawCode;
    if (filePath.empty()) {
        switch (stage) {
            case ShaderStage::VERTEX_SHADER:
                rawCode = Info::triangleVert;
                break;
            case ShaderStage::FRAGMENT_SHADER:
                rawCode = Info::triangleFrag;
                break;
        }
    } else {
        rawCode = Util::ReadFile(filePath.generic_string());
    }

    if (rawCode == "") {
        exit(-1);
    }

    auto spirv = Compile(rawCode, filePath.empty()
                                      ? "main"
                                      : filePath.filename().generic_string());
    Init(spirv);
}

Shader::~Shader() {
    if (!core)
        return;
    Util::VkSafeClean(vkDestroyShaderModule, core->GetRenderDevice(),
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
    shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(
        content, shader_kind, name.c_str(), options);

    if (module.GetCompilationStatus() != shaderc_compilation_status_success) {
        Util::ErrorPopup("failed to compile shader");
    }

    LOGGER(LOGGER::INFO) << "Shader compiled";

    return {module.cbegin(), module.cend()};
}
