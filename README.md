# XRLib
This is a personal library aimed at accelerating the development of XR applications without the use of a traditional game engine. The library is built using Vulkan and OpenXR.


This project is currently in the very early stages of development and is subject to significant changes. As this is an ongoing personal project, stability and feature completeness cannot be guaranteed at this time.

![image](./imgs/screenshot.png)



## Implementation Overview
**Completed features**:
- screen space and vr rendering
- basic single-pass phong illumination
- controller binding "/khr/simple_controller"

**WIP**:
- custom renderpass
- PBR
- more controller profiles (Quest, Pico)
- xr mirror view

## Usage
```
include(FetchContent)

FetchContent_Declare(
    XRLib
    GIT_REPOSITORY https://github.com/AoGao-Kedoka/XRLib_Renderer.git
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
