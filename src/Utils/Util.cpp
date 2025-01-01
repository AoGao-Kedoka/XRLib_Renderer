#include "Util.h"
#include "boxer.h"

namespace XRLib {
const char* getCurrentPlatform() {
#if defined(_WIN32)
#define PLATFORM_NAME "windows"
#elif defined(_WIN64)
#define PLATFORM_NAME "windows"
#elif defined(__CYGWIN__) && !defined(_WIN32)
#define PLATFORM_NAME "windows"    // Windows (Cygwin POSIX under Microsoft Window)
#elif defined(__ANDROID__)
#define PLATFORM_NAME "android"    // Android (implies Linux, so it must come first)
#elif defined(__linux__)
#define PLATFORM_NAME "linux"    // Debian, Ubuntu, Gentoo, Fedora, openSUSE, RedHat, Centos and other
#elif defined(__unix__) || !defined(__APPLE__) && defined(__MACH__)
#include <sys/param.h>
#if defined(BSD)
#define PLATFORM_NAME "bsd"    // FreeBSD, NetBSD, OpenBSD, DragonFly BSD
#endif
#elif defined(__hpux)
#define PLATFORM_NAME "hp-ux"    // HP-UX
#elif defined(_AIX)
#define PLATFORM_NAME "aix"                      // IBM AIX
#elif defined(__APPLE__) && defined(__MACH__)    // Apple OSX and iOS (Darwin)
#include <TargetConditionals.h>
#if TARGET_IPHONE_SIMULATOR == 1
#define PLATFORM_NAME "ios"    // Apple iOS
#elif TARGET_OS_IPHONE == 1
#define PLATFORM_NAME "ios"    // Apple iOS
#elif TARGET_OS_MAC == 1
#define PLATFORM_NAME "osx"    // Apple OSX
#endif
#elif defined(__sun) && defined(__SVR4)
#define PLATFORM_NAME "solaris"    // Oracle Solaris, Open Indiana
#else
#define PLATFORM_NAME NULL
#endif
    return (PLATFORM_NAME == nullptr) ? "" : PLATFORM_NAME;
}

bool Util::CheckPlatformSupport() {
    auto platform = getCurrentPlatform();
    LOGGER(LOGGER::INFO) << "Current platform " << platform;
    if (std::strcmp("windows", platform) == 0)
        return true;
    if (std::strcmp("linux", platform) == 0) {
        LOGGER(LOGGER::WARNING) << "Linux is not fully tested yet.";
        return true;
    }
    if (std::strcmp("osx", platform) == 0) {
        std::string message{"OSX is currently very buggy, use at your own risk!"};
        LOGGER(LOGGER::WARNING) << message;
        boxer::show(message.c_str(), "WARNING", boxer::Style::Warning);
        return true;
    }
    LOGGER(LOGGER::ERR) << "Unsuppored platform";
    return false;
}

static std::filesystem::path GetUserHomeDirectory() {
#ifdef _WIN32
    const char* userProfile = getenv("USERPROFILE");
    if (userProfile) {
        return std::filesystem::path(userProfile);
    } else {
        throw std::runtime_error("Failed to retrieve user's home directory.");
    }
#else
    const char* homeDir = getenv("HOME");
    if (homeDir) {
        return std::filesystem::path(homeDir);
    } else {
        throw std::runtime_error("Failed to retrieve user's home directory.");
    }
#endif
}

std::vector<const char*> Util::SplitStringToCharPtr(const std::string& input) {
    static std::vector<std::string> managed_strings;
    managed_strings.clear();
    std::vector<const char*> out;
    std::istringstream stream(input);
    std::string extension;

    while (getline(stream, extension, ' ')) {
        managed_strings.push_back(extension);
    }

    for (const auto& str : managed_strings) {
        out.push_back(str.c_str());
    }

    return out;
}

void Util::ErrorPopup(const std::string& message) {
    LOGGER(LOGGER::ERR) << message;
    boxer::show(message.c_str(), "ERROR", boxer::Style::Error);
    throw std::runtime_error(message.c_str());
}

void Util::EnsureDirExists(const std::filesystem::path& dirPath) {
    if (!std::filesystem::exists(dirPath)) {
        if (std::filesystem::create_directories(dirPath)) {
            LOGGER(LOGGER::INFO) << "Directory created: " << dirPath;
        } else {
            ErrorPopup(FORMAT_STRING("Directory creation failed: {}", dirPath.generic_string()));
        }
    }
}

std::string Util::ReadFile(const std::filesystem::path& filePath) {
    std::ifstream t(filePath);
    std::string result = std::string((std::istreambuf_iterator<char>(t)), (std::istreambuf_iterator<char>()));
    if (result == "") {
        LOGGER(LOGGER::ERR) << "File is empty";
        return result;
    }
    LOGGER(LOGGER::DEBUG) << result;
    return result;
}

std::vector<uint32_t> Util::ReadBinaryFile(const std::filesystem::path& filePath) {
    std::ifstream file(filePath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        ErrorPopup("Error opening file");
        return {};
    }

    std::ifstream::pos_type fileSize = file.tellg();
    std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));

    file.seekg(0, std::ios::beg);
    file.read(reinterpret_cast<char*>(buffer.data()), fileSize);

    if (!file.good()) {
        ErrorPopup("Error reading file");
        return {};
    }

    file.close();
    return buffer;
}

bool Util::WriteFile(const std::filesystem::path& filePath, const std::vector<uint32_t>& data) {
    std::ofstream outFile(filePath, std::ios::binary);
    if (!outFile.is_open()) {
        ErrorPopup(FORMAT_STRING("Error: Cannot open file {} for writing", filePath.generic_string()));
        return false;
    }

    outFile.write(reinterpret_cast<const char*>(data.data()), data.size() * sizeof(uint32_t));
    if (!outFile.good()) {
        ErrorPopup(FORMAT_STRING("Error writing to file: {}", filePath.generic_string()));
        return false;
    }

    outFile.close();
    return true;
}

std::string Util::GetFileNameWithoutExtension(const std::filesystem::path& filePath) {
    return filePath.stem().generic_string();
}
std::size_t Util::HashString(const std::string& content) {
    std::hash<std::string> hashFunction;
    return hashFunction(content);
}

std::filesystem::path Util::ResolvePath(const std::filesystem::path& path) {
    std::filesystem::path res = path;
    if (!path.empty() && path.native()[0] == '~') {
        std::filesystem::path homeDir = GetUserHomeDirectory();
        res = homeDir.string() + path.string().substr(1);
    }
#ifdef _WIN32
    std::string nativePath = res.string();
    std::replace(nativePath.begin(), nativePath.end(), '/', '\\');
    res = nativePath;
#endif
    return res;
}
}    // namespace XRLib
