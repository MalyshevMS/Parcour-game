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

using namespace std;

glm::ivec2 Size(1280, 720);

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

int cam_x = Size.x / 2; // Camera X pos
int cam_y = Size.y / 2; // Camera Y pos
int cam_zero_x = Size.x / 2; // Camera Zero X pos
int cam_zero_y = Size.y / 2; // Camera Zero Y pos
float cam_rot = 180.f; // Camera rotation
float cam_mag = 1.f;
float cam_offset_x = 0;
float cam_offset_y = 0;
float cam_speed = 10.f;
float cam_mag_speed = 0.01f;

int sv_max_speed = 10; // Max player movement speed
int sv_jump_speed = 5;
int sv_gravity = 800;
bool sv_on_floor = true;
bool sv_is_jumping = false;

int pl_offset_x = 40;
int pl_offset_y = 280;
int pl_x = cam_x - pl_offset_x;
int pl_y = cam_y - pl_offset_y;

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

int collision_y() {
    if (pl_x >= -gl_sprite_size && pl_x <= 1280) return gl_sprite_size;
    else return 0;
}

int collides_x() {
    vector sprites_pos = sg_sprites.get_current_pos();
    for (int i = 0; i < sprites_pos.size(); i++) {
        if (abs(pl_x - (sprites_pos[i].x + gl_sprite_size)) <= 0.1f && abs(pl_y - sprites_pos[i].y) < gl_sprite_size) return -1;
        if (abs((pl_x + gl_sprite_size) - sprites_pos[i].x) <= 0.1f && abs(pl_y - sprites_pos[i].y) < gl_sprite_size) return 1;
    }
    return 0;
}

void Sleep(unsigned int milliseconds) {
    this_thread::sleep_for(chrono::milliseconds(milliseconds));
}

void jump() {
    if (sv_on_floor) {
        int height = pl_y + gl_sprite_size;
        sv_is_jumping = true;
        sv_on_floor = false;
        while (pl_y < height) {
            pl_y += sv_jump_speed;
            Sleep(1);
        }

        while (pl_y > collision_y()) {
            pl_y -= sv_jump_speed;
            Sleep(1);
        }
        sv_on_floor = true;
        sv_is_jumping = false;
    }
}

void keyHandler(GLFWwindow* win) {
    if (glfwGetKey(win, KEY_A) == GLFW_PRESS && collides_x() != -1) {
        pl_x -= sv_max_speed;
    }

    if (glfwGetKey(win, KEY_D) == GLFW_PRESS && collides_x() != 1) {
        pl_x += sv_max_speed;
    }

    if (glfwGetKey(win, KEY_LEFT_CONTROL) == GLFW_PRESS) {
        sv_max_speed = 20;
    } else if (glfwGetKey(win, KEY_LEFT_CONTROL) == GLFW_RELEASE) sv_max_speed = 10;

    if (glfwGetKey(win, KEY_SPACE) == GLFW_PRESS) {
        std::thread t(jump);
        t.detach();
    }

    if (glfwGetKey(win, KEY_W) == GLFW_PRESS) {
        pl_y = 160;
    }

    if (glfwGetKey(win, KEY_EQUAL) == GLFW_PRESS && cam_mag - cam_mag_speed >= 1.f) {
        cam_mag -= cam_mag_speed;
    }

    if (glfwGetKey(win, KEY_MINUS) == GLFW_PRESS) {
        cam_mag += cam_mag_speed;
    }

    if (glfwGetKey(win, KEY_UP) == GLFW_PRESS) {
        cam_offset_y += cam_speed;
    }

    if (glfwGetKey(win, KEY_DOWN) == GLFW_PRESS) {
        cam_offset_y -= cam_speed;
    }

    if (glfwGetKey(win, KEY_RIGHT) == GLFW_PRESS) {
        cam_offset_x += cam_speed;
    }

    if (glfwGetKey(win, KEY_LEFT) == GLFW_PRESS) {
        cam_offset_x -= cam_speed;
    }

    if (glfwGetKey(win, KEY_F1) == GLFW_PRESS) {
        cam_offset_x = 0;
        cam_offset_y = 0;
    }
}

void fall() {
    if (pl_y > collision_y() && !sv_is_jumping) {
        pl_y -= sv_jump_speed;
    }
}

int main(int argc, char const *argv[]) {
    string comand_line;

    #ifdef debug
        do {
            cout << "Minimal2D > ";
            cin >> comand_line;

            if (comand_line == "exit") { exit(0); }
            else if (comand_line == "get") {
                string variable;
                cin >> variable;

                if (variable == "gl_texture_mode") {
                    if (gl_texture_mode == GL_LINEAR) cout << "gl_texture_mode: " << '"' << "GL_LINEAR" << '"' << endl;
                    else if (gl_texture_mode == GL_NEAREST) cout << "gl_texture_mode: " << '"' << "GL_NEAREST" << '"' << endl;
                } else if (variable == "gl_default_shader") {
                    cout << "gl_default_shader: " << '"' << gl_default_shader << '"' << endl;
                }
            } else if (comand_line == "set") {
                string new_value, variable;
                cin >> new_value;
                cin >> variable;

                if (variable == "gl_texture_mode") {                
                    if (new_value == "GL_LINEAR") gl_texture_mode = GL_LINEAR;
                    else if (new_value == "GL_NEAREST") gl_texture_mode = GL_NEAREST;
                } else if (variable == "gl_default_shader") {
                    gl_default_shader = new_value;
                } else if (variable == "gl_sprite_shader") {
                    gl_sprite_shader = new_value;
                }
            } else if (comand_line == "unittest") {
                string testfunc;
                cin >> testfunc;
                if (testfunc == "collides_x") {
                    int par_pl_x, par_spr_x, par_spr_size;
                    int buff_pl_x = pl_x;
                    cin >> par_pl_x >> par_spr_x >> par_spr_size;
                    pl_x = par_pl_x;
                    cout << collides_x() << endl;

                    rm_main = ResourceManager(argv[0]);
                    tl_textures = TexLoader(&rm_main);
                    sg_sprites = SprGroup(&rm_main);

                    rm_main.loadShaders("", "", "");
                    tl_textures.add_texture("", "");
                    sg_sprites.add_sprite("unittest", "", "", par_spr_size, par_spr_size, 0.f, par_spr_x, 0);
                }
            }
        } while (comand_line != "init");
    #endif

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

    glfwMakeContextCurrent(window);

    if (!gladLoadGL()) {
        cerr << "Can't load GLAD!" << endl;
    }

    cout << "Renderer: " << glGetString(GL_RENDERER) << endl;
    cout << "OpenGL version: " << glGetString(GL_VERSION) << endl;

    glClearColor(0, 1, 0, 1);

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

        for (int i = 0; i < Size.x; i += gl_sprite_size) {
            sg_sprites.add_sprite("Sprite_Wall_Bottom_" + std::to_string(i/gl_sprite_size), "Wall", gl_sprite_shader, gl_sprite_size, gl_sprite_size, 0.f, i, 0);
        }

        /*sg_sprites.add_sprite("Sprite_Wall_Obstacle_0", "Wall", gl_sprite_shader, gl_sprite_size * 2, gl_sprite_size, 0.f, gl_sprite_size * 2, gl_sprite_size);
        sg_sprites.add_sprite("Sprite_Wall_Obstacle_1", "Wall", gl_sprite_shader, gl_sprite_size * 2, gl_sprite_size, 0.f, gl_sprite_size * 3, gl_sprite_size * 2);
        sg_sprites.add_sprite("Sprite_Wall_Obstacle_2", "Wall", gl_sprite_shader, gl_sprite_size * 2, gl_sprite_size, 0.f, gl_sprite_size * 4, gl_sprite_size * 3);*/

        while (!glfwWindowShouldClose(window)) {
            glClear(GL_COLOR_BUFFER_BIT);

            keyHandler(window);

            cam_x = pl_x + pl_offset_x;
            cam_y = pl_y + pl_offset_y;

            float projMat_right  = Size.x * cam_mag + cam_offset_x;
            float projMat_top    = Size.y * cam_mag + cam_offset_y;
            float projMat_left   = - Size.x * (cam_mag - 1) + cam_offset_x;
            float projMat_bottom = - Size.y * (cam_mag - 1) + cam_offset_y;

            glm::mat4 projMat = glm::ortho(projMat_left, projMat_right, projMat_bottom, projMat_top, -100.f, 100.f);

            defaultShaderProgram->setMat4("projMat", projMat);
            spriteShaderProgram->setMat4("projMat", projMat);


            defaultShaderProgram->use();
            tl_textures.bind_all();

            fall();

            sg_sprites.follow_cam(cam_x, cam_y, cam_zero_x, cam_zero_y);
            sg_mobs.follow_cam(cam_x, cam_y, cam_zero_x, cam_zero_y);

            sg_player.rotate_all(180 - cam_rot);

            sg_sprites.render_all();
            sg_player.render_all();
            sg_mobs.render_all();

            Sleep(1);
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