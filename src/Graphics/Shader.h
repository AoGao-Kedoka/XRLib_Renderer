#pragma once

#include "Core.h"
#include "Util.h"
#include "Logger.h"

class Shader {
   public:
    Shader(Core* core,const std::string& file_path);
    ~Shader();

    VkShaderModule GetShaderModule() { return shaderModule; };

   private:
    Core* core = nullptr;
    VkShaderModule shaderModule{VK_NULL_HANDLE};

};