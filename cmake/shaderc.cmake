set(SHADERC_DEPS unofficial::shaderc::shaderc)
find_package(unofficial-shaderc CONFIG QUIET)
if (NOT unofficial-shaderc_FOUND)
    message("${MESSAGE_BOX}\nShaderc not found, try fetching...\n${MESSAGE_BOX}")
    FetchContent_Declare(SPIRV-Cross
        GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Cross.git
        GIT_TAG vulkan-sdk-1.3.275.0
    )
    set(SPIRV_CROSS_CLI OFF)
    set(SPIRV_CROSS_ENABLE_TESTS OFF)
    set(SPIRV_CROSS_SKIP_INSTALL OFF)
    FetchContent_MakeAvailable(SPIRV-Cross)

    FetchContent_Declare(SPIRV-Headers
        GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Headers.git
        GIT_TAG vulkan-sdk-1.3.275.0
    )
    set(SPIRV_HEADERS_SKIP_EXAMPLES ON)
    set(SPIRV_HEADERS_SKIP_INSTALL OFF)
    FetchContent_MakeAvailable(SPIRV-Headers)

    FetchContent_Declare(SPIRV-Tools
        GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Tools.git
        GIT_TAG vulkan-sdk-1.3.275.0
    )
    set(SKIP_SPIRV_TOOLS_INSTALL oFF)
    set(SPIRV_SKIP_EXECUTABLES ON)
    set(SPIRV_SKIP_TESTS ON)
    FetchContent_MakeAvailable(SPIRV-Tools)

    FetchContent_Declare(glslang
        GIT_REPOSITORY https://github.com/KhronosGroup/glslang.git
        GIT_TAG d73712b8f6c9047b09e99614e20d456d5ada2390
    )
    set(SKIP_GLSLANG_INSTALL OFF)
    set(ENABLE_SPVREMAPPER OFF)
    set(ENABLE_GLSLANG_BINARIES OFF)
    set(ENABLE_GLSLANG_JS OFF)
    set(ENABLE_GLSLANG_BINARIES OFF)
    FetchContent_MakeAvailable(glslang)

    FetchContent_Declare(shaderc
        GIT_REPOSITORY https://github.com/google/shaderc.git
        GIT_TAG v2024.0
    )
    set(SHADERC_ENABLE_WGSL_OUTPUT OFF)
    set(SHADERC_OFF ON CACHE BOOL "" FORCE)
    set(SHADERC_SKIP_TESTS ON)
    set(SHADERC_SKIP_EXAMPLES ON)
    set(SHADERC_SKIP_COPYRIGHT_CHECK ON)
    set(SHADERC_ENABLE_WERROR_COMPILE OFF)
    set(SHADERC_ENABLE_SHARED_CRT ON)
    FetchContent_MakeAvailable(shaderc)
    set(SHADERC_DEPS shaderc)
endif()

