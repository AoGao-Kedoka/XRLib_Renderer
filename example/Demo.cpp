#include "XRLib.h"

int main() {
    XRLib xrLib;

    xrLib.SetVersionNumber(1)
        .SetApplicationName("Demo Application")
        .EnableValidationLayer();

    XRBackend xr = xrLib.InitXRBackend();
    RenderBackend renderer = xrLib.InitRenderBackend();
}
