#pragma once
#include <string>

#define l_left -1
#define l_right 1

struct Player {
    int x = 0; // Player X
    int y = 80; // Player Y
    int look = l_left; // Player looking direction
    std::string current_anim = ""; // Player current animation
    std::string name = ""; // Player nickname
    bool moving = false; // Player moving flag
    bool jumping = false; // Player jumping flag
    bool noclip = false; // Player no clipping flag
};