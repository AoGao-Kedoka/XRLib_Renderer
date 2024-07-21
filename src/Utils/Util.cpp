#include "Util.h"

#if defined(_WIN64)
#include "NMB.h"
#endif

const char* getCurrentPlatform() {
#if defined(_WIN32)
#define PLATFORM_NAME "windows"
#elif defined(_WIN64)
#define PLATFORM_NAME "windows"
#elif defined(__CYGWIN__) && !defined(_WIN32)
#define PLATFORM_NAME                                                          \
    "windows"    // Windows (Cygwin POSIX under Microsoft Window)
#elif defined(__ANDROID__)
#define PLATFORM_NAME                                                          \
    "android"    // Android (implies Linux, so it must come first)
#elif defined(__linux__)
#define PLATFORM_NAME                                                          \
    "linux"    // Debian, Ubuntu, Gentoo, Fedora, openSUSE, RedHat, Centos and other
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
    LOGGER(LOGGER::INFO) << "Current platform" << platform;
    if (std::strcmp("windows", platform))
        return true;
    if (std::strcmp("linux", platform)) {
        LOGGER(LOGGER::WARNING) << "Linux is not fully tested yet";
        return true;
    } else
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

void Util::ErrorPopup(std::string message) {
    LOGGER(LOGGER::ERR) << message;
#if defined(_WIN64)
    NMB::show("Error", message.c_str(), NMB::Icon::ICON_ERROR);
#endif
    throw std::runtime_error(message.c_str());
}

std::string Util::ReadFile(std::string file_path) {
    std::ifstream t(file_path);
    std::string result = std::string((std::istreambuf_iterator<char>(t)),
                                     (std::istreambuf_iterator<char>()));
    if (result == "") {
        LOGGER(LOGGER::ERR) << "FileReader: FILE IS EMPTY!!";
        return result;
    }
    LOGGER(LOGGER::DEBUG) << result;
    return result;
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
