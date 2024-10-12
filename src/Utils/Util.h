#pragma once

#include "Logger.h"
#include <pch.h>

class Util {
   public:
    template <typename T>
    static bool VectorContains(const std::vector<T>& vector, T t) {
        return std::find(vector.begin(), vector.end(), t) != vector.end();
    }

    static bool CheckPlatformSupport();

    static std::string ReadFile(std::string file_path);

    static std::filesystem::path ResolvePath(const std::filesystem::path& path);

    static std::vector<const char*> SplitStringToCharPtr(const std::string& input);

    static void ErrorPopup(std::string&& message);
};
