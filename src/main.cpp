#include <glad/glad.h> // OpenGL libs
#include <GLFW/glfw3.h>

#include <glm/vec2.hpp> // glm libs
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Renderer/ShaderProgram.hpp" // my libs
#include "Renderer/Texture2D.hpp"
#include "Renderer/Sprite.hpp"
#include "Resources/ResourceManager.hpp"
#include "Resources/TextureLoader.hpp"
#include "Resources/SpriteLoader.hpp"

#include <iostream> // other libs
#include <string>
#include <Windows.h>

glm::ivec2 Size(1280, 720);

// Eng vars
GLenum gl_texture_mode = GL_LINEAR; // GL_LINEAR or GL_NEAREST
string gl_default_shader = "DefaultShader"; // Name of default shader
string gl_default_sprite_shader = "SpriteShader"; // Name of default sprite shader

void sizeHandler(GLFWwindow* win, int width, int height) {
    Size.x = width;
    Size.y = height;
    glViewport(0, 0, Size.x, Size.y);
}

void keyHandler(GLFWwindow* win, int key, int scancode, int action, int mode) {
    if ((key == GLFW_KEY_ESCAPE || key == GLFW_KEY_GRAVE_ACCENT) && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(win, GL_TRUE);
    }
}

int main(int argc, char* argv[]) {
    if (!glfwInit()) {
        std::cerr << "glfwInit failed!" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(Size.x, Size.y, "M2 Engine", nullptr, nullptr);
    if (!window) {
        std::cerr << "glfwCreateWindow failed!" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwSetWindowSizeCallback(window, sizeHandler);
    glfwSetKeyCallback(window, keyHandler);

    glfwMakeContextCurrent(window);

    if (!gladLoadGL()) {
        std::cerr << "Can't load GLAD!" << std::endl;
    }

    std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;

    glClearColor(1, 1, 0, 1);

    {
        ResourceManager resMan(argv[0]);
        TexLoader textures(&resMan);
        SprLoader sprites(&resMan);

        auto defaultShaderProgram = resMan.loadShaders(gl_default_shader, "res/shaders/vertex.cfg", "res/shaders/fragment.cfg");

        if (!defaultShaderProgram) {
            std::cerr << "Can't create shader program!" << std::endl;
            return -1;
        }

        auto spriteShaderProgram = resMan.loadShaders(gl_default_sprite_shader, "res/shaders/vSprite.cfg", "res/shaders/fSprite.cfg");

        if (!spriteShaderProgram) {
            std::cerr << "Can't create SpriteShader" << std::endl;
            return -1;
        }

        textures.add_texture("Texture001", "res/textures/texture001.png");
        textures.add_texture("Texture002", "res/textures/texture002.png");

        defaultShaderProgram->use();
        defaultShaderProgram->setInt("tex", 0);

        spriteShaderProgram->use();
        spriteShaderProgram->setInt("tex", 0);

        glm::mat4 projMat = glm::ortho(0.f, static_cast <float> (Size.x), 0.f, static_cast <float> (Size.y), -100.f, 100.f);

        defaultShaderProgram->setMat4("projMat", projMat);
        spriteShaderProgram->setMat4("projMat", projMat);

        sprites.add_sprite("Sprite_1", "Texture001", gl_default_sprite_shader, 200, 200, 0.f, 0, 0);
        sprites.add_sprite("Sprite_2", "Texture002", gl_default_sprite_shader, 200, 200, 0.f, 0, 200);

        while (!glfwWindowShouldClose(window)) {
            glClear(GL_COLOR_BUFFER_BIT);
            
            defaultShaderProgram->use();
            textures.bind_all();
            sprites.render_all();

            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    }

    glfwTerminate();
    return 0;
}