struct Camera {
    float x = 0; // Camera X pos
    float y = 0; // Camera Y pos
    float rot = 180.f; // Camera rotation
    float mag = 1.f; // Current camera magnifying
    float speed = 10.f; // Camera movement speed
    float mag_speed = 0.01f; // Camera magnifying speed
    bool locked = true; // Camera lock (to player) flag
};