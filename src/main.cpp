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
#include "Resources/SpriteGroup.hpp"

#include "keys"

#include <iostream> // other libs
#include <string>
#include <cmath>
#include <vector>
#include <thread>

// #define debug
#define fullscreen

using namespace std;

#ifndef fullscreen
    glm::ivec2 Size(1280, 720);
#else
    glm::ivec2 Size(3840, 2160);
#endif

// Eng vars
GLenum gl_texture_mode = GL_LINEAR; // GL_LINEAR or GL_NEAREST
string gl_default_shader = "DefaultShader"; // Name of default shader
string gl_sprite_shader = "SpriteShader"; // Name of default sprite shader
vector <string> gl_default_shader_path_list = {"res/shaders/vertex.cfg", "res/shaders/fragment.cfg"}; // List of paths to default shader
vector <string> gl_sprite_shader_path_list = {"res/shaders/vSprite.cfg", "res/shaders/fSprite.cfg"}; // List of paths to sprite shader
int gl_sprite_size = 80;

vector <string> tx_path_list = {
    "res/textures/wall.png",
    "res/textures/player.png",
    "res/textures/bullet.png"
}; // List of paths to textures

float cam_x = 0; // Camera X pos
float cam_y = 0; // Camera Y pos
float cam_rot = 180.f; // Camera rotation
float cam_mag = 1.f;
float cam_speed = 10.f;
float cam_mag_speed = 0.01f;
bool cam_locked = true;

int sv_max_speed = 10; // Max player movement speed
int sv_jump_speed = 5;
int sv_jump_height = 2 * gl_sprite_size;
int sv_gravity = 800;
bool sv_on_floor = true;
bool sv_is_jumping = false;
bool sv_is_spidering = false;

int pl_offset_x = 40;
int pl_offset_y = 280;
int pl_x = Size.x / 2 - pl_offset_x;
int pl_y = Size.y / 2 - pl_offset_y;

ResourceManager rm_main;

TexLoader tl_textures;

SprGroup sg_sprites;
SprGroup sg_player;
SprGroup sg_mobs;

void sizeHandler(GLFWwindow* win, int width, int height) {
    Size.x = width;
    Size.y = height;
    glViewport(0, 0, Size.x, Size.y);
}

bool collides_floor() {
    vector sprites_pos = sg_sprites.get_current_pos();
    for (int i = 0; i < sprites_pos.size(); i++) {
        if (abs(pl_y - (sprites_pos[i].y + gl_sprite_size)) <= 0.f && abs(pl_x - sprites_pos[i].x) < gl_sprite_size) return true;
    }
    return false;
}

bool collides_ceiling() {
    vector sprites_pos = sg_sprites.get_current_pos();
    for (int i = 0; i < sprites_pos.size(); i++) {
        if (abs((pl_y + gl_sprite_size) - sprites_pos[i].y) <= 0.1f && abs(pl_x - sprites_pos[i].x) < gl_sprite_size) return true;
    }
    return false;
}

int collides_left() {
    vector sprites_pos = sg_sprites.get_current_pos();
    for (int i = 0; i < sprites_pos.size(); i++) {
        if (abs(pl_x - (sprites_pos[i].x + gl_sprite_size)) <= 0.f && abs(pl_y - sprites_pos[i].y) < gl_sprite_size) return true;
    }
    return false;
}


int collides_right() {
    vector sprites_pos = sg_sprites.get_current_pos();
    for (int i = 0; i < sprites_pos.size(); i++) {
        if (abs((pl_x + gl_sprite_size) - sprites_pos[i].x) <= 0.f && abs(pl_y - sprites_pos[i].y) < gl_sprite_size) return true;
    }
    return false;
}

void jump() {
    if (sv_on_floor && !collides_ceiling()) {
        int height = pl_y + sv_jump_height;
        sv_is_jumping = true;
        sv_on_floor = false;
        while (pl_y < height && !collides_ceiling()) {
            pl_y += sv_jump_speed;
            sleep(1);
        }

        while (!collides_floor() && pl_y > 0 && !sv_is_spidering) {
            pl_y -= sv_jump_speed;
            sleep(1);
        }
        sv_on_floor = true;
        sv_is_jumping = false;
    }
}

bool sg_collision(SprGroup& sg1, SprGroup& sg2) {
    for (auto i : sg1.get_current_pos()) {
        for (auto j : sg2.get_current_pos()) {
            if (abs(i.x - j.x) <= gl_sprite_size && abs(i.y - j.y) <= gl_sprite_size) return true;
        }
    }
    return false;
}

void onceKeyHandler(GLFWwindow* win, int key, int scancode, int action, int mode) {
    if (key == KEY_LEFT_ALT && action == GLFW_PRESS) {
        cam_locked ? cam_locked = false : cam_locked = true;
    }
}

void fall() {
    if (!collides_floor() && !sv_is_spidering && !sv_is_jumping && pl_y > 0) pl_y -= sv_jump_speed;
}

void detect_fail() {}

void keyHandler(GLFWwindow* win) {
    if (glfwGetKey(win, KEY_A) == GLFW_PRESS && !collides_left()) {
        pl_x -= sv_max_speed;
        if (cam_locked) cam_x -= sv_max_speed;
    }

    if (glfwGetKey(win, KEY_D) == GLFW_PRESS && !collides_right()) {
        pl_x += sv_max_speed;
        if (cam_locked) cam_x += sv_max_speed;
    }

    if (glfwGetKey(win, KEY_LEFT_CONTROL) == GLFW_PRESS) {
        sv_max_speed = 20;
    } else if (glfwGetKey(win, KEY_LEFT_CONTROL) == GLFW_RELEASE && glfwGetKey(win, KEY_LEFT_SHIFT) != GLFW_PRESS) sv_max_speed = 10;

    if (glfwGetKey(win, KEY_LEFT_SHIFT) == GLFW_PRESS) {
        sv_max_speed = 5;
    } else if (glfwGetKey(win, KEY_LEFT_SHIFT) == GLFW_RELEASE && glfwGetKey(win, KEY_LEFT_CONTROL) != GLFW_PRESS) sv_max_speed = 10;

    if (glfwGetKey(win, KEY_SPACE) == GLFW_PRESS) {
        std::thread t(jump);
        t.detach();
    }

    if (glfwGetKey(win, KEY_W) == GLFW_PRESS && collides_ceiling()) {
        sv_is_spidering = true;
    } else sv_is_spidering = false;

    if (glfwGetKey(win, KEY_EQUAL) == GLFW_PRESS && cam_mag - cam_mag_speed >= 2.f) {
        cam_mag -= cam_mag_speed;
    }

    if (glfwGetKey(win, KEY_MINUS) == GLFW_PRESS) {
        cam_mag += cam_mag_speed;
    }

    if (glfwGetKey(win, KEY_UP) == GLFW_PRESS) {
        cam_y += cam_speed;
    }

    if (glfwGetKey(win, KEY_DOWN) == GLFW_PRESS) {
        cam_y -= cam_speed;
    }

    if (glfwGetKey(win, KEY_RIGHT) == GLFW_PRESS) {
        cam_x += cam_speed;
    }

    if (glfwGetKey(win, KEY_LEFT) == GLFW_PRESS) {
        cam_x -= cam_speed;
    }

    if (glfwGetKey(win, KEY_F1) == GLFW_PRESS) {
        cam_x = 0;
        cam_y = 0;
    }
}

int main(int argc, char const *argv[]) {
    string comand_line;

    #ifdef debug
        do {
            cout << "Minimal2D > ";
            cin >> comand_line;

            if (comand_line == "exit") { exit(0); }
        } while (comand_line != "play");
    #endif

    if (!glfwInit()) {
        cerr << "glfwInit failed!" << endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    glfwWindowHint(GLFW_MAXIMIZED, GL_TRUE);

    #ifdef fullscreen
        GLFWwindow* window = glfwCreateWindow(Size.x, Size.y, "ParCour game", glfwGetPrimaryMonitor(), nullptr);
    #else 
        GLFWwindow* window = glfwCreateWindow(Size.x, Size.y, "ParCour game", nullptr, nullptr);
    #endif

    if (!window) {
        cerr << "glfwCreateWindow failed!" << endl;
        glfwTerminate();
        return -1;
    }

    glfwSetKeyCallback(window, onceKeyHandler);
    glfwSetWindowSizeCallback(window, sizeHandler);

    glfwMakeContextCurrent(window);

    if (!gladLoadGL()) {
        cerr << "Can't load GLAD!" << endl;
    }

    cout << "Renderer: " << glGetString(GL_RENDERER) << endl;
    cout << "OpenGL version: " << glGetString(GL_VERSION) << endl;

    glClearColor(0.19f, 0.61f, 0.61f, 1.f);

    {
        rm_main = ResourceManager(argv[0]);
        tl_textures = TexLoader(&rm_main);
        sg_sprites = SprGroup(&rm_main);
        sg_player = SprGroup(&rm_main);
        sg_mobs = SprGroup(&rm_main);

        auto defaultShaderProgram = rm_main.loadShaders(gl_default_shader, gl_default_shader_path_list[0], gl_default_shader_path_list[1]);

        if (!defaultShaderProgram) {
            cerr << "Can't create shader program!" << endl;
            return -1;
        }

        auto spriteShaderProgram = rm_main.loadShaders(gl_sprite_shader, gl_sprite_shader_path_list[0], gl_sprite_shader_path_list[1]);

        if (!spriteShaderProgram) {
            cerr << "Can't create SpriteShader" << endl;
            return -1;
        }

        tl_textures.add_texture("Wall", tx_path_list[0]);
        tl_textures.add_texture("Player", tx_path_list[1]);
        tl_textures.add_texture("Bullet", tx_path_list[2]);

        defaultShaderProgram->use();
        defaultShaderProgram->setInt("tex", 0);

        spriteShaderProgram->use();
        spriteShaderProgram->setInt("tex", 0);

        sg_player.add_sprite("Sprite_Player", "Player", gl_sprite_shader, gl_sprite_size, gl_sprite_size, 0.f, pl_x, pl_y);

        for (int i = 0; i < 3840; i += gl_sprite_size) {
            sg_sprites.add_sprite("Sprite_Wall_Bottom_" + to_string(i/gl_sprite_size), "Wall", gl_sprite_shader, gl_sprite_size, gl_sprite_size, 0.f, i, 0);
        }

        sg_sprites.add_sprite("Sprite_Wall_Obstacle_0", "Wall", gl_sprite_shader, gl_sprite_size, gl_sprite_size, 0.f, gl_sprite_size * 2, gl_sprite_size);
        sg_sprites.add_sprite("Sprite_Wall_Obstacle_1", "Wall", gl_sprite_shader, gl_sprite_size, gl_sprite_size, 0.f, gl_sprite_size * 4, gl_sprite_size * 3);
        sg_sprites.add_sprite("Sprite_Wall_Obstacle_2", "Wall", gl_sprite_shader, gl_sprite_size, gl_sprite_size, 0.f, gl_sprite_size * 2, gl_sprite_size * 5);
        sg_sprites.add_sprite("Sprite_Wall_Obstacle_3", "Wall", gl_sprite_shader, gl_sprite_size, gl_sprite_size, 0.f, gl_sprite_size * 4, gl_sprite_size * 7);
        sg_sprites.add_sprite("Sprite_Wall_Obstacle_4", "Wall", gl_sprite_shader, gl_sprite_size, gl_sprite_size, 0.f, gl_sprite_size * 2, gl_sprite_size * 9);

        sg_sprites.add_sprite("Sprite_Wall_Ceiling_0", "Wall", gl_sprite_shader, gl_sprite_size, gl_sprite_size, 0.f, gl_sprite_size * 2, gl_sprite_size * 7);
        sg_sprites.add_sprite("Sprite_Wall_Ceiling_1", "Wall", gl_sprite_shader, gl_sprite_size, gl_sprite_size, 0.f, gl_sprite_size * 3, gl_sprite_size * 7);
        sg_sprites.add_sprite("Sprite_Wall_Ceiling_2", "Wall", gl_sprite_shader, gl_sprite_size, gl_sprite_size, 0.f, gl_sprite_size * 4, gl_sprite_size * 7);
        sg_sprites.add_sprite("Sprite_Wall_Ceiling_3", "Wall", gl_sprite_shader, gl_sprite_size, gl_sprite_size, 0.f, gl_sprite_size * 5, gl_sprite_size * 7);
        sg_sprites.add_sprite("Sprite_Wall_Ceiling_4", "Wall", gl_sprite_shader, gl_sprite_size, gl_sprite_size, 0.f, gl_sprite_size * 6, gl_sprite_size * 7);

        while (!glfwWindowShouldClose(window)) {
            glClear(GL_COLOR_BUFFER_BIT);

            keyHandler(window);

            float projMat_right  = Size.x * cam_mag + cam_x;
            float projMat_top    = Size.y * cam_mag + cam_y;
            float projMat_left   = - Size.x * (cam_mag - 1) + cam_x;
            float projMat_bottom = - Size.y * (cam_mag - 1) + cam_y;

            glm::mat4 projMat = glm::ortho(projMat_left, projMat_right, projMat_bottom, projMat_top, -100.f, 100.f);

            defaultShaderProgram->setMat4("projMat", projMat);
            spriteShaderProgram->setMat4("projMat", projMat);

            defaultShaderProgram->use();
            tl_textures.bind_all();

            fall();

            sg_player.rotate_all(180 - cam_rot);
            sg_player.set_pos(pl_x, pl_y);

            sg_sprites.render_all();
            sg_player.render_all();
            sg_mobs.render_all();

            sleep(1);
            glfwSwapBuffers(window);
            glfwPollEvents();
        }
        
        sg_sprites.delete_all();
        sg_player.delete_all();
        sg_mobs.delete_all();
    }


    glfwTerminate();
    return 0;
}