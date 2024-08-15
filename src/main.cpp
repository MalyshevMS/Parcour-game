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
#include "Resources/LvlParser.hpp"
#include "Online/Client.hpp"

#include "keys"

#include <iostream> // other libs
#include <string>
#include <cmath>
#include <vector>
#include <thread>
#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#pragma warning(disable: 4996)

// #define debug
// #define fullscreen

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

int pl_offset_x = 40;
int pl_offset_y = 280;
int pl_x = Size.x / 2 - pl_offset_x;
int pl_y = Size.y / 2 - pl_offset_y;
int pl2_x = 0;
int pl2_y = 0;
bool pl_on_floor = true;
bool pl_is_jumping = false;
bool pl_is_spidering = false;
vector <glm::vec2> pl_coords;

ResourceManager rm_main;

TexLoader tl_textures;

SprGroup sg_sprites;
SprGroup sg_player;
SprGroup sg_mobs;
SprGroup sg_player2;

Parser pars_main;

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
    if (pl_on_floor && !collides_ceiling()) {
        int height = pl_y + sv_jump_height;
        pl_is_jumping = true;
        pl_on_floor = false;
        while (pl_y < height && !collides_ceiling()) {
            pl_y += sv_jump_speed;
            sleep(1);
        }

        while (!collides_floor() && pl_y > 0 && !pl_is_spidering) {
            pl_y -= sv_jump_speed;
            sleep(1);
        }
        pl_on_floor = true;
        pl_is_jumping = false;
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

    if (key == KEY_G && action == GLFW_PRESS) {
        cout << "(" << pl2_x << ", " << pl2_y << ")" << endl;
    }
}

void fall() {
    if (!collides_floor() && !pl_is_spidering && !pl_is_jumping && pl_y > 0) pl_y -= sv_jump_speed;
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
        pl_is_spidering = true;
    } else pl_is_spidering = false;

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

    WSAData wsaData;
	WORD DLLVersion = MAKEWORD(2, 1);
	if (WSAStartup(DLLVersion, &wsaData) != 0) {
	    std::cout << "Error" << std::endl;
		return -1;
	}

    cout << "Renderer: " << glGetString(GL_RENDERER) << endl;
    cout << "OpenGL version: " << glGetString(GL_VERSION) << endl;

    glClearColor(0.19f, 0.61f, 0.61f, 1.f);

    {
        rm_main = ResourceManager(argv[0]);
        tl_textures = TexLoader(&rm_main);
        sg_sprites = SprGroup(&rm_main);
        sg_player = SprGroup(&rm_main);
        sg_player2 = SprGroup(&rm_main);
        sg_mobs = SprGroup(&rm_main);
        pars_main = Parser(&rm_main, &tl_textures, &sg_sprites);

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

        defaultShaderProgram->use();
        defaultShaderProgram->setInt("tex", 0);

        spriteShaderProgram->use();
        spriteShaderProgram->setInt("tex", 0);
        
        pars_main.parse_lvl("res/lvl/level.json", gl_sprite_shader, &gl_sprite_size);

        sg_player.add_sprite("Player", gl_sprite_shader, gl_sprite_size, gl_sprite_size, 0.f, pl_x, pl_y);
        sg_player2.add_sprite("Player", gl_sprite_shader, gl_sprite_size, gl_sprite_size, 0.f, pl2_x, pl2_y);


        string servercfg = rm_main.getFileStr("server.cfg");
        string ip = servercfg.substr(0, servercfg.find_first_of(':'));
        USHORT port = atoi(servercfg.substr(servercfg.find_first_of(':') + 1, servercfg.find_first_of(';')).c_str());
        
        Client cli(ip, port);

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

            cli.send_msg(to_string(pl_x) + "/" + to_string(pl_y) + ";");
            auto pl2_coords = cli.recv_msg();
            pl2_x = stoi(pl2_coords.substr(0, pl2_coords.find_first_of("/")));
            pl2_y = stoi(pl2_coords.substr(pl2_coords.find_first_of("/") + 1, pl2_coords.find_first_of(";")));

            fall();

            sg_player.rotate_all(180 - cam_rot);
            sg_player.set_pos(pl_x, pl_y);

            sg_player2.rotate_all(180 - cam_rot);
            sg_player2.set_pos(pl2_x, pl2_y);

            sg_sprites.render_all();
            sg_player.render_all();
            sg_mobs.render_all();
            sg_player2.render_all();

            sleep(1);
            glfwSwapBuffers(window);
            glfwPollEvents();
        }
        
        sg_sprites.delete_all();
        sg_player.delete_all();
        sg_mobs.delete_all();
        sg_player2.delete_all();
    }


    glfwTerminate();
    return 0;
}