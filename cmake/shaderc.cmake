find_package(unofficial-shaderc CONFIG QUIET)

if (unofficial-shaderc_FOUND)
    set(SHADERC_DEPS unofficial::shaderc::shaderc)
elseif(NOT TARGET shaderc::shaderc)
    find_package(shaderc CONFIG QUIET)

    if (shaderc_FOUND)
        set(SHADERC_DEPS shaderc::shaderc)
    else()
        message(WARNING "${MESSAGE_BOX}\nShaderc not found. Fetching dependencies...\n${MESSAGE_BOX}")

        # Fetch SPIRV-Cross
        FetchContent_Declare(SPIRV-Cross
            GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Cross.git
            GIT_TAG vulkan-sdk-1.3.275.0
        )
        set(SPIRV_CROSS_CLI OFF)
        set(SPIRV_CROSS_ENABLE_TESTS OFF)

        # Fetch SPIRV-Headers
        FetchContent_Declare(SPIRV-Headers
            GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Headers.git
            GIT_TAG vulkan-sdk-1.3.275.0
        )
        set(SPIRV_HEADERS_SKIP_EXAMPLES ON)
        set(SPIRV_HEADERS_SKIP_INSTALL ON)

        # Fetch SPIRV-Tools
        FetchContent_Declare(SPIRV-Tools
            GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Tools.git
            GIT_TAG vulkan-sdk-1.3.275.0
        )
        set(SKIP_SPIRV_TOOLS_INSTALL ON)
        set(SPIRV_SKIP_EXECUTABLES ON)
        set(SPIRV_SKIP_TESTS ON)

        # Fetch glslang
        FetchContent_Declare(glslang
            GIT_REPOSITORY https://github.com/KhronosGroup/glslang.git
            GIT_TAG d73712b8f6c9047b09e99614e20d456d5ada2390
        )
        set(SKIP_GLSLANG_INSTALL OFF)
        set(ENABLE_SPVREMAPPER OFF)
        set(ENABLE_GLSLANG_BINARIES OFF)
        set(ENABLE_GLSLANG_JS OFF)

        # Fetch Shaderc
        FetchContent_Declare(shaderc
            GIT_REPOSITORY https://github.com/google/shaderc.git
            GIT_TAG v2024.0
        )
        set(SHADERC_ENABLE_WGSL_OUTPUT OFF)
        set(SHADERC_SKIP_INSTALL ON)
        set(SHADERC_SKIP_TESTS ON)
        set(SHADERC_SKIP_EXAMPLES ON)
        set(SHADERC_SKIP_COPYRIGHT_CHECK ON)
        set(SHADERC_ENABLE_WERROR_COMPILE OFF)
        set(SHADERC_ENABLE_SHARED_CRT ON)

        # Fetch all dependencies in one call
        FetchContent_MakeAvailable(SPIRV-Cross SPIRV-Headers SPIRV-Tools glslang shaderc)

        set(SHADERC_DEPS shaderc)
    endif()
endif()
