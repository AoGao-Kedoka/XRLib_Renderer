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
    Shader(VkCore& core, const std::filesystem::path& file_path, ShaderStage stage, bool stereo);
    ~Shader();

    VkShaderModule GetShaderModule() const { return shaderModule; };
    VkPipelineShaderStageCreateInfo GetShaderStageInfo() const { return shaderStageInfo; }

   private:
    void Init(std::vector<uint32_t> spirv);
    std::vector<uint32_t> Compile(std::string content, std::string name);

   private:
    VkCore& core;
    ShaderStage stage;
    VkShaderModule shaderModule{VK_NULL_HANDLE};
    VkPipelineShaderStageCreateInfo shaderStageInfo{};
    std::string shaderCacheDir = "./ShaderCache";
};
}    // namespace Graphics
}    // namespace XRLib
