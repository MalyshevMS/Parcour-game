#pragma once
#include <string>
#include "Cursor.hpp"

#define l_left -1
#define l_right 1

struct Player {
    int x = 0; // Player X
    int y = 80; // Player Y
    int look = l_left; // Player looking direction
    std::string current_anim = ""; // Player current animation
    std::string name = ""; // Player nickname
    std::string msg = ""; // Player current message
    Cursor cur; // Player cursor position
    bool moving = false; // Player moving flag
    // bool moving_online = false; // Player moving flag (online)
    bool jumping = false; // Player jumping flag
    bool noclip = false; // Player no clipping flag
};