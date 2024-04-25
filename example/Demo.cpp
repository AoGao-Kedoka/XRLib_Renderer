#include "XRLib.h"

int main() {
    XRLib::XRLib xrLib;

    xrLib.SetVersionNumber(0, 0, 1)
        .EnableValidationLayer()
        .SetApplicationName("Demo Application")
        .Init(false);

    xrLib.SetRenderPassShader("./vertex.spv", "./fragment.spv");
    return 0;
}
