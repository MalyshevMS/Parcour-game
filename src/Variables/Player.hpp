#pragma once
#include <string>
#include <vector>
#include "Cursor.hpp"

#define l_left -1
#define l_right 1

using BPair = std::pair <int, int>;

struct Player {
    int x = 0; // Player X
    int y = 80; // Player Y
    int look = l_left; // Player looking direction
    int current_cd = 0; // Player current bullet cooldown
    int global_cd = 100; // Player bullet cooldown
    int score = 0;
    std::string current_anim = ""; // Player current animation
    std::string name = ""; // Player nickname
    std::string msg = ""; // Player current message
    std::vector <BPair> bullet_array;
    Cursor cur = Cursor(); // Player cursor position
    bool moving = false; // Player moving flag
    bool jumping = false; // Player jumping flag
    bool noclip = false; // Player no clipping flag
    bool bullet_cd = false; // Player bullet cooldown flag
    float hp = 100.f; // Player HP
    float max_hp = 100.f; // Player maximum HP

    void update() {
        if (bullet_cd) current_cd--;
        if (current_cd == 0) {
            bullet_cd = false;
        }
    };
};