#pragma once

#include <vector>
#include <string>
#include <memory>
#include <map>
#include <thread>

#include "ResourceManager.hpp"
#include "../Renderer/Sprite.hpp"

void sleep(unsigned int milliseconds) {
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

class SprGroup {
protected:
    ResourceManager* resMan;
    std::vector <std::shared_ptr<Renderer::Sprite>> sprites;
    std::vector <glm::vec2> default_positions;
    std::vector <glm::vec2> current_positions;
    std::vector <float> rotations;
    glm::vec2 origin = glm::vec2(0, 0);
public:
    SprGroup(ResourceManager* resMan = nullptr) {
        this->resMan = resMan;
    };

    void add_sprite(const std::string& tex, const std::string& subtex, const std::string& shader, const unsigned int& width, const unsigned int& height, const float& rotation, const int& pos_x, const int& pos_y) {
        auto new_spr = this->resMan->loadSprite(tex, subtex, shader, width, height, rotation);
        new_spr->setPos(glm::vec2(pos_x, pos_y));

        if (sprites.size() == 0) origin = glm::vec2(pos_x, pos_y);

        sprites.push_back(new_spr);
        default_positions.push_back(glm::vec2(pos_x, pos_y));
        current_positions.push_back(glm::vec2(pos_x, pos_y));
        rotations.push_back(rotation);
    };

    void add_text(const std::string& atlas, const std::string& text, const std::string& shader, const int& width, const int& height, const float& rotation, const int& pos_x, const int& pos_y) {
        for (int i = 0; i < text.size(); i++) {
            std::string str;
            str.push_back((char)std::toupper(text[i]));

            this->add_sprite(atlas, str, shader, width, height, rotation, pos_x + (i * width), pos_y);
        }
    };

    void render_all() {
        for (int i = 0; i < this->sprites.size(); i++) {
            this->sprites[i]->render();
        }
    };

    void move_all(int x, int y) {
        for (int i = 0; i < sprites.size(); i++) {
            sprites[i]->setPos(glm::vec2(current_positions[i].x + x, current_positions[i].y + y));
            current_positions[i].x += x;
            current_positions[i].y += y;
            if (i == 0) origin = current_positions[i];
        }
    };

    void set_pos(int x, int y) {
        this->move_all(x - origin.x, y - origin.y);
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

    void delete_sprite(int spr_num) {
        sprites.erase(sprites.begin() + spr_num);
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