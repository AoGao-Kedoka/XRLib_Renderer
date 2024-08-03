find_package(GLM QUIET)
if (NOT GLM_FOUND)
    message("${MESSAGE_BOX}\nglm not found, try fetching...\n${MESSAGE_BOX}")
    FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG 1.0.1
    )
    set(GLM_BUILD_INSTALL OFF)
    FetchContent_MakeAvailable(glm)
endif()

