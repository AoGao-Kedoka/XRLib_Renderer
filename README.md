# XRLib
A library to create XR application without an engine. Build with Vulkan and OpenXR.

## Usage
```
include(FetchContent)

FetchContent_Declare(
    XRLib
    GIT_REPOSITORY https://github.com/AoGao-Kedoka/XRLib.git
    GIT_TAG main
)

FetchContent_MakeAvailable(XRLib)

target_link_libraries(YOUR_TARGET PRIVATE XRLib::XRLib)
```

**Note**:

It's **recommended** to pre-install the dependencies via vcpkg, otherwise it will cause very long compile time for the first time:
```
vcpkg install openxr-loader vulkan glm glfw3 shaderc
```
Then build your project via:
```
cmake .. -DCMAKE_TOOLCHAIN_FILE=<path_to_vcpkg>/scripts/buildsystems/vcpkg.cmake
```

## References
- https://openxr-tutorial.com//windows/vulkan/index.html
- https://amini-allight.org/post/openxr-tutorial-part-0
