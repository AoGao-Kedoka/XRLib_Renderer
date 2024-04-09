# XRLib
A library to create XR application without an engine. Build with Vulkan and OpenXR.
## Build
### Vcpkg (Recommended)
Install the dependencies:
```
vcpkg install openxr vulkan glm glfw
```

Build the project:
```
mkdir build
cmake .. -DCMAKE_TOOLCHAIN_FILE=<path_to_vcpkg>/scripts/buildsystems/vcpkg.cmakek
cmake --build .
```
### Without dependencies installed (longer build time)
```
mkdir build
cmake ..
cmake --build.
```
## References
- https://openxr-tutorial.com//windows/vulkan/index.html
- https://amini-allight.org/post/openxr-tutorial-part-0
