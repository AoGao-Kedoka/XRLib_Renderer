#pragma once

#include <string>

namespace XRLib {
class Info {
   public:
    bool validationLayer = false;

    unsigned int majorVersion = 0;
    unsigned int minorVersion = 0;
    unsigned int patchVersion = 0;

    std::string applicationName = "";

    // window properties
    int windowMode = 0;
    unsigned int defaultWindowWidth = 720;
    unsigned int defaultWindowHeight = 480;

    // input
    float mouseSensitivity = 0.1f;
    float movementSpeed = 0.02;
};
}    // namespace XRLib
