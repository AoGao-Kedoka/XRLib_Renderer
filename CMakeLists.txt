cmake_minimum_required(VERSION 3.15)
project(XRLib_Example)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


file(DOWNLOAD
	https://raw.githubusercontent.com/nothings/stb/master/stb_image.h
	${CMAKE_SOURCE_DIR}/lib/external/stb_image.h
)

if (WIN32)
file(DOWNLOAD
    https://raw.githubusercontent.com/thegabman/native_message_box/master/include/NMB/NMB.h
    ${CMAKE_SOURCE_DIR}/lib/external/NMB.h
)
endif()

set(MESSAGE_BOX "---------------------------")
include(FetchContent)

# include dependency cmake files
include(cmake/vulkan.cmake)
include(cmake/shaderc.cmake)
include(cmake/openxr.cmake)
include(cmake/glm.cmake)
include(cmake/glfw.cmake)
include(cmake/assimp.cmake)

include_directories(${PROJECT_SOURCE_DIR}/src ${PROJECT_SOURCE_DIR}/lib ${PROJECT_SOURCE_DIR}/lib/external ${PROJECT_SOURCE_DIR}/example)

file (GLOB SRCS
    ${PROJECT_SOURCE_DIR}/src/*.h
    ${PROJECT_SOURCE_DIR}/src/*.cpp
    ${PROJECT_SOURCE_DIR}/src/Graphics/*.h
    ${PROJECT_SOURCE_DIR}/src/Graphics/*.cpp
    ${PROJECT_SOURCE_DIR}/src/Utils/*.h
    ${PROJECT_SOURCE_DIR}/src/Utils/*.cpp
	${PROJECT_SOURCE_DIR}/src/XR/*.h
	${PROJECT_SOURCE_DIR}/src/XR/*.cpp
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
    ${SHADERC_DEPS}
    glm::glm
    glfw
	${ASSIMP_DEPS}
)
