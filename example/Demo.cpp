#include "XRLib.h"

int main() {
    XRLib::XRLib xrLib;

    xrLib.SetVersionNumber(0, 0, 1)
         .EnableValidationLayer()
         .SetApplicationName("Demo Application");

    Scene& sceneContex = xrLib.SceneBackend();
    sceneContex.LoadMeshAsync("./mesh_1.obj")
               .LoadMeshAsync("./mesh_2.obj")
               .WaitForAllMeshesToLoad();

    xrLib.Init(false);

    while (!xrLib.WindowShouldClose()) {
        xrLib.Run();
    }

    return 0;
}
