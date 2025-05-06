#pragma once

#include <string>

struct Client {
    int max_speed = 10; // Max player movement speed
    int bullet_speed = 5; // Bullet movement speed
    float bullet_dmg = 30.f; // Bullet damage
    int jump_speed = 5; // Player(s) jump speed
    int jump_height = 160; // Max player jump height
    float gravity = 9.80665f; // Gravity value
    bool server = false; // Is this a client or host
    bool paused = true; // Is game paused
    bool in_game = false; // Is in game or in menu
    bool placed_block = false; // Is block placed
    bool shooted = false; // Is the shot taken
    bool round_win = false; // Is the round won
    std::string level; // Path to level.json
    std::string ip; // Server IP
    unsigned short port; // Server port
    unsigned short delay; // delay between sending messages  
};