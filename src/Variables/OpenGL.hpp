#pragma once
#include <glad/glad.h>
#include <string>
#include <glm/vec2.hpp>

struct OpenGL {
    GLenum texture_mode = GL_LINEAR; // GL_LINEAR or GL_NEAREST
    std::string sprite_shader = "default"; // Name of default sprite shader
    std::string sprite_shader_vertex = "res/shaders/vSprite.cfg";
    std::string sprite_shader_fragment = "res/shaders/fSprite.cfg";
    int sprite_size = 80; // Default size of a sprite
    int font_width = 80;
    int font_height = 80;
    glm::vec2 win_size = glm::vec2(1280, 720);
    std::string win_title = "ParCour-game";
    bool fullscreen = false;
};