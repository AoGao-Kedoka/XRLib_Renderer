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
set(VULKAN_DEPS "Vulkan::Vulkan")
find_package(Vulkan QUIET)
if (NOT Vulkan_FOUND)
    FetchContent_Declare(
        Vulkan-Headers
        GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-Headers.git
        GIT_TAG v1.3.278
    )
    FetchContent_MakeAvailable(Vulkan-Headers)

    FetchContent_Declare(
        Vulkan-Loader
        GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-Loader.git
        GIT_TAG v1.3.278
    )
    FetchContent_MakeAvailable(Vulkan-Loader)
    set(VULKAN_DEPS "Vulkan::Headers vulkan")
endif()

#----------------
# OpenXR
#----------------
set(OpenXR_DEPS "Openxr::header OpenXR::openxr_loader")
find_package(OpenXR CONFIG QUIET)
if (NOT Openxr_FOUND)
    FetchContent_Declare(
        OpenXR
        URL_HASH MD5=924a94a2da0b5ef8e82154c623d88644
        URL https://github.com/KhronosGroup/OpenXR-SDK-Source/archive/refs/tags/release-1.0.34.zip
            SOURCE_DIR
            openxr
    )
    set(BUILD_TESTS
        OFF
        CACHE INTERNAL "Build tests"
    )
    set(BUILD_API_LAYERS
        ON
        CACHE INTERNAL "Use OpenXR layers"
    )
    FetchContent_MakeAvailable(OpenXR)
    set(OpenXR_DEPS "OpenXR::openxr_loader")
endif()

#----------------
# GLM 
#----------------
find_package(GLM QUIET)
if (NOT GLM_FOUND)
    FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG 1.0.1
    )
    FetchContent_MakeAvailable(glm)
endif()

#----------------
# GLM 
#----------------
find_package(glfw3 CONFIG QUIET)
if (NOT glfw3_FOUND)
    FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.4
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
    )
    set(GLFW_INSTALL OFF)
    set(BUILD_SHARED_LIBS ON)
    set(GLFW_BUILD_DOCS OFF)
    set(GLFW_BUILD_COCOA OFF)
    set(GLFW_BUILD_WAYLAND OFF)
    FetchContent_MakeAvailable(glfw)
endif()

include_directories(${PROJECT_SOURCE_DIR}/src ${PROJECT_SOURCE_DIR}/lib ${PROJECT_SOURCE_DIR}/lib/external ${PROJECT_SOURCE_DIR}/example})

file (GLOB SRCS
    ${PROJECT_SOURCE_DIR}/src/*.h
    ${PROJECT_SOURCE_DIR}/src/*.cpp
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
    ${OpenXR_DEPS}
    glm::glm
    glfw)
