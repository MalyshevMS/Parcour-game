cmake_minimum_required(VERSION 3.25 FATAL_ERROR)

set(project_name_ Parcour-game)

project(${project_name_})

add_executable(${project_name_} 
    src/main.cpp
    src/Renderer/ShaderProgram.hpp
    src/Resources/ResourceManager.hpp
    src/Resources/stb_image.hpp
    src/Renderer/Texture2D.hpp
    src/Renderer/Sprite.hpp
)

target_compile_features(${project_name_} PUBLIC cxx_std_17)

set(GLFW_BUILD_DOCS OFF BOOL "")
set(GLFW_BUILD_TESTS OFF BOOL "")
set(GLFW_BUILD_EXAMPLES OFF BOOL "")
set(GLFW_INSTALL OFF BOOL "")

add_subdirectory(external/glfw)
target_link_libraries(${project_name_} glfw)

add_subdirectory(external/glad)
target_link_libraries(${project_name_} glad)

add_subdirectory(external/glm)
target_link_libraries(${project_name_} glm)

set_target_properties(${project_name_} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/)

add_custom_command(TARGET ${project_name_} POST_BUILD 
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/res $<TARGET_FILE_DIR:${project_name_}>/res)