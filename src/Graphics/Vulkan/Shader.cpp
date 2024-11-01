#include "Shader.h"
#include "VulkanDefaults.h"

namespace XRLib {
namespace Graphics {

Shader::Shader(std::shared_ptr<VkCore> core, const std::filesystem::path& filePath, ShaderStage shaderStage,
               bool stereo)
    : core{core}, stage{shaderStage} {
    Util::EnsureDirExists(shaderCacheDir);
    std::string rawCode;
    if (filePath.empty()) {
        switch (stage) {
            case ShaderStage::VERTEX_SHADER:
                rawCode = stereo ? VulkanDefaults::defaultVertStereo : VulkanDefaults::defaultVertFlat;
                break;
            case ShaderStage::FRAGMENT_SHADER:
                rawCode = VulkanDefaults::defaultPhongFrag;
                break;
        }
    } else {
        rawCode = Util::ReadFile(filePath.generic_string());
    }

    std::string cacheNamePrefix = filePath.empty() ? "defaultMain" : filePath.filename().generic_string();
    std::string cacheNameSuffix = std::to_string(Util::HashString(rawCode));

#ifdef __cpp_lib_format
    std::string cacheFilePath = std::format("{}/{}_{}.spv", shaderCacheDir, cacheNamePrefix, cacheNameSuffix);
#else
    std::string cacheFilePath = fmt::format("{}/{}_{}.spv", shaderCacheDir, cacheNamePrefix, cacheNameSuffix);
#endif

    std::vector<uint32_t> code;
    if (std::filesystem::exists(cacheFilePath)) {
        code = Util::ReadBinaryFile(cacheFilePath);
        LOGGER(LOGGER::INFO) << "Loaded shader from cache: " << cacheFilePath;
    } else {

        if (rawCode.empty()) {
            Util::ErrorPopup("Shader file content is empty");
        }

        code = Compile(rawCode, cacheNamePrefix);
        Util::WriteFile(cacheFilePath, code);
        LOGGER(LOGGER::INFO) << "Compiled and cached shader: " << cacheFilePath;
    }
    Init(code);
}

Shader::~Shader() {
    if (!core)
        return;
    VkUtil::VkSafeClean(vkDestroyShaderModule, core->GetRenderDevice(), this->shaderModule, nullptr);
}

void Shader::Init(std::vector<uint32_t> spirv) {
    // create shader module
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = spirv.size() * sizeof(unsigned int);
    createInfo.pCode = spirv.data();
    if (vkCreateShaderModule(core->GetRenderDevice(), &createInfo, nullptr, &this->shaderModule)) {
        Util::ErrorPopup("Failed to create shader module");
    }

    // create pipeline shader stage
    this->shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    this->shaderStageInfo.stage = static_cast<VkShaderStageFlagBits>(this->stage);
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
    shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(content, shader_kind, name.c_str(), options);

    if (module.GetCompilationStatus() != shaderc_compilation_status_success) {
        Util::ErrorPopup("Failed to compile shader: " + std::string(module.GetErrorMessage()));
    }

    return {module.cbegin(), module.cend()};
}
}    // namespace Graphics
}    // namespace XRLib
