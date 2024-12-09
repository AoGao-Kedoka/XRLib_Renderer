#pragma once

#include "Logger.h"
#include <pch.h>

namespace XRLib {
class Util {
   public:
    template <typename T>
    static bool VectorContains(const std::vector<T>& vector, T t) {
        return std::find(vector.begin(), vector.end(), t) != vector.end();
    }

    static bool CheckPlatformSupport();

    static void EnsureDirExists(const std::filesystem::path& dirPath);
    static std::string ReadFile(const std::filesystem::path& filePath);
    static std::vector<uint32_t> ReadBinaryFile(const std::filesystem::path& filePath);
    static bool WriteFile(const std::filesystem::path& filePath, const std::vector<uint32_t>& data);
    static std::size_t HashString(const std::string& content);
    static std::filesystem::path ResolvePath(const std::filesystem::path& path);

    static std::vector<const char*> SplitStringToCharPtr(const std::string& input);

    static void ErrorPopup(const std::string& message);
};
}    // namespace XRLib
