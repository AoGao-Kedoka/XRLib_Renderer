#pragma once

#include <string>

namespace XRLib {
class Config {
   public:
    bool validationLayer = false;

    unsigned int majorVersion = 0;
    unsigned int minorVersion = 0;
    unsigned int patchVersion = 0;

    std::string applicationName = "";

    // window properties
    int windowMode = 0;
    bool resizable = true;
    unsigned int defaultWindowWidth = 720;
    unsigned int defaultWindowHeight = 480;

    // input
    float mouseSensitivity = 0.1f;
    float movementSpeed = 5.0f;
};
}    // namespace XRLib
