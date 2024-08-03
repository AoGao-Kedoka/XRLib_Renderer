set(OPENXR_DEPS OpenXR::headers OpenXR::openxr_loader)
find_package(OpenXR CONFIG QUIET)
if (NOT OpenXR_FOUND)
    message("${MESSAGE_BOX}\nOpenXR not found, try fetching...\n${MESSAGE_BOX}")
    FetchContent_Declare(
        OpenXR
        URL_HASH MD5=924a94a2da0b5ef8e82154c623d88644
        URL https://github.com/KhronosGroup/OpenXR-SDK-Source/archive/refs/tags/release-1.0.34.zip
        SOURCE_DIR openxr
    )

    set(BUILD_TESTS OFF CACHE INTERNAL "Build tests")
    set(BUILD_API_LAYERS ON CACHE INTERNAL "Use OpenXR layers")
    FetchContent_MakeAvailable(OpenXR)
    set(OPENXR_DEPS openxr_loader)
endif()

