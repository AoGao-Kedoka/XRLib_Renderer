find_package(glfw3 CONFIG QUIET)
if (NOT glfw3_FOUND)
    message("${MESSAGE_BOX}\nGLFW not found, try fetching...\n${MESSAGE_BOX}")
    FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.4
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
    )
    FetchContent_MakeAvailable(glfw)
endif()

