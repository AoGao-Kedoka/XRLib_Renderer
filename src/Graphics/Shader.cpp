#include "Shader.h"

Shader::Shader(Core* core, const std::string& file_path) : core{core} {
    auto code = Util::ReadFile(file_path);
    VkShaderModule shaderModule;
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
    if (vkCreateShaderModule(core->GetRenderDevice(), &createInfo, nullptr,
        &shaderModule)) {
        LOGGER(LOGGER::ERR) << "Failed to create shader module";
        exit(-1);
    }
    
    this->shaderModule = shaderModule;
}

Shader::~Shader() {
    Util::VkSafeClean(vkDestroyShaderModule, core->GetRenderDevice(),
                      this->shaderModule, nullptr);
}
