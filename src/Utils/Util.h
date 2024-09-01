#pragma once

#include <pch.h>
#include "Logger.h"

class Util {
   public:
    static bool CheckPlatformSupport();

    static std::string ReadFile(std::string file_path);

    static std::filesystem::path ResolvePath(const std::filesystem::path& path);

    static std::vector<const char*>
    SplitStringToCharPtr(const std::string& input);

    static void ErrorPopup(std::string message);
};
