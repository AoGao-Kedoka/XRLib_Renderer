#pragma once

#include <string>

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
};
