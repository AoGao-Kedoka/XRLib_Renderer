#pragma once

#include <cstring>
#include <filesystem>

#include <vulkan/vulkan.h>
#define XR_USE_GRAPHICS_API_VULKAN
#include "Logger.h"
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>

class Util {
   public:
    static bool CheckPlatformSupport();

    static std::string ReadFile(std::string file_path);

    static std::filesystem::path ResolvePath(const std::filesystem::path& path);

    static std::vector<const char*>
    SplitStringToCharPtr(const std::string& input);

    static void ErrorPopup(std::string message);
};
