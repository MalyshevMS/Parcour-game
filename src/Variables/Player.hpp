#define l_left -1
#define l_right 1

struct Player {
    int x = 0; // Player X
    int y = 80; // Player Y
    int look = l_left; // Player looking direction
    bool moving = false; // Player moving flag
    bool jumping = false; // Player jumping flag
    bool spidering = false; // Player spider mode flag
    bool noclip = false; // Player no clipping flag
};