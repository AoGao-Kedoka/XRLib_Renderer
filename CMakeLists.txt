cmake_minimum_required(VERSION 3.15)
project(XRLib VERSION 1.0.0)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(DOWNLOAD
    https://raw.githubusercontent.com/nothings/stb/master/stb_image.h
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/external/stb_image.h
)

if (WIN32)
    file(DOWNLOAD
        https://raw.githubusercontent.com/thegabman/native_message_box/master/include/NMB/NMB.h
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/external/NMB.h
    )
endif()

set(MESSAGE_BOX "---------------------------")
include(FetchContent)

# Include dependency cmake files
include(cmake/vulkan.cmake)
include(cmake/shaderc.cmake)
include(cmake/openxr.cmake)
include(cmake/glm.cmake)
include(cmake/glfw.cmake)
include(cmake/assimp.cmake)

# Define the library target
add_library(${PROJECT_NAME} STATIC)
add_library(XRLib::XRLib ALIAS XRLib)

target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/lib>
    $<INSTALL_INTERFACE:include>
)

file(GLOB_RECURSE XRLIB_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/external/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/external/*.cpp
)

target_sources(${PROJECT_NAME} PRIVATE ${XRLIB_SOURCES})

target_link_libraries(${PROJECT_NAME} PUBLIC 
    ${VULKAN_DEPS}
    ${OPENXR_DEPS}
    ${SHADERC_DEPS}
    glm::glm
    glfw
    ${ASSIMP_DEPS}
)

# Export targets for use in other projects
include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

install(TARGETS ${PROJECT_NAME}
    EXPORT ${PROJECT_NAME}Targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY src/ lib/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}
    FILES_MATCHING PATTERN "*.h"
)

install(EXPORT ${PROJECT_NAME}Targets
    FILE ${PROJECT_NAME}Targets.cmake
    NAMESPACE ${PROJECT_NAME}::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
include(CMakeFindDependencyMacro)

find_dependency(Vulkan REQUIRED)
find_dependency(OpenXR REQUIRED)
find_dependency(glm REQUIRED)
find_dependency(glfw3 REQUIRED)
find_dependency(assimp REQUIRED)

include(\"\${CMAKE_CURRENT_LIST_DIR}/${PROJECT_NAME}Targets.cmake\")
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)
