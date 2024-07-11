#include "XRLib.h"

int main() {
    XRLib::XRLib xrLib;

    xrLib.SetVersionNumber(0, 0, 1)
         .EnableValidationLayer()
         .SetApplicationName("Demo Application");

    Scene& sceneContex = xrLib.SceneBackend();
    sceneContex.LoadMeshAsync("./resources/teapot.obj")
               .WaitForAllMeshesToLoad();

    xrLib.Init();

    while (!xrLib.WindowShouldClose()) {
        xrLib.Run();
    }

    return 0;
}
