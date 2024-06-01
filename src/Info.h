#pragma once

#include <string>

class Info {
   public:
    bool validationLayer = false;

    unsigned int majorVersion = 0;
    unsigned int minorVersion = 0;
    unsigned int patchVersion = 0;

    std::string applicationName = "";

    bool fullscreen = false;

    inline static std::string triangleVert{
        "#version 450\n"
        "\n"
        "vec2 positions[3] = vec2[](\n"
        "    vec2(0.0, -0.5),\n"
        "    vec2(0.5, 0.5),\n"
        "    vec2(-0.5, 0.5)\n"
        ");\n"
        "\n"
        "void main() {\n"
        "    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);\n"
        "}\n"};

    inline static std::string triangleFrag{
        "#version 450\n"
        "\n"
        "layout(location = 0) out vec4 outColor;\n"
        "\n"
        "void main() {\n"
        "    outColor = vec4(1.0, 0.0, 0.0, 1.0);\n"
        "}\n"};
};
