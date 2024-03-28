#include "XRLib.h"

int main() {
    XRLib xrLib;

    auto [xr, renderer] = xrLib.SetVersionNumber(0,0,1)
                              .SetApplicationName("Demo Application")
                              .InitRenderBackend()
                              .InitXRBackend()
                              .GetBackend();

}
