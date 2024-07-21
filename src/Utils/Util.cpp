#include "Util.h"

#if defined(_WIN64)
#include "NMB.h"
#endif

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
        return fs::path(homeDir);
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
