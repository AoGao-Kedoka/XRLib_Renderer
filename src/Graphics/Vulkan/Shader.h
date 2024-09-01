#pragma once

#include "Logger.h"
#include "Utils/Util.h"
#include "Utils/Info.h"
#include "VkCore.h"

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
};
