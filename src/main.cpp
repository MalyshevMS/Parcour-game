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
#include <cmath>
#include <vector>
#include <thread>
#include <Windows.h>

using namespace std;

glm::ivec2 Size(1280, 720);

// Eng vars
GLenum gl_texture_mode             = GL_LINEAR;                                              // GL_LINEAR or GL_NEAREST
string gl_default_shader           = "DefaultShader";                                        // Name of default shader
string gl_default_sprite_shader    = "SpriteShader";                                         // Name of default sprite shader
vector <string> gl_default_shader_path_list = {"res/shaders/vertex.cfg", "res/shaders/fragment.cfg"}; // List of paths to default shader
vector <string> gl_sprite_shader_path_list  = {"res/shaders/vSprite.cfg", "res/shaders/fSprite.cfg"}; // List of paths to sprite shader

vector <string> tx_path_list = {
    "res/textures/texture001.png",
    "res/textures/texture002.png"
}; // List of paths to textures

int cam_x = Size.x / 2; // Camera X pos
int cam_y = Size.y / 2; // Camera Y pos
int cam_zero_x = Size.x / 2; // Camera Zero X pos
int cam_zero_y = Size.y / 2; // Camera Zero Y pos
float cam_rot = 180.f; // Camera rotation

int sv_max_speed = 10; // Max player (camera) movement speed
float sv_rot_speed = 10.f; // Max player (camera) rotation speed

void sizeHandler(GLFWwindow* win, int width, int height) {
    Size.x = width;
    Size.y = height;
    glViewport(0, 0, Size.x, Size.y); // Razrab OpenGL ti psina
}

void keyHandler(GLFWwindow* win, int key, int scancode, int action, int mode) {
    if ((key == GLFW_KEY_ESCAPE || key == GLFW_KEY_GRAVE_ACCENT) && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(win, GL_TRUE);
    }

    if (action == GLFW_REPEAT || action == GLFW_PRESS) {
        if (key == GLFW_KEY_W) {
            cam_x -= sv_max_speed * sin(glm::radians(cam_rot));
            cam_y -= sv_max_speed * cos(glm::radians(cam_rot));
        }

        if (key == GLFW_KEY_S) {
            cam_x += sv_max_speed * sin(glm::radians(cam_rot));
            cam_y += sv_max_speed * cos(glm::radians(cam_rot));
        }

        if (key == GLFW_KEY_A) {
            cam_rot -= sv_rot_speed;
        }

        if (key == GLFW_KEY_D) {
            cam_rot += sv_rot_speed;
        }
    }
}

int main(int argc, char const *argv[]) {
    /*do {
        string com;
        cout << "Minimal2D > ";
        cin >> com;

        if (com == "exit") { exit(0); }
        else if (com == "init") {
            if (!eng_is_init) {
                eng_is_init = true;
                eng_codes = eng_init_code;
            } else cout << "Already initialized!" << endl;
        }
        else if (com == "get") {
            string variable;
            cin >> variable;

            if (variable == "gl_texture_mode") {
                if (gl_texture_mode == GL_LINEAR) cout << "gl_texture_mode: " << '"' << "GL_LINEAR" << '"' << endl;
                else if (gl_texture_mode == GL_NEAREST) cout << "gl_texture_mode: " << '"' << "GL_NEAREST" << '"' << endl;
            } else if (variable == "gl_default_shader") {
                cout << "gl_default_shader: " << '"' << gl_default_shader << '"' << endl;
            }
        } else if (com == "set") {
            string new_, variable;
            cin >> new_;
            cin >> variable;

            if (variable == "gl_texture_mode") {                
                if (new_ == "GL_LINEAR") gl_texture_mode = GL_LINEAR;
                else if (new_ == "GL_NEAREST") gl_texture_mode = GL_NEAREST;
            } else if (variable == "gl_default_shader") {
                gl_default_shader = new_;
            }
        }
    } while (eng_codes != eng_init_code);*/

    if (!glfwInit()) {
        cerr << "glfwInit failed!" << endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(Size.x, Size.y, "M2 Engine", nullptr, nullptr);
    if (!window) {
        cerr << "glfwCreateWindow failed!" << endl;
        glfwTerminate();
        return -1;
    }

    glfwSetWindowSizeCallback(window, sizeHandler);
    glfwSetKeyCallback(window, keyHandler);

    glfwMakeContextCurrent(window);

    if (!gladLoadGL()) {
        cerr << "Can't load GLAD!" << endl;
    }

    cout << "Renderer: " << glGetString(GL_RENDERER) << endl;
    cout << "OpenGL version: " << glGetString(GL_VERSION) << endl;

    glClearColor(0, 1, 0, 1);

    {
        ResourceManager resMan(argv[0]);
        TexLoader textures(&resMan);
        SprLoader sprites(&resMan);
        SprLoader player(&resMan);

        auto defaultShaderProgram = resMan.loadShaders(gl_default_shader, gl_default_shader_path_list[0], gl_default_shader_path_list[1]);

        if (!defaultShaderProgram) {
            cerr << "Can't create shader program!" << endl;
            return -1;
        }

        auto spriteShaderProgram = resMan.loadShaders(gl_default_sprite_shader, gl_sprite_shader_path_list[0], gl_sprite_shader_path_list[1]);

        if (!spriteShaderProgram) {
            cerr << "Can't create SpriteShader" << endl;
            return -1;
        }

        textures.add_texture("Texture001", tx_path_list[0]);
        textures.add_texture("Texture002", tx_path_list[1]);

        defaultShaderProgram->use();
        defaultShaderProgram->setInt("tex", 0);

        spriteShaderProgram->use();
        spriteShaderProgram->setInt("tex", 0);

        glm::mat4 projMat = glm::ortho(0.f, static_cast <float> (Size.x), 0.f, static_cast <float> (Size.y), -100.f, 100.f);

        defaultShaderProgram->setMat4("projMat", projMat);
        spriteShaderProgram->setMat4("projMat", projMat);

        player.add_sprite("Sprite_Player_1", "Texture001", gl_default_sprite_shader, 80, 80, 0.f, cam_x - 40, cam_y - 40);
        
        for (int i = 0; i < Size.y; i += 80) {
            sprites.add_sprite("Sprite_Wall_Left_" + std::to_string(i/80), "Texture002", gl_default_sprite_shader, 80, 80, 0.f, 0, i);
            sprites.add_sprite("Sprite_Wall_Right_" + std::to_string(i/80), "Texture002", gl_default_sprite_shader, 80, 80, 0.f, Size.x - 80, i);
        }

        for (int i = 80; i < Size.x; i += 80) {
            sprites.add_sprite("Sprite_Wall_Bottom_" + std::to_string(i/80), "Texture002", gl_default_sprite_shader, 80, 80, 0.f, i, 0);
            sprites.add_sprite("Sprite_Wall_Top_" + std::to_string(i/80), "Texture002", gl_default_sprite_shader, 80, 80, 0.f, i, Size.y - 80);
        }

        while (!glfwWindowShouldClose(window)) {
            glClear(GL_COLOR_BUFFER_BIT);

            defaultShaderProgram->use();
            textures.bind_all();

            sprites.move_all(cam_x, cam_y, cam_zero_x, cam_zero_y);
            player.rotate_all(180 - cam_rot);
            sprites.render_all();
            player.render_all();

            Sleep(1);
            glfwSwapBuffers(window);
            glfwPollEvents();
        }
        
    }

    glfwTerminate();
    return 0;
}