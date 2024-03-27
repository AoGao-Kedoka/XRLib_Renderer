#include "XRLib.h"

int main() {
    XRLib xrLib;

    auto [xr, renderer] = xrLib.SetVersionNumber(1)
                              .SetApplicationName("Demo Application")
                              .EnableValidationLayer()
                              .InitRenderBackend()
                              .InitXRBackend()
                              .GetBackend();

}
