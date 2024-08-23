#include <glad/glad.h>
#include <string>

struct OpenGL {
    GLenum texture_mode; // GL_LINEAR or GL_NEAREST
    std::string default_shader; // Name of default shader
    std::string sprite_shader; // Name of default sprite shader
    std::string default_shader_path_list[2] = { "res/shaders/vertex.cfg", "res/shaders/fragment.cfg" }; // List of paths to default shader
    std::string sprite_shader_path_list[2] = { "res/shaders/vSprite.cfg", "res/shaders/fSprite.cfg" }; // List of paths to sprite shader
    int sprite_size; // Default size of a sprite
};