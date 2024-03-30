#include "XRLib.h"

int main() {
    XRLib xrLib;

    xrLib.SetVersionNumber(0, 0, 1)
        .SetApplicationName("Demo Application")
        .Init();

    return 0;
}
