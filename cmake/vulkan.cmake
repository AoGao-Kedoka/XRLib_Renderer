set(VULKAN_DEPS Vulkan::Vulkan)
find_package(Vulkan QUIET)
if (NOT Vulkan_FOUND)
    message("${MESSAGE_BOX}\nVulkan not found, try fetching...\n${MESSAGE_BOX}")
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

    FetchContent_MakeAvailable(Vulkan-Headers Vulkan-Loader)

    set(VULKAN_DEPS Vulkan::Headers Vulkan::Loader)
endif()

