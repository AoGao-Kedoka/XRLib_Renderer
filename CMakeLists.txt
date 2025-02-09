cmake_minimum_required(VERSION 3.15)

if(APPLE)
    set(LANGUAGES C CXX OBJC OBJCXX)
else()
    set(LANGUAGES C CXX)
endif()
project(XRLib VERSION 1.0.0 LANGUAGES ${LANGUAGES})

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(DOWNLOAD
    https://raw.githubusercontent.com/nothings/stb/master/stb_image.h
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/stb_image.h
)

add_library(${PROJECT_NAME} STATIC)
add_library(XRLib::XRLib ALIAS XRLib)

if (LINUX)
    message( STATUS "Platform: Linux" )
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GTK REQUIRED gtk+-3.0)
    target_include_directories(${PROJECT_NAME} PUBLIC ${GTK_INCLUDE_DIRS})
    target_link_directories(${PROJECT_NAME} PUBLIC ${GTK_LIBRARY_DIRS})
    target_compile_options(${PROJECT_NAME} PRIVATE ${GTK_CFLAGS_OTHER})
    target_link_libraries(${PROJECT_NAME} PUBLIC ${GTK_LIBRARIES})
elseif (APPLE)
    find_library(COCOA_LIBRARY Cocoa)
    target_link_libraries(${PROJECT_NAME} PUBLIC ${COCOA_LIBRARY})
else()
    message( STATUS "Platform: Unkown" )
endif()

set(MESSAGE_BOX "---------------------------")
include(FetchContent)

include(cmake/vulkan.cmake)
include(cmake/openxr.cmake)
include(cmake/glm.cmake)
include(cmake/glfw.cmake)
include(cmake/assimp.cmake)
include(cmake/shaderc.cmake)
include(cmake/format.cmake)
target_compile_definitions(${PROJECT_NAME} PRIVATE USE_STD_FORMAT)

# Precompile header
set(PCH_HEADER "src/pch.h")
set(PCH_SOURCE "src/pch.cpp")
target_precompile_headers(${PROJECT_NAME} PRIVATE ${PCH_HEADER})

target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/lib>
    $<INSTALL_INTERFACE:include>
)

file(GLOB_RECURSE XRLIB_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.tpp
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/*.cpp
)
if (APPLE)
    list(APPEND XRLIB_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/lib/boxer_mac.mm)
endif()

target_sources(${PROJECT_NAME} PRIVATE ${XRLIB_SOURCES})

target_link_libraries(${PROJECT_NAME} PUBLIC 
    ${VULKAN_DEPS}
    ${OPENXR_DEPS}
    ${SHADERC_DEPS}
    glm::glm
    glfw
    ${ASSIMP_DEPS}
)
if (NOT HAS_STD_FORMAT)
    target_link_libraries( ${PROJECT_NAME} PUBLIC fmt::fmt)
endif()

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
    find_dependency(shaderc REQUIRED)
if (NOT HAS_STD_FORMAT)
    find_dependency(fmt REQUIRED)
endif()
    include("${CMAKE_CURRENT_LIST_DIR}/${PROJECT_NAME}Targets.cmake")
)
