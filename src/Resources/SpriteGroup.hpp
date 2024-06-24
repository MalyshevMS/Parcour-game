#pragma once

#include <vector>
#include <memory>

#include "ResourceManager.hpp"
#include "../Renderer/Sprite.hpp"

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

    void add_sprite(const string& name, const string& tex, const string& shader, unsigned int width, unsigned int height, float rotation, int pos_x, int pos_y) {
        auto new_spr = this->resMan->loadSprite(name, tex, shader, width, height, rotation);
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

    void move_all(int cam_x, int cam_y, int zer_x, int zer_y) {
        for (int i = 0; i < this->sprites.size(); i++) {
            glm::vec2 point = this->default_positions[i];
            glm::vec2 new_ = glm::vec2(point.x - cam_x + zer_x, point.y - cam_y + zer_y);
            this->sprites[i]->setPos(new_);
            this->current_positions[i] = new_;
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
        sizes.clear();
        points.clear();
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