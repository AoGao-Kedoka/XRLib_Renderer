cmake_minimum_required(VERSION 3.15)
project(XRLib_Example)

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Vulkan REQUIRED)
find_package(OpenXR CONFIG REQUIRED)
find_package(GLM REQUIRED)

include_directories(${PROJECT_SOURCE_DIR}/src ${PROJECT_SOURCE_DIR}/lib ${PROJECT_SOURCE_DIR}/example})

file (GLOB SRCS
    ${PROJECT_SOURCE_DIR}/src/*.h
    ${PROJECT_SOURCE_DIR}/src/*.cpp
    ${PROJECT_SOURCE_DIR}/lib/*.h
    ${PROJECT_SOURCE_DIR}/lib/*.cpp
    ${PROJECT_SOURCE_DIR}/example/*.h
    ${PROJECT_SOURCE_DIR}/example/*.cpp
)
add_executable(${PROJECT_NAME} ${SRCS})
target_link_libraries(${PROJECT_NAME} PRIVATE Vulkan::Vulkan OpenXR::headers OpenXR::openxr_loader glm::glm)
