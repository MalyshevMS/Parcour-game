#pragma once

#include <vector>
#include <memory>
#include <map>
#include <thread>

#include "ResourceManager.hpp"
#include "../Renderer/Sprite.hpp"
#include "../maths"

void sleep(unsigned int milliseconds) {
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

class SprGroup {
private:
    ResourceManager* resMan;
    std::vector <std::shared_ptr<Renderer::Sprite>> sprites;
    std::vector <glm::vec2> default_positions;
    std::vector <glm::vec2> current_positions;
    std::vector <float> rotations;
public:
    SprGroup(ResourceManager* resMan = nullptr) {
        this->resMan = resMan;
    };

    void add_sprite(std::string tex, std::string shader, unsigned int width, unsigned int height, float rotation, int pos_x, int pos_y) {
        auto new_spr = this->resMan->loadSprite(tex, shader, width, height, rotation);
        new_spr->setPos(glm::vec2(pos_x, pos_y));

        sprites.push_back(new_spr);
        default_positions.push_back(glm::vec2(pos_x, pos_y));
        current_positions.push_back(glm::vec2(pos_x, pos_y));
        rotations.push_back(rotation);
    };

    void render_all() {
        for (int i = 0; i < this->sprites.size(); i++) {
            this->sprites[i]->render();
        }
    };

    void move_all(int x, int y) {
        for (int i = 0; i < sprites.size(); i++) {
            sprites[i]->setPos(glm::vec2(current_positions[i].x + x, current_positions[i].y + y));
        }
    };

    void set_pos(int x, int y) {
        for (int i = 0; i < sprites.size(); i++) {
            sprites[i]->setPos(glm::vec2(x, y));
        }
    }

    // Please don't use this function!
    void follow_cam(int cam_x, int cam_y, int zer_x, int zer_y) {
        for (int i = 0; i < sprites.size(); i++) {
            glm::vec2 point(default_positions[i].x, default_positions[i].y);
            glm::vec2 new_(point.x - cam_x + zer_x, point.y - cam_y + zer_y);
            sprites[i]->setPos(new_);
        }
    };

    void rotate_all(float cam_rot) {
        for (int i = 0; i < this->sprites.size(); i++) {
            this->sprites[i]->setRotation(cam_rot);
            this->rotations[i] = cam_rot;
        }
    };

    void delete_all() {
        sprites.clear();
        default_positions.clear();
        current_positions.clear();
        rotations.clear();
    };

    std::vector <std::shared_ptr<Renderer::Sprite>> get_sprites() {
        return this->sprites;
    };

    std::vector <glm::vec2> get_default_pos() {
        return this->default_positions;
    };

    std::vector <glm::vec2> get_current_pos() {
        return current_positions;
    };

    std::vector <float> get_rotation() {
        return this->rotations;
    };
};