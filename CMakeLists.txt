cmake_minimum_required(VERSION 3.15)
project(XRLib_Example)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


file(DOWNLOAD
	https://raw.githubusercontent.com/nothings/stb/master/stb_image.h
	${CMAKE_SOURCE_DIR}/lib/external/stb_image.h
)
file(DOWNLOAD
    https://raw.githubusercontent.com/thegabman/native_message_box/master/include/NMB/NMB.h
    ${CMAKE_SOURCE_DIR}/lib/external/NMB.h
)

include(FetchContent)

#----------------
# Vulkan
#----------------
set(VULKAN_DEPS Vulkan::Vulkan)
find_package(Vulkan QUIET)
if (NOT Vulkan_FOUND)
    message("Vulkan not found")
    FetchContent_Declare(
        Vulkan-Headers
        GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-Headers.git
        GIT_TAG v1.3.278
    )

    FetchContent_Declare(
        Vulkan-Loader
        GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-Loader.git
        GIT_TAG v1.3.278
    )

    FetchContent_MakeAvailable(Vulkan-Headers)
    FetchContent_MakeAvailable(Vulkan-Loader)

    set(VULKAN_DEPS Vulkan::Headers Vulkan::Loader)
endif()


#----------------
# shaderc
#----------------
set(SHADERC_DEPS unofficial::shaderc::shaderc)
find_package(unofficial-shaderc CONFIG QUIET)
if (NOT unofficial-shaderc_FOUND)
	FetchContent_Declare(SPIRV-Cross
        GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Cross.git
        GIT_TAG vulkan-sdk-1.3.275.0
        UPDATE_DISCONNECTED TRUE
        GIT_PROGRESS TRUE
        )

    set(SPIRV_CROSS_CLI OFF)
    set(SPIRV_CROSS_ENABLE_TESTS OFF)
    FetchContent_MakeAvailable(SPIRV-Cross)

    FetchContent_Declare(SPIRV-Headers
        GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Headers.git
        GIT_TAG vulkan-sdk-1.3.275.0
        UPDATE_DISCONNECTED TRUE
        GIT_PROGRESS TRUE
        )
    set(SPIRV_HEADERS_SKIP_EXAMPLES ON)
    set(SPIRV_HEADERS_SKIP_INSTALL ON)
    FetchContent_MakeAvailable(SPIRV-Headers)

    FetchContent_Declare(SPIRV-Tools
        GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Tools.git
        GIT_TAG vulkan-sdk-1.3.275.0
        UPDATE_DISCONNECTED TRUE
        GIT_PROGRESS TRUE
        )
    set(SKIP_SPIRV_TOOLS_INSTALL ON)
    set(SPIRV_SKIP_EXECUTABLES ON)
    set(SPIRV_SKIP_TESTS ON)
    FetchContent_MakeAvailable(SPIRV-Tools)

    FetchContent_Declare(glslang
        GIT_REPOSITORY https://github.com/KhronosGroup/glslang.git
        GIT_TAG d73712b8f6c9047b09e99614e20d456d5ada2390
        UPDATE_DISCONNECTED TRUE
        GIT_PROGRESS TRUE
        )
    set(SKIP_GLSLANG_INSTALL ON)
    set(ENABLE_SPVREMAPPER OFF)
    set(ENABLE_GLSLANG_BINARIES OFF)
    set(ENABLE_GLSLANG_JS OFF)
    set(ENABLE_GLSLANG_BINARIES OFF)
    FetchContent_MakeAvailable(glslang)


    FetchContent_Declare(shaderc
        GIT_REPOSITORY https://github.com/google/shaderc.git
        GIT_TAG v2024.0
        UPDATE_DISCONNECTED TRUE
        GIT_PROGRESS TRUE
        )
    set(SHADERC_ENABLE_WGSL_OUTPUT OFF)
    set(SHADERC_SKIP_INSTALL ON)
    set(SHADERC_SKIP_TESTS ON)
    set(SHADERC_SKIP_EXAMPLES ON)
    set(SHADERC_SKIP_COPYRIGHT_CHECK ON)
    set(SHADERC_SKIP_COPYRIGHT_CHECK ON)
    set(SHADERC_ENABLE_WERROR_COMPILE OFF)
    set(SHADERC_ENABLE_SHARED_CRT ON)
    FetchContent_MakeAvailable(shaderc)
endif()

#----------------
# OpenXR
#----------------
set(OPENXR_DEPS
    OpenXR::headers OpenXR::openxr_loader
)
find_package(OpenXR CONFIG QUIET)
if (NOT OpenXR_FOUND)
    message("OpenXR not found")

    set(BUILD_TESTS
        OFF
        CACHE INTERNAL "Build tests"
    )
    set(BUILD_API_LAYERS
        ON
        CACHE INTERNAL "Use OpenXR layers"
    )

    FetchContent_Declare(
        OpenXR
        URL_HASH MD5=924a94a2da0b5ef8e82154c623d88644
        URL https://github.com/KhronosGroup/OpenXR-SDK-Source/archive/refs/tags/release-1.0.34.zip
            SOURCE_DIR
            openxr
    )
    FetchContent_MakeAvailable(OpenXR)
    set(OPENXR_DEPS openxr_loader)
endif()

#----------------
# GLM 
#----------------
find_package(GLM QUIET)
if (NOT GLM_FOUND)
    message("glm not found")
    FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG 1.0.1
    )
    FetchContent_MakeAvailable(glm)
endif()

#----------------
# GLFW
#----------------
find_package(glfw3 CONFIG QUIET)
if (NOT glfw3_FOUND)
    message("GLFW not found")
    FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.4
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
    )
    FetchContent_MakeAvailable(glfw)
endif()

include_directories(${PROJECT_SOURCE_DIR}/src ${PROJECT_SOURCE_DIR}/lib ${PROJECT_SOURCE_DIR}/lib/external ${PROJECT_SOURCE_DIR}/example)

file (GLOB SRCS
    ${PROJECT_SOURCE_DIR}/src/*.h
    ${PROJECT_SOURCE_DIR}/src/*.cpp
    ${PROJECT_SOURCE_DIR}/src/Graphics/*.h
    ${PROJECT_SOURCE_DIR}/src/Graphics/*.cpp
    ${PROJECT_SOURCE_DIR}/lib/*.h
    ${PROJECT_SOURCE_DIR}/lib/*.cpp
    ${PROJECT_SOURCE_DIR}/lib/external/*.h
    ${PROJECT_SOURCE_DIR}/lib/external/*.cpp
    ${PROJECT_SOURCE_DIR}/example/*.h
    ${PROJECT_SOURCE_DIR}/example/*.cpp
)
add_executable(${PROJECT_NAME} ${SRCS})

target_link_libraries(${PROJECT_NAME} PRIVATE 
    ${VULKAN_DEPS}
    ${OPENXR_DEPS}
	shaderc
    glm::glm
    glfw)
