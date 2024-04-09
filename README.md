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
cmake .. -DCMAKE_TOOLCHAIN_FILE=<path_to_vcpkg>/scripts/buildsystems/vcpkg.cmakek
```

## References
- https://openxr-tutorial.com//windows/vulkan/index.html
- https://amini-allight.org/post/openxr-tutorial-part-0
