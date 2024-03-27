#pragma once
#include <string>
class Info {
public:
    bool validationLayer = false;

    unsigned int majorVersion = 0;
    unsigned int minorVersion = 0;
    unsigned int patchVersion = 0;

    std::string applicationName = "";
};