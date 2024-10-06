#pragma once

#include <string>

struct Client {
    int max_speed = 10; // Max player movement speed
    int jump_speed = 5; // Player(s) jump speed
    int jump_height = 160; // Max player jump height
    float gravity = 9.80665f; // Gravity value
    bool server = false; // Is this a client or host
    bool paused = false; // Is game paused
    bool in_game = false; // Is in game or in menu
    std::string ip; // Server IP
    unsigned short port; // Server port
};