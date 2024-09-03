#define debug
// #define fullscreen
#define online

#include <glad/glad.h> // OpenGL libs
#include <GLFW/glfw3.h>

#include <glm/vec2.hpp> // glm libs
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Renderer/ShaderProgram.hpp" // my libs
#include "Renderer/Texture2D.hpp"
#include "Renderer/Sprite.hpp"
#include "Renderer/AnimatedSprite.hpp"
#include "Resources/ResourceManager.hpp"
#include "Resources/TextureLoader.hpp"
#include "Resources/SpriteGroup.hpp"
#include "Resources/AnimSpriteGroup.hpp"
#include "Resources/Parser.hpp"
#include "Other/KeyHandler.hpp"
#include "Online/NetHandler.hpp"

#include "Variables/OpenGL.hpp" // variables
#include "Variables/Camera.hpp"
#include "Variables/Client.hpp"
#include "Variables/Player.hpp"


#include "keys"

#include <iostream> // other libs
#include <string>
#include <cmath>
#include <vector>
#include <thread>

using namespace std;
using DPair = pair<string, uint64_t>;

#ifndef fullscreen
    glm::ivec2 Size(1280, 720);
#else
    glm::ivec2 Size(3840, 2160);
#endif

OpenGL gl;
Camera cam;
Client cl;
Player pl;
Player pl2;

ResourceManager rm_main; // Main Resource manager

TexLoader tl_main; // Main Texture loader

SprGroup sg_sprites; // Group for obstacles, walls, etc.
AnimSprGroup sg_player; // Group for Player 1
AnimSprGroup sg_player2; // Group for Player 2

Parser pars_main; // Main parser

KeyHandler kh_main; // Main Key Handler

float __ticks;
float __ticks2;

void sizeHandler(GLFWwindow* win, int width, int height) {
    Size.x = width;
    Size.y = height;
    glViewport(0, 0, Size.x, Size.y);
}

bool collides_floor(int epsilon = 2, int epsilon2 = 0) {
    if (pl.y <= 0) return true;
    vector sprites_pos = sg_sprites.get_current_pos();
    for (int i = 0; i < sprites_pos.size(); i++) {
        if (pl.y - (sprites_pos[i].y + gl.sprite_size) <= -epsilon && abs(pl.x - sprites_pos[i].x) < gl.sprite_size - epsilon2) return true;
    }
    return false;
}

bool collides_ceiling(int epsilon = 0, int epsilon2 = 0) {
    vector sprites_pos = sg_sprites.get_current_pos();
    for (int i = 0; i < sprites_pos.size(); i++) {
        if (abs((pl.y + gl.sprite_size) - sprites_pos[i].y) <= -epsilon && abs(pl.x - sprites_pos[i].x) < gl.sprite_size - epsilon2) return true;
    }
    return false;
}

bool collides_left(int epsilon = 0, int epsilon2 = 2) {
    vector sprites_pos = sg_sprites.get_current_pos();
    for (int i = 0; i < sprites_pos.size(); i++) {
        if (pl.x - (sprites_pos[i].x + gl.sprite_size) <= -epsilon && pl.x - (sprites_pos[i].x + gl.sprite_size) > -2 * gl.sprite_size && abs(pl.y - sprites_pos[i].y) < gl.sprite_size - epsilon2) return true;
    }
    return false;
}

bool collides_right(int epsilon = 0, int epsilon2 = 2) {
    vector sprites_pos = sg_sprites.get_current_pos();
    for (int i = 0; i < sprites_pos.size(); i++) {
        if (abs((pl.x + gl.sprite_size) - sprites_pos[i].x) <= -epsilon && abs(pl.y - sprites_pos[i].y) < gl.sprite_size - epsilon2) return true;
    }
    return false;
}

void prevent_clipping() {
    if (!pl.noclip) {
        if (collides_floor(1) && !collides_ceiling()) pl.y++;
        else if (collides_left(1) && !collides_right()) {
            pl.x += gl.sprite_size / 2;
            cam.x += gl.sprite_size / 2;
        }
    }
}

auto g = [](float ticks, float tick_time = 50.f){ return (ticks / tick_time) * cl.gravity; };

void jump() {
    if ((collides_floor() || collides_floor(0)) && !collides_ceiling()) {
        switch (pl.look) {
            case l_left: { pl.current_anim = "jleft"; break; }
            case l_right: { pl.current_anim = "jright"; break; }
        }
        __ticks2 = 0.f;
        int height = pl.y + cl.jump_height;
        pl.jumping = true;
        while (pl.y < height && !collides_ceiling()) {
            pl.y += -g(__ticks2) + cl.gravity;
            __ticks2++;
            sleep(1);
        }
        pl.jumping = false;
        __ticks2 = 0;
    }
}

bool sg_collision(SprGroup& sg1, SprGroup& sg2) {
    for (auto i : sg1.get_current_pos()) {
        for (auto j : sg2.get_current_pos()) {
            if (abs(i.x - j.x) <= gl.sprite_size && abs(i.y - j.y) <= gl.sprite_size) return true;
        }
    }
    return false;
}

void fall() {
    if (!collides_floor() && !collides_floor(0) && !pl.spidering && !pl.jumping && pl.y > 0) {
        if (g(__ticks) < cl.max_speed) pl.y -= g(__ticks);
        else pl.y -= cl.max_speed;
        __ticks++;
    } else __ticks = 0;
}

void detect_fail() {

}

void onceKeyHandler(GLFWwindow* win, int key, int scancode, int action, int mode) {
    if (key == KEY_LEFT_ALT && action == GLFW_PRESS) {
        cam.locked ? cam.locked = false : cam.locked = true;
    }

    if (key == KEY_Q && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(win, GL_TRUE);
    }

    if(key == KEY_B && action == GLFW_PRESS) {
        pl.noclip ? pl.noclip = false : pl.noclip = true;
    }
}

void set_stand_anim() {
    if (!pl.moving && !pl.jumping) {
        switch (pl.look) {
            case l_left: { pl.current_anim = "stand_left"; break; }
            case l_right: { pl.current_anim = "stand_right"; break; }
        }
    }
}

void netloop(NetHandler* cli) {
    while (true) {
        cli->send_msg(to_string(pl.x) + "/" + to_string(pl.y) + "/" + pl.current_anim);
        auto msg = cli->recv_msg();
        pl2.x = stoi(msg.substr(0, msg.find_first_of("/")));
        pl2.y = stoi(msg.substr(msg.find_first_of("/") + 1, msg.find_last_of("/")));
        pl2.current_anim = msg.substr(msg.find_last_of("/") + 1);
    }
}

void keyHandler(GLFWwindow* win) {
    if (glfwGetKey(win, KEY_A) == GLFW_PRESS && (pl.noclip ? true : !collides_left())) {
        pl.moving = true;
        pl.look = l_left;
        if (collides_floor() || collides_floor(0)) pl.current_anim = "mleft";
        pl.x -= cl.max_speed;
        if (cam.locked) cam.x -= cl.max_speed;
    }

    if (glfwGetKey(win, KEY_D) == GLFW_PRESS && (pl.noclip ? true : !collides_right())) {
        pl.moving = true;
        pl.look = l_right;
        if (collides_floor() || collides_floor(0)) pl.current_anim = "mright";
        pl.x += cl.max_speed;
        if (cam.locked) cam.x += cl.max_speed;
    }

    if (glfwGetKey(win, KEY_D) == GLFW_RELEASE && glfwGetKey(win, KEY_A) == GLFW_RELEASE) {
        pl.moving = false;
    }

    if (glfwGetKey(win, KEY_LEFT_CONTROL) == GLFW_PRESS) {
        cl.max_speed = 20;
    } else if (glfwGetKey(win, KEY_LEFT_CONTROL) == GLFW_RELEASE && glfwGetKey(win, KEY_LEFT_SHIFT) != GLFW_PRESS) cl.max_speed = 10;

    if (glfwGetKey(win, KEY_LEFT_SHIFT) == GLFW_PRESS) {
        cl.max_speed = 5;
    } else if (glfwGetKey(win, KEY_LEFT_SHIFT) == GLFW_RELEASE && glfwGetKey(win, KEY_LEFT_CONTROL) != GLFW_PRESS) cl.max_speed = 10;
}

int main(int argc, char const *argv[]) {
    // Engine vars
    gl.texture_mode = GL_LINEAR;
    gl.default_shader = "DefaultShader";
    gl.sprite_shader = "SpriteShader";
    gl.default_shader_path_list; // Please change this value inside of the structure
    gl.sprite_shader_path_list; // Please change this value inside of the structure
    gl.sprite_size = 80;

    cam.x = pl.x - Size.x / 2;
    cam.y = pl.y - gl.sprite_size;
    cam.rot = 180.f;
    cam.mag = 1.f;
    cam.speed = 10.f;
    cam.mag_speed = 0.01f;
    cam.locked = true;

    cl.max_speed = 10;
    cl.jump_speed = 5;
    cl.jump_height = 160;
    cl.gravity = 9.80665f;
    cl.server = false;

    pl.x = 0;
    pl.y = 80;
    pl2.x = 0;
    pl2.y = 0;
    pl.look = l_left;
    pl.current_anim = "stand_right";
    pl.moving = false;
    pl.jumping = false;
    pl.spidering = false;
    pl.noclip = false;

    string comand_line;

    #ifdef debug // Debugger console (not ready yet)
        do {
            cout << "Minimal2D > ";
            cin >> comand_line;

            if (comand_line == "exit") { exit(0); }
            else if (comand_line == "mkhost") {
                cl.server = true;
                cout << "Your role: Server" << endl;
            } else if (comand_line == "rmhost") {
                cl.server = false;
                cout << "Your role: Client" << endl;
            }
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

    cout << "Renderer: " << glGetString(GL_RENDERER) << endl; // Displaying OpenGL info
    cout << "OpenGL version: " << glGetString(GL_VERSION) << endl;

    glClearColor(0.19f, 0.61f, 0.61f, 1.f); // Sky color (in vec4 format)

    {
        rm_main = ResourceManager(argv[0]); // Binding all classes together
        tl_main = TexLoader(&rm_main);
        sg_sprites = SprGroup(&rm_main);
        sg_player = AnimSprGroup(&rm_main);
        sg_player2 = AnimSprGroup(&rm_main);
        pars_main = Parser(&rm_main, &tl_main, &sg_sprites);
        kh_main = KeyHandler(window);

        // Creating and checking for default shader
        auto defaultShaderProgram = rm_main.loadShaders(gl.default_shader, gl.default_shader_path_list[0], gl.default_shader_path_list[1]);
        if (!defaultShaderProgram) {
            cerr << "Can't create shader program!" << endl;
            return -1;
        }

        // Creating and checking for sprite shader
        auto spriteShaderProgram = rm_main.loadShaders(gl.sprite_shader, gl.sprite_shader_path_list[0], gl.sprite_shader_path_list[1]);
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

        pars_main.parse_lvl("res/lvl/level.json", &gl.sprite_size); // Parsing level
        tl_main.add_textures_from_atlas("Player", "res/textures/player.png", { "stand_right", "stand_left", "walk1_right", "walk1_left", "walk2_right", "walk2_left", "jump_right", "jump_left" }, glm::vec2(16, 16));

        sg_player.add_sprite("Player", "default", gl.sprite_shader, gl.sprite_size, gl.sprite_size, 0.f, pl.x, pl.y); // Adding Player 1 sprite

        sg_player.add_animation(0, "mleft", { DPair("walk1_left", 100), DPair("walk2_left", 100) });
        sg_player.add_animation(0, "mright", { DPair("walk1_right", 100), DPair("walk2_right", 100) });
        sg_player.add_animation(0, "jleft", { DPair("jump_left", 700) });
        sg_player.add_animation(0, "jright", { DPair("jump_right", 700) });
        sg_player.add_animation(0, "stand_right", { DPair("stand_right", 1) });
        sg_player.add_animation(0, "stand_left", { DPair("stand_left", 1) });

        pl.current_anim = "stand_left";
        pl2.current_anim = "stand_left";

        #ifdef online
            sg_player2.add_sprite("Player", "default", gl.sprite_shader, gl.sprite_size, gl.sprite_size, 0.f, pl2.x, pl2.y); // Adding Player 2 sprite

            sg_player2.add_animation(0, "mleft", { DPair("walk1_left", 100), DPair("walk2_left", 100) });
            sg_player2.add_animation(0, "mright", { DPair("walk1_right", 100), DPair("walk2_right", 100) });
            sg_player2.add_animation(0, "jleft", { DPair("jump_left", 700) });
            sg_player2.add_animation(0, "jright", { DPair("jump_right", 700) });
            sg_player2.add_animation(0, "stand_right", { DPair("stand_right", 1) });
            sg_player2.add_animation(0, "stand_left", { DPair("stand_left", 1) });
        #endif

        // Parsing server.cfg
        string servercfg = rm_main.getFileStr("server.cfg");
        string ip = servercfg.substr(0, servercfg.find_first_of(':'));
        unsigned short port = atoi(servercfg.substr(servercfg.find_first_of(':') + 1, servercfg.find_first_of(';')).c_str());
        
        NetHandler cli(ip, port, cl.server); // Creating client

        // Binding keys (some functions are still in older Key Handler)
        kh_main.bind(KEY_SPACE, [](){ if (!pl.jumping) { std::thread t(jump); t.detach(); } });
        kh_main.bind('W', [](){ if (collides_ceiling()) pl.spidering = true; }, [](){ pl.spidering = false; });
        kh_main.bind('0', [](){ cam.mag -= cam.mag_speed; });
        kh_main.bind('9', [](){ cam.mag += cam.mag_speed; });
        kh_main.bind(KEY_UP, [](){ cam.y += cam.speed; });
        kh_main.bind(KEY_DOWN, [](){ cam.y -= cam.speed; });
        kh_main.bind(KEY_RIGHT, [](){ cam.x += cam.speed; });
        kh_main.bind(KEY_LEFT, [](){ cam.x -= cam.speed; });
        kh_main.bind(KEY_F1, [](){ cam.x = pl.x - Size.x / 2; cam.y = pl.y - gl.sprite_size; });

        sg_player.set_timer();
        sg_player2.set_timer();

        #ifdef online
            thread t(netloop, &cli);
            t.detach();
        #endif

        while (!glfwWindowShouldClose(window)) { // Main game loop
            glClear(GL_COLOR_BUFFER_BIT);

            keyHandler(window); // Setting (old) key handler
            kh_main.use(); // Setting (new) key handler

            // Projection matrix variables
            float projMat_right  = Size.x * cam.mag + cam.x;
            float projMat_top    = Size.y * cam.mag + cam.y;
            float projMat_left   = - Size.x * (cam.mag - 1) + cam.x;
            float projMat_bottom = - Size.y * (cam.mag - 1) + cam.y;

            glm::mat4 projMat = glm::ortho(projMat_left, projMat_right, projMat_bottom, projMat_top, -100.f, 100.f); // Setting projection matrix

            // Using projection matrix
            defaultShaderProgram->setMat4("projMat", projMat);
            spriteShaderProgram->setMat4("projMat", projMat);

            defaultShaderProgram->use(); // Using default shader
            tl_main.bind_all(); // Binding all textures

            fall(); // Always falling down
            prevent_clipping(); // Prevent player from clipping through walls
            set_stand_anim(); 

            sg_player.rotate_all(180 - cam.rot); // Setting rotation (Player 1)
            sg_player.set_pos(pl.x, pl.y); // Setting position (Player 1)
            sg_player.set_animation(0, pl.current_anim); // Setting animation (Player 1)

            #ifdef online
                sg_player2.rotate_all(180 - cam.rot); // Setting rotation (Player 2)
                sg_player2.set_pos(pl2.x, pl2.y); // Setting position (Player 2)
                sg_player2.set_animation(0, pl2.current_anim); // Setting animation (Player 2)
            #endif

            // Rendering all sprites
            sg_sprites.render_all();
            sg_player.render_all();
            sg_player2.render_all();

            sg_player.update_all();
            sg_player2.update_all();            

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