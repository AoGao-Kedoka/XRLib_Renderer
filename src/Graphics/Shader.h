#pragma once

#include "Core.h"
#include "Util.h"
#include "Logger.h"


class Shader {
   public:
    enum ShaderStage {
        VERTEX_SHADER = VK_SHADER_STAGE_VERTEX_BIT,
        FRAGMENT_SHADER = VK_SHADER_STAGE_FRAGMENT_BIT,
        // possibly more
    };

    Shader(Core* core,const std::string& file_path, ShaderStage stage);
    ~Shader();
    
    VkShaderModule GetShaderModule() const { return shaderModule; };
    VkPipelineShaderStageCreateInfo GetShaderStageInfo() const {
        return shaderStageInfo;
    }

   private:
    Core* core = nullptr;
    VkShaderModule shaderModule{VK_NULL_HANDLE};
    VkPipelineShaderStageCreateInfo shaderStageInfo{};
};