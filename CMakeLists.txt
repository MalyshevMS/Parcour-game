cmake_minimum_required(VERSION 3.29 FATAL_ERROR)

set(project_name_ FirstOpenGL)

project(${project_name_})

add_executable(${project_name_} src/main.cpp)

set(GLFW_BUILD_DOCS OFF BOOL "")
set(GLFW_BUILD_TESTS OFF BOOL "")
set(GLFW_BUILD_EXAMPLES OFF BOOL "")
set(GLFW_INSTALL OFF BOOL "")

add_subdirectory(external/glfw)
target_link_libraries(${project_name_} glfw)

add_subdirectory(external/glad)
target_link_libraries(${project_name_} glad)