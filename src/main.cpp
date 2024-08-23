// #define debug
// #define fullscreen
// #define online

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
#include "Resources/Parser.hpp"
#include "Other/KeyHandler.hpp"

#ifdef online
    #include "Online/Client.hpp"
#endif

#include "keys"

#include <iostream> // other libs
#include <string>
#include <cmath>
#include <vector>
#include <thread>

#ifdef online
    #pragma comment(lib, "ws2_32.lib")
    #include <winsock2.h>
    #pragma warning(disable: 4996)
#endif

using namespace std;

#ifndef fullscreen
    glm::ivec2 Size(1280, 720);
#else
    glm::ivec2 Size(3840, 2160);
#endif

// Engine vars
GLenum gl_texture_mode = GL_LINEAR; // GL_LINEAR or GL_NEAREST
string gl_default_shader = "DefaultShader"; // Name of default shader
string gl_sprite_shader = "SpriteShader"; // Name of default sprite shader
string gl_default_shader_path_list[] = {
    "res/shaders/vertex.cfg", "res/shaders/fragment.cfg"
}; // List of paths to default shader
string gl_sprite_shader_path_list[] = {
    "res/shaders/vSprite.cfg", "res/shaders/fSprite.cfg"
}; // List of paths to sprite shader
int gl_sprite_size = 80; // Default size of a sprite

float cam_x = 0; // Camera X pos
float cam_y = 0; // Camera Y pos
float cam_rot = 180.f; // Camera rotation
float cam_mag = 1.f; // Current camera magnifying
float cam_speed = 10.f; // Camera movement speed
float cam_mag_speed = 0.01f; // Camera magnifying speed
bool cam_locked = true; // Camera lock (to player) flag

int sv_max_speed = 10; // Max player movement speed
int sv_jump_speed = 5; // Player(s) jump speed
int sv_jump_height = 160; // Max player jump height
float sv_gravity = 9.80665f;

int pl_x = 0; // Player 1 X
int pl_y = 80; // Player 1 Y
int pl2_x = 0; // Player 2 X
int pl2_y = 0; // Player 2 Y
bool pl_jumping = false; // Player jumping flag
bool pl_spidering = false; // Player spider mode flag
bool pl_noclip = false;

ResourceManager rm_main; // Main Resource manager

TexLoader tl_main; // Main Texture loader

SprGroup sg_sprites; // Group for obstacles, walls, etc.
SprGroup sg_player; // Group for Player 1
SprGroup sg_player2; // Group for Player 2

Parser pars_main; // Main parser

KeyHandler kh_main;

float __ticks;
float __ticks2;

void sizeHandler(GLFWwindow* win, int width, int height) {
    Size.x = width;
    Size.y = height;
    glViewport(0, 0, Size.x, Size.y);
}

bool collides_floor(int epsilon = 2, int epsilon2 = 0) {
    if (pl_y <= 0) return true;
    vector sprites_pos = sg_sprites.get_current_pos();
    for (int i = 0; i < sprites_pos.size(); i++) {
        if (pl_y - (sprites_pos[i].y + gl_sprite_size) <= -epsilon && abs(pl_x - sprites_pos[i].x) < gl_sprite_size - epsilon2) return true;
    }
    return false;
}

bool collides_ceiling(int epsilon = 0, int epsilon2 = 0) {
    vector sprites_pos = sg_sprites.get_current_pos();
    for (int i = 0; i < sprites_pos.size(); i++) {
        if (abs((pl_y + gl_sprite_size) - sprites_pos[i].y) <= -epsilon && abs(pl_x - sprites_pos[i].x) < gl_sprite_size - epsilon2) return true;
    }
    return false;
}

bool collides_left(int epsilon = 0, int epsilon2 = 2) {
    vector sprites_pos = sg_sprites.get_current_pos();
    for (int i = 0; i < sprites_pos.size(); i++) {
        if (pl_x - (sprites_pos[i].x + gl_sprite_size) <= -epsilon && pl_x - (sprites_pos[i].x + gl_sprite_size) > -2 * gl_sprite_size && abs(pl_y - sprites_pos[i].y) < gl_sprite_size - epsilon2) return true;
    }
    return false;
}

bool collides_right(int epsilon = 0, int epsilon2 = 2) {
    vector sprites_pos = sg_sprites.get_current_pos();
    for (int i = 0; i < sprites_pos.size(); i++) {
        if (abs((pl_x + gl_sprite_size) - sprites_pos[i].x) <= -epsilon && abs(pl_y - sprites_pos[i].y) < gl_sprite_size - epsilon2) return true;
    }
    return false;
}

void prevent_clipping() {
    if (!pl_noclip) {
        if (collides_floor(1) && !collides_ceiling()) pl_y++;
        else if (collides_left(1) && !collides_right()) {
            pl_x += gl_sprite_size / 2;
            cam_x += gl_sprite_size / 2;
        }
    }
}

auto g = [](float ticks, float tick_time = 50.f){ return (ticks / tick_time) * sv_gravity; };

void jump() {
    if ((collides_floor() || collides_floor(0)) && !collides_ceiling()) {
        __ticks2 = 0.f;
        int height = pl_y + sv_jump_height;
        pl_jumping = true;
        while (pl_y < height && !collides_ceiling()) {
            pl_y += -g(__ticks2) + sv_gravity;
            __ticks2++;
            sleep(1);
        }
        pl_jumping = false;
        __ticks2 = 0;
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

void fall() {
    if (!collides_floor() && !collides_floor(0) && !pl_spidering && !pl_jumping && pl_y > 0) {
        if (g(__ticks) < sv_max_speed) pl_y -= g(__ticks);
        else pl_y -= sv_max_speed;
        __ticks++;
    } else __ticks = 0;
}

void detect_fail() {

}

void onceKeyHandler(GLFWwindow* win, int key, int scancode, int action, int mode) {
    if (key == KEY_LEFT_ALT && action == GLFW_PRESS) {
        cam_locked ? cam_locked = false : cam_locked = true;
    }

    if (key == KEY_Q && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(win, GL_TRUE);
    }
}

void keyHandler(GLFWwindow* win) {
    if (glfwGetKey(win, KEY_A) == GLFW_PRESS && (pl_noclip ? true : !collides_left())) {
        pl_x -= sv_max_speed;
        if (cam_locked) cam_x -= sv_max_speed;
    }

    if (glfwGetKey(win, KEY_D) == GLFW_PRESS && (pl_noclip ? true : !collides_right())) {
        pl_x += sv_max_speed;
        if (cam_locked) cam_x += sv_max_speed;
    }

    if (glfwGetKey(win, KEY_LEFT_CONTROL) == GLFW_PRESS) {
        sv_max_speed = 20;
    } else if (glfwGetKey(win, KEY_LEFT_CONTROL) == GLFW_RELEASE && glfwGetKey(win, KEY_LEFT_SHIFT) != GLFW_PRESS) sv_max_speed = 10;

    if (glfwGetKey(win, KEY_LEFT_SHIFT) == GLFW_PRESS) {
        sv_max_speed = 5;
    } else if (glfwGetKey(win, KEY_LEFT_SHIFT) == GLFW_RELEASE && glfwGetKey(win, KEY_LEFT_CONTROL) != GLFW_PRESS) sv_max_speed = 10;
}

int main(int argc, char const *argv[]) {
    if (argc >= 2 && argv[1] == "-res") {
        if (argc < 3) {
            cerr << "Using: -res widthxheight" << endl;
            return -1;
        } else {
            string res = argv[2];
            Size.x = stoi(res.substr(0, res.find_first_of('x')));
            Size.y = stoi(res.substr(res.find_first_of('x') + 1));

            pl_x = Size.x / 2 - 40;
            pl_y = Size.y / 2 - 280;
        }
    }

    string comand_line;

    #ifdef debug // Debugger console (not ready yet)
        do {
            cout << "Minimal2D > ";
            cin >> comand_line;

            if (comand_line == "exit") { exit(0); }
        } while (comand_line != "play");
    #endif

    if (!glfwInit()) { // Check for GLFW
        cerr << "glfwInit failed!" << endl;
        return -1;
    }

    // GLFW window settings
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_MAXIMIZED, 0);

    #ifdef fullscreen // Creating window (fullscreen or windowed)
        GLFWwindow* window = glfwCreateWindow(Size.x, Size.y, "ParCour game", glfwGetPrimaryMonitor(), nullptr);
    #else 
        GLFWwindow* window = glfwCreateWindow(Size.x, Size.y, "ParCour game", nullptr, nullptr);
    #endif

    if (!window) { // Checking for creating window
        cerr << "glfwCreateWindow failed!" << endl;
        glfwTerminate();
        return -1;
    }

    glfwSetKeyCallback(window, onceKeyHandler); // Setting Key Handler
    glfwSetWindowSizeCallback(window, sizeHandler); // Setting Size Handler

    glfwMakeContextCurrent(window);

    if (!gladLoadGL()) { // Checking for GLAD
        cerr << "Can't load GLAD!" << endl;
    }

    #ifdef online // Checking for WSA (if online mode is on)
        WSAData wsaData;
        WORD DLLVersion = MAKEWORD(2, 1);
        if (WSAStartup(DLLVersion, &wsaData) != 0) {
            std::cout << "Error" << std::endl;
        	return -1;
        }
    #endif

    cout << "Renderer: " << glGetString(GL_RENDERER) << endl; // Displaying OpenGL info
    cout << "OpenGL version: " << glGetString(GL_VERSION) << endl;

    glClearColor(0.19f, 0.61f, 0.61f, 1.f); // Sky color (in vec4 format)

    {
        rm_main = ResourceManager(argv[0]); // Binding all classes together
        tl_main = TexLoader(&rm_main);
        sg_sprites = SprGroup(&rm_main);
        sg_player = SprGroup(&rm_main);
        sg_player2 = SprGroup(&rm_main);
        pars_main = Parser(&rm_main, &tl_main, &sg_sprites);
        kh_main = KeyHandler(window);

        // Creating and checking for default shader
        auto defaultShaderProgram = rm_main.loadShaders(gl_default_shader, gl_default_shader_path_list[0], gl_default_shader_path_list[1]);
        if (!defaultShaderProgram) {
            cerr << "Can't create shader program!" << endl;
            return -1;
        }

        // Creating and checking for sprite shader
        auto spriteShaderProgram = rm_main.loadShaders(gl_sprite_shader, gl_sprite_shader_path_list[0], gl_sprite_shader_path_list[1]);
        if (!spriteShaderProgram) {
            cerr << "Can't create SpriteShader" << endl;
            return -1;
        }

        // Using default shader
        defaultShaderProgram->use();
        defaultShaderProgram->setInt("tex", 0);

        // Using sprite shader
        spriteShaderProgram->use();
        spriteShaderProgram->setInt("tex", 0);

        pars_main.parse_lvl("res/lvl/level.json", &gl_sprite_size); // Parsing level

        sg_player.add_sprite("Player", gl_sprite_shader, gl_sprite_size, gl_sprite_size, 0.f, pl_x, pl_y); // Adding Player 1 sprite

        #ifdef online
            sg_player2.add_sprite("Player", gl_sprite_shader, gl_sprite_size, gl_sprite_size, 0.f, pl2_x, pl2_y); // Adding Player 2 sprite

            // Parsing serffer.cfg
            string servercfg = rm_main.getFileStr("server.cfg");
            string ip = servercfg.substr(0, servercfg.find_first_of(':'));
            unsigned short port = atoi(servercfg.substr(servercfg.find_first_of(':') + 1, servercfg.find_first_of(';')).c_str());
            
            Client cli(ip, port); // Creating client
        #endif

        // Binding keys (some functions are still in older Key Handler)
        kh_main.bind(KEY_SPACE, [](){ if (!pl_jumping) { std::thread t(jump); t.detach(); } });
        kh_main.bind('W', [](){ if (collides_ceiling()) pl_spidering = true; }, [](){ pl_spidering = false; });
        kh_main.bind('0', [](){ cam_mag -= cam_mag_speed; });
        kh_main.bind('9', [](){ cam_mag += cam_mag_speed; });
        kh_main.bind(KEY_UP, [](){ cam_y += cam_speed; });
        kh_main.bind(KEY_DOWN, [](){ cam_y -= cam_speed; });
        kh_main.bind(KEY_RIGHT, [](){ cam_x += cam_speed; });
        kh_main.bind(KEY_LEFT, [](){ cam_x -= cam_speed; });
        kh_main.bind(KEY_F1, [](){ cam_x = 0; cam_y = 0; });

        while (!glfwWindowShouldClose(window)) { // Main game loop
            glClear(GL_COLOR_BUFFER_BIT);

            keyHandler(window); // Setting key handler
            kh_main.use();

            // Projection matrix variables
            float projMat_right  = Size.x * cam_mag + cam_x;
            float projMat_top    = Size.y * cam_mag + cam_y;
            float projMat_left   = - Size.x * (cam_mag - 1) + cam_x;
            float projMat_bottom = - Size.y * (cam_mag - 1) + cam_y;

            glm::mat4 projMat = glm::ortho(projMat_left, projMat_right, projMat_bottom, projMat_top, -100.f, 100.f); // Setting projection matrix

            // Using projection matrix
            defaultShaderProgram->setMat4("projMat", projMat);
            spriteShaderProgram->setMat4("projMat", projMat);

            defaultShaderProgram->use(); // Using default shader
            tl_main.bind_all(); // Binding all textures

            #ifdef online // Sending and recieving position
                cli.send_msg(to_string(pl_x) + "/" + to_string(pl_y) + ";");
                auto pl2_coords = cli.recv_msg();
                pl2_x = stoi(pl2_coords.substr(0, pl2_coords.find_first_of("/")));
                pl2_y = stoi(pl2_coords.substr(pl2_coords.find_first_of("/") + 1, pl2_coords.find_first_of(";")));
            #endif

            fall(); // Always falling down
            prevent_clipping();

            sg_player.rotate_all(180 - cam_rot); // Setting rotation (Player 1)
            sg_player.set_pos(pl_x, pl_y); // Setting position (Player 1)

            sg_player2.rotate_all(180 - cam_rot); // Setting rotation (Player 2)
            sg_player2.set_pos(pl2_x, pl2_y); // Setting position (Player 2)

            // Rendering all sprites
            sg_sprites.render_all();
            sg_player.render_all();
            sg_player2.render_all();

            sleep(1); // 1ms delay
            glfwSwapBuffers(window); // Swapping front and back buffers
            glfwPollEvents(); // Polling events
        }
        
        // Deleting all sprites from all groups
        sg_sprites.delete_all(); 
        sg_player.delete_all();
        sg_player2.delete_all();
    }


    glfwTerminate(); 
    return 0; // Exiting programm
}