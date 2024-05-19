#include "Shader.h"

Shader::Shader(Core* core, const std::string& file_path, ShaderStage shaderStage) : core{core} {
    auto code = Util::ReadFile(file_path);
    if (code == "") {
        exit(-1);
    }

    //create shader module
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
    if (vkCreateShaderModule(core->GetRenderDevice(), &createInfo, nullptr,
        &this->shaderModule)) {
        LOGGER(LOGGER::ERR) << "Failed to create shader module";
        exit(-1);
    }
    
    // create pipeline shader stage
    this->shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    this->shaderStageInfo.stage = static_cast<VkShaderStageFlagBits>(shaderStage);
    this->shaderStageInfo.module = this->shaderModule;
    this->shaderStageInfo.pName = "main";
}

Shader::~Shader() {
    Util::VkSafeClean(vkDestroyShaderModule, core->GetRenderDevice(),
                      this->shaderModule, nullptr);
}
