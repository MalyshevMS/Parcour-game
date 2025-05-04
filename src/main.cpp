// #define debug
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
#include "Resources/Parser.hpp"
#include "Other/KeyHandler.hpp"
#include "Other/Console.hpp"
#include "Online/NetHandler.hpp"

#include "Variables/OpenGL.hpp" // variables
#include "Variables/Camera.hpp"
#include "Variables/Client.hpp"
#include "Variables/Player.hpp"
#include "Variables/Cursor.hpp"

#include "keys"

#include <iostream> // other libs
#include <string>
#include <cmath>
#include <vector>
#include <thread>

using namespace std;
using DPair = pair<string, uint64_t>;

OpenGL gl;
Camera cam;
Client cl;
Player pl;
Player pl2;

ResourceManager rm_main; // Main Resource manager

TexLoader tl_main; // Main Texture loader

SprGroup sg_sprites; // Group for obstacles, walls, etc.
SprGroup sg_bullets; // Group for bullets
SprGroup sg_text; // Group for text rendering
SprGroup sg_pause; // Group for text while pauses
SprGroup sg_buttons; // Group for buttons
SprGroup sg_player; // Group for Player 1
SprGroup sg_player2; // Group for Player 2

Parser pars_main; // Main parser

KeyHandler kh_main; // Main Key Handler

Console cmd(&cl);

float __ticks;
float __ticks2;

/// @brief Function for resizing window
/// @param win GLFW window poiner
/// @param width new window width
/// @param height new window height
void sizeHandler(GLFWwindow* win, int width, int height) {
    gl.win_size.x = width;
    gl.win_size.y = height;
    glViewport(0, 0, gl.win_size.x, gl.win_size.y);
}

/// @brief Checks if player collides floor
/// @param epsilon how much units player can go through sprite in Y direction
/// @param epsilon2 how much units player can be far away from sprite in X direction
/// @return true, if player collides floor. Else false
bool collides_floor(int epsilon = 2, int epsilon2 = 0) {
    if (pl.y <= epsilon) return true;
    vector sprites_pos = sg_sprites.get_sprites();
    for (int i = 0; i < sprites_pos.size(); i++) {
        if (abs(pl.y - sprites_pos[i]->getPos().y) <= gl.sprite_size - epsilon && pl.y - sprites_pos[i]->getPos().y >= epsilon && abs(pl.x - sprites_pos[i]->getPos().x) < gl.sprite_size - epsilon2) return true;
    }
    return false;
}

/// @brief Checks if player collides ceiling
/// @param epsilon how much units player can go through sprite in Y direction
/// @param epsilon2 how much units player can be far away from sprite in X direction
/// @return true, if player collides ceiling. Else false
bool collides_ceiling(int epsilon = 2 , int epsilon2 = 0) {
    vector sprites_pos = sg_sprites.get_sprites();
    for (int i = 0; i < sprites_pos.size(); i++) {
        if ((pl.y + gl.sprite_size) - sprites_pos[i]->getPos().y >= -epsilon && (pl.y + gl.sprite_size) - sprites_pos[i]->getPos().y <= gl.sprite_size && abs(pl.x - sprites_pos[i]->getPos().x) < gl.sprite_size) return true;
    }
    return false;
}

/// @brief Checks if player collides left wall
/// @param epsilon how much units player can go through sprite in X direction
/// @param epsilon2 how much units player can be far away from sprite in Y direction
/// @return true, if player collides left wall. Else false
bool collides_left(int epsilon = 0, int epsilon2 = 2) {
    vector sprites_pos = sg_sprites.get_sprites();
    for (int i = 0; i < sprites_pos.size(); i++) {
        if (pl.x - (sprites_pos[i]->getPos().x + gl.sprite_size) <= -epsilon && pl.x - (sprites_pos[i]->getPos().x + gl.sprite_size) > -2 * gl.sprite_size && abs(pl.y - sprites_pos[i]->getPos().y) < gl.sprite_size - epsilon2) return true;
    }
    return false;
}

/// @brief Checks if player collides right wall
/// @param epsilon how much units player can go through sprite in X direction
/// @param epsilon2 how much units player can be far away from sprite in Y direction
/// @return true, if player collides right wall. Else false
bool collides_right(int epsilon = 0, int epsilon2 = 2) {
    vector sprites_pos = sg_sprites.get_sprites();
    for (int i = 0; i < sprites_pos.size(); i++) {
        if (abs((pl.x + gl.sprite_size) - sprites_pos[i]->getPos().x) <= -epsilon && abs(pl.y - sprites_pos[i]->getPos().y) < gl.sprite_size - epsilon2) return true;
    }
    return false;
}


/// @brief Prevents player from clipping through the wall
void prevent_clipping() {
    if (!pl.noclip) {
        if (collides_floor(1)) {
            pl.y++;
        }
    }
}


/// @brief Function for calculating falling speed based on time
/// @param ticks how much much ticks passed from the start of falling
/// @param tick_time how much game ticks will be proceed as 1 second
/// @return Falling velocity
auto g = [](float ticks, float tick_time = 50.f){ return (ticks / tick_time) * cl.gravity; };

/// @brief Function for player jumping
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

/// @brief Function for checking collisions beetween 2 sprite groups
/// @param sg1 First sprite group
/// @param sg2 Second sprite group
/// @return Is these groups colliding
bool sg_collision(SprGroup& sg1, SprGroup& sg2) {
    for (auto i : sg1.get_sprites()) {
        for (auto j : sg2.get_sprites()) {
            if (abs(i->getPos().x - j->getPos().x) <= gl.sprite_size && abs(i->getPos().y - j->getPos().y) <= gl.sprite_size) return true;
        }
    }
    return false;
}

/// @brief Function for player falling
void fall() {
    if (!collides_floor() && !collides_floor(0) && !pl.jumping && pl.y > 0) {
        if (g(__ticks) < cl.max_speed) pl.y -= g(__ticks);
        else pl.y -= cl.max_speed;
        __ticks++;
    } else {
        __ticks = 0;
    }
}

void shoot(Player& p) {
    if (!pl.bullet_cd) {
        float ang;
        if (p.look == l_left) {
            ang = 90.f;
        } else if (p.look == l_right) {
            ang = -90.f;
        }
        sg_bullets.add_sprite("Bullet", "default", gl.sprite_shader, gl.sprite_size, gl.sprite_size, ang, p.x, p.y);
        p.bullet_array.push_back(BPair(sg_bullets.get_sprites().size() - 1, p.look));
        p.bullet_cd = true;
        p.current_cd = p.global_cd;
        p.msg = "+shoot:1";
    }
}

void move_bullets(Player& p) {
    for (auto i : p.bullet_array) {
        int speed;
        if (i.second == l_left) speed = -cl.bullet_speed;
        else speed = cl.bullet_speed;
        sg_bullets.move(i.first, sg_bullets.get_sprites()[i.first]->getPos().x + speed, sg_bullets.get_sprites()[i.first]->getPos().y);
    }
}

void check_bullets() {
    if (cl.shooted) {
        shoot(pl2);
        cl.shooted = false;
    }
}

void detect_fail() {

}

/// @brief Displays pause text
void pause() {
    sg_pause.add_text("Font", "Game paused.", gl.sprite_shader, gl.font_width, gl.font_height, 0.f, gl.win_size.x / 2 + cam.x - 6 * gl.font_width, gl.win_size.y / 2 + cam.y);
}

/// @brief Deletes pause text
void unpause() {
    sg_pause.delete_all();
}

/// @brief Sets player animation to stand in the right direction
void set_stand_anim() {
    if (!pl.moving && !pl.jumping) {
        switch (pl.look) {
            case l_left: { pl.current_anim = "stand_left"; break; }
            case l_right: { pl.current_anim = "stand_right"; break; }
        }
    }
}

void look_update() {
    if (pl2.current_anim == "mleft" || pl2.current_anim == "jleft" || pl2.current_anim == "stand_left") {
        pl2.look = l_left;
    } else if (pl2.current_anim == "mright" || pl2.current_anim == "jright" || pl2.current_anim == "stand_right") {
        pl2.look = l_right;
    }
}

/// @brief Updates position message
/// @param delay a delay between updating position
/// @warning Use this function only in separate thread!
void pos_update(int delay = 10) {
    while (true) {
        if (cl.placed_block) continue;
        pl.msg = string("+move:") + to_string(pl.x) + "," + to_string(pl.y) + "," + pl.current_anim;
        sleep(delay);
    }
}

/// @brief Places block, using the position of player cursor
/// @param p reference to player
void place_block(Player& p) {
    glm::vec2 block;
    block.x = p.cur.x / gl.sprite_size * gl.sprite_size;
    block.y = p.cur.y / gl.sprite_size * gl.sprite_size;
    for (auto i : sg_sprites.get_sprites()) {
        if (i->getPos().x == block.x && i->getPos().y == block.y) return;
    }
    sg_sprites.add_sprite("Wall", "default", gl.sprite_shader, gl.sprite_size, gl.sprite_size, 0.f, block.x, block.y);
    p.msg = string("+block:") + to_string((int)block.x) + "," + to_string((int)block.y);
}

/// @brief checks placing block by other player
void check_block_placement() {
    if (cl.placed_block) {
        place_block(pl2);
        cl.placed_block = false;
    }
}

/// @brief Proceeds a Net realtion beetween 2 players
/// @warning Use this function only in separate thread!
/// @param cli pointer to the current Net Handler
void netloop(NetHandler* cli) {
    while (true) {
        cli->send_msg(pl.msg);
        auto msg = cli->recv_msg();
        
        cmd.action(msg, &pl2);
    }
}

/// @brief Proceeds a CMD interactions
void cmdloop() {
    string com;
    Player* current_player = &pl;
    while (true) {
        cout << gl.win_title + ": ";
        getline(cin, com);

        if (com == "exit" || com == "quit") exit(0);
        if (com == "chpl") {
            if (current_player == &pl) {
                current_player = &pl2;
                cmd.echo("Current player: PLAYER 2");
            } else {
                current_player = &pl;
                cmd.echo("Current player: PLAYER 1");
            }
        }

        endcmd:
        cmd.action(com, current_player);
        current_player->msg = com;
    }
}

/// @brief Proceeds once key pressings (if key was hold down for a long it's still will be recognize as once pressing)
void onceKeyHandler(GLFWwindow* win, int key, int scancode, int action, int mode) {
    if (key == KEY_LEFT_ALT && action == GLFW_PRESS) {
        cam.locked = !cam.locked;
    }

    if (key == KEY_B && action == GLFW_PRESS) {
        pl.noclip = !pl.noclip;
    }

    if ((key == KEY_ESCAPE || key == KEY_PAUSE) && action == GLFW_PRESS) {
        cl.paused = !cl.paused;
        if (cl.paused) pause();
    } else if ((key == KEY_ESCAPE || key == KEY_PAUSE) && action == GLFW_RELEASE && !cl.paused) unpause();

    if (key == KEY_ENTER && action == GLFW_PRESS) {
        cout << pl2.look << endl;
    }
}

/// @brief Function for proceeding keys pressing. P.S. This function supports long key pressing
/// @param win GLFW window pointer
void keyHandler(GLFWwindow* win) {
    if (cl.in_game) {
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

        if (glfwGetMouseButton(win, MOUSE_RIGHT)) {
            place_block(pl);
        }

        if (glfwGetMouseButton(win, MOUSE_LEFT)) {
            shoot(pl);
        }
    } else {
        if (glfwGetMouseButton(win, MOUSE_LEFT)) {
            if (sg_buttons.hovered(0, pl.cur)) {
                glfwSetWindowShouldClose(win, GL_TRUE);
            }

            if (sg_buttons.hovered(1, pl.cur)) {
                cout << "\nSettings:\nName: " << pl.name << endl;
                cout << "Renderer: " << glGetString(GL_RENDERER) << endl;
                cout << "OpenGL version: " << glGetString(GL_VERSION) << endl;
                cout << "Online: \n\tServer: " << cl.server << "\n\tIP: " << cl.ip << "\n\tPort: " << cl.port << endl << "Parcour-game:";
                sg_buttons.hide(1);
                sleep(100);
                sg_buttons.show(1);
            }

            if (sg_buttons.hovered(2, pl.cur)) {
                cl.paused = false;
                unpause();
                for (int i = 0; i < sg_buttons.get_sprites().size(); i++) sg_buttons.hide(i);
            }
        }
    }
}

void server_conf(string path) {
    auto file = pars_main.get_json(std::move(path));
    cl.ip = file["ip"];
    cl.port = file["port"];
    string mode = file["mode"];
    cl.server = mode == "host" ? true : false;
    pl.name = file["name"];
}

int main(int argc, char const *argv[]) {
    size_t found = string(argv[0]).find_last_of("/\\");
    string exe_path = string(argv[0]).substr(0, found + 1);

    std::fstream f;
    f.open(exe_path + "config.json");
    nlohmann::json file;
    f >> file;

    // Engine vars
    auto _gl = file["OpenGL"];
    gl.sprite_shader = _gl["sprite.shader"];
    gl.sprite_shader_vertex = _gl["sprite.shader.vertex"];
    gl.sprite_shader_fragment = _gl["sprite.shader.fragment"];
    gl.sprite_size = 80;
    gl.font_width = _gl["font.width"];
    gl.font_height = _gl["font.height"];
    gl.win_size.x = _gl["window.width"];
    gl.win_size.y = _gl["window.height"];
    gl.win_title = _gl["window.title"];
    gl.fullscreen = _gl["fullscreen"];

    auto _cam = file["Camera"];
    cam.x = pl.x - gl.win_size.x / 2;
    cam.y = pl.y - gl.sprite_size;
    cam.rot = 180.f;
    cam.mag = 1.f;
    cam.speed = _cam["speed"];
    cam.mag_speed = _cam["mag.speed"];
    cam.locked = _cam["locked"];

    auto _cl = file["Client"];
    cl.max_speed = _cl["max_speed"];
    cl.jump_speed = _cl["jump.speed"];
    cl.jump_height = _cl["jump.height"];
    cl.gravity = _cl["gravity"];
    cl.delay = _cl["delay"];

    auto _pl = file["Player"];
    pl.x = _pl["spawn.x"];
    pl.y = _pl["spawn.y"];
    pl2.x = 0;
    pl2.y = 0;
    pl.look = l_left;
    pl.current_anim = "stand_right";
    pl.noclip = _pl["noclip"];

    string com;

    #ifdef debug // Debugger console
        do {
            cout << "Parcour-game: ";
            getline(cin, com);

            if (com == "$exit") return 0;

            cmd.action(com, &pl);
        } while (com != "$play");
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

    GLFWwindow* window;
    if (gl.fullscreen) { // Creating window (fullscreen or windowed)
        window = glfwCreateWindow(gl.win_size.x, gl.win_size.y, gl.win_title.c_str(), glfwGetPrimaryMonitor(), nullptr);
    } else {
        window = glfwCreateWindow(gl.win_size.x, gl.win_size.y, gl.win_title.c_str(), nullptr, nullptr);
    }

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
        sg_bullets = SprGroup(&rm_main);
        sg_player = SprGroup(&rm_main);
        sg_player2 = SprGroup(&rm_main);
        sg_text = SprGroup(&rm_main);
        sg_pause = SprGroup(&rm_main);
        sg_buttons = SprGroup(&rm_main);
        pars_main = Parser(&rm_main, &tl_main, &sg_sprites, &gl);
        kh_main = KeyHandler(window);

        // Creating and checking for sprite shader
        auto spriteShaderProgram = rm_main.loadShaders(gl.sprite_shader, gl.sprite_shader_vertex, gl.sprite_shader_fragment);
        if (!spriteShaderProgram) {
            cerr << "Can't create SpriteShader" << endl;
            return -1;
        }

        // Using sprite shader
        spriteShaderProgram->use();
        spriteShaderProgram->setInt("tex", 0);

        pars_main.parse_lvl("res/lvl/level.json", &gl.sprite_size); // Parsing level
        pars_main.parse_player("player.json", &sg_player, glm::vec2(pl.x, pl.y));

        pl.current_anim = "stand_left";
        pl2.current_anim = "stand_left";

        #ifdef online
            pars_main.parse_player("player.json", &sg_player2, glm::vec2(pl2.x, pl2.y));
        #endif

        vector <string> chars;
        for (int i = 32; i < 96; i++) {
            string str;
            str.push_back((char)i);
            chars.push_back(str);
        }
        tl_main.add_textures_from_atlas("Font", "res/textures/font.png", chars, glm::vec2(64, 64));
        tl_main.add_texture("Filler", "res/textures/filler.png");

        server_conf("server.json"); // Parsing server.json

        #ifdef online
            NetHandler cli(cl.ip, cl.port, cl.server); // Creating client
            cli.send_msg(pl.name); // Recieving nickname
            pl2.name = cli.recv_msg();
            sg_player2.add_sprite("Filler", "default", gl.sprite_shader, gl.font_width * pl2.name.size(), gl.font_height, 0.f, pl2.x - gl.font_width * (pl2.name.size() / 2) + gl.sprite_size / 2 - 5, pl2.y + gl.sprite_size); // Creating nickname above Player 2
            sg_player2.add_text("Font", pl2.name, gl.sprite_shader, gl.font_width, gl.font_height, 0.f, pl2.x - gl.font_width * (pl2.name.size() / 2) + gl.sprite_size / 2, pl2.y + gl.sprite_size);
        #endif

        // Binding keys (some functions are still in older Key Handler)
        kh_main.bind(KEY_SPACE, [](){ if (!pl.jumping) { std::thread t(jump); t.detach(); } });
        kh_main.bind('=', [](){ cam.mag -= cam.mag_speed; });
        kh_main.bind('-', [](){ cam.mag += cam.mag_speed; });
        kh_main.bind(KEY_UP, [](){ cam.y += cam.speed; });
        kh_main.bind(KEY_DOWN, [](){ cam.y -= cam.speed; });
        kh_main.bind(KEY_RIGHT, [](){ cam.x += cam.speed; });
        kh_main.bind(KEY_LEFT, [](){ cam.x -= cam.speed; });
        kh_main.bind(KEY_F1, [](){ cam.x = pl.x - gl.win_size.x / 2; cam.y = pl.y - gl.sprite_size; });

        sg_player.set_timer();
        sg_player2.set_timer();

        // Threads
        #ifdef online
            thread thr_net(&netloop, &cli);
            thr_net.detach();

            thread thr_pos(&pos_update, cl.delay);
            thr_pos.detach();
        #endif

        thread thr_cmd(&cmdloop);
        thr_cmd.detach();

        // Adding pause menu buttons
        sg_player.add_sprite("Filler", "default", gl.sprite_shader, gl.font_width * pl.name.size(), gl.font_height, 0.f, pl.x - gl.font_width * (pl.name.size() / 2) + gl.sprite_size / 2 - 5, pl.y + gl.sprite_size);
        sg_player.add_text("Font", pl.name, gl.sprite_shader, gl.font_width, gl.font_height, 0.f, pl.x - gl.font_width * (pl.name.size() / 2) + gl.sprite_size / 2, pl.y + gl.sprite_size);

        sg_buttons.add_sprite("Filler", "default", gl.sprite_shader, gl.sprite_size * 3, gl.sprite_size, 0.f, cam.x + 10, cam.y + 160);
        sg_buttons.add_sprite("Filler", "default", gl.sprite_shader, gl.sprite_size * 3, gl.sprite_size, 0.f, cam.x + 10, cam.y + 250);
        sg_buttons.add_sprite("Filler", "default", gl.sprite_shader, gl.sprite_size * 3, gl.sprite_size, 0.f, cam.x + 10, cam.y + 340);

        sg_buttons.add_text("Font", "quit", gl.sprite_shader, gl.font_width, gl.font_height, 0.f, cam.x + 10 + 50, cam.y + 160 + 20);
        sg_buttons.add_text("Font", "settings", gl.sprite_shader, gl.font_width, gl.font_height, 0.f, cam.x + 10 + 20, cam.y + 250 + 20);
        sg_buttons.add_text("Font", "play", gl.sprite_shader, gl.font_width, gl.font_height, 0.f, cam.x + 10 + 50, cam.y + 340 + 20);

        while (!glfwWindowShouldClose(window)) { // Main game loop
            glClear(GL_COLOR_BUFFER_BIT);

            keyHandler(window); // Setting (old) key handler
            kh_main.use(cl); // Setting (new) key handler

            // Projection matrix variables
            float projMat_right  = gl.win_size.x * cam.mag + cam.x;
            float projMat_top    = gl.win_size.y * cam.mag + cam.y;
            float projMat_left   = - gl.win_size.x * (cam.mag - 1) + cam.x;
            float projMat_bottom = - gl.win_size.y * (cam.mag - 1) + cam.y;

            glm::mat4 projMat = glm::ortho(projMat_left, projMat_right, projMat_bottom, projMat_top, -100.f, 100.f); // Setting projection matrix

            // Using projection matrix
            spriteShaderProgram->setMat4("projMat", projMat);

            tl_main.bind_all(); // Binding all textures

            cl.in_game = !cl.paused;
            
            if (cl.in_game) {
                #ifndef online
                    fall(); // Always falling down
                    prevent_clipping(); // Prevent player from clipping through walls
                    set_stand_anim();
                #endif
                    
                    for (int i = 0; i < sg_buttons.get_sprites().size(); i++) {
                        sg_buttons.hide(i);
                    }
                } else {
                    for (int i = 0; i < sg_buttons.get_sprites().size(); i++) {
                        sg_buttons.show(i);
                    }
                }
                
            #ifdef online
                fall(); // Always falling down
                prevent_clipping(); // Prevent player from clipping through walls
                set_stand_anim();
            #endif
            
            check_block_placement();
            check_bullets();
            look_update();
            move_bullets(pl);
            move_bullets(pl2);

            sg_player.rotate_all(180 - cam.rot); // Setting rotation (Player 1)
            sg_player.set_pos(pl.x, pl.y); // Setting position (Player 1)
            sg_player.set_animation(0, pl.current_anim); // Setting animation (Player 1)

            #ifdef online
                sg_player2.rotate_all(180 - cam.rot); // Setting rotation (Player 2)
                sg_player2.set_pos(pl2.x, pl2.y); // Setting position (Player 2)
                sg_player2.set_animation(0, pl2.current_anim); // Setting animation (Player 2)
            #endif

            sg_player.update_all();
            sg_player2.update_all();

            pl.update();
            pl2.update();

            // Rendering all sprites
            sg_sprites.render_all();
            sg_bullets.render_all();
            sg_player.render_all();
            sg_player2.render_all();
            sg_text.render_all();
            sg_pause.render_all();
            sg_buttons.render_all();

            sg_buttons.set_pos(cam.x + 10, cam.y + 160);
            
            double cx, cy;
            glfwGetCursorPos(window, &cx, &cy);
            pl.cur.x = cx + cam.x;
            pl.cur.y = gl.win_size.y - cy + cam.y;

            sleep(1); // 1ms delay
            glfwSwapBuffers(window); // Swapping front and back buffers
            glfwPollEvents(); // Polling events
        }
        
        // Deleting all sprites from all groups
        sg_sprites.delete_all();
        sg_bullets.delete_all();
        sg_player.delete_all();
        sg_player2.delete_all();
        sg_text.delete_all();
        sg_pause.delete_all();
        sg_buttons.delete_all();
    }


    glfwTerminate(); 
    return 0; // Exiting programm
}