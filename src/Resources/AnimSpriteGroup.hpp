#pragma once

#include <vector>
#include <memory>
#include <map>
#include <thread>
#include <chrono>

#include "ResourceManager.hpp"
#include "../Renderer/Sprite.hpp"
#include "../Renderer/AnimatedSprite.hpp"

class AnimSprGroup {
private:
    ResourceManager* resMan;
    std::vector <std::shared_ptr <Renderer::AnimatedSprite>> sprites;
    std::chrono::_V2::system_clock::time_point last;
    std::chrono::_V2::system_clock::time_point current;
public:
    AnimSprGroup(ResourceManager* resMan = nullptr) {
        this->resMan = resMan;
    }

    void add_sprite(const std::string& tex, const std::string& subtex, const std::string& shader, const unsigned int& width, const unsigned int& height, const float& rotation, const int& pos_x, const int& pos_y) {
        auto new_spr = resMan->loadAnimSprite(tex, subtex, shader, width, height, rotation);
        new_spr->setPos(glm::vec2(pos_x, pos_y));

        sprites.push_back(new_spr);
    };

    void add_animation(const int& sprite_num, const std::string& name, const std::initializer_list <std::pair <std::string, uint64_t>>& lst) {
        sprites[sprite_num]->insert_state(std::move(name), std::move(lst));
    };

    void set_animation(const int& sprite_num, const std::string& name) {
        sprites[sprite_num]->set_state(std::move(name));
    };

    void render_all() {
        for (auto i : sprites) i->render();
    };

    void update_all() {
        current = std::chrono::high_resolution_clock::now();
        uint64_t duration = std::chrono::duration_cast<std::chrono::milliseconds>(current - last).count();
        last = current;

        for (auto i : sprites) {
            i->update(duration);
        }
    };

    void set_timer() {
        last = std::chrono::high_resolution_clock::now();
    };

    void set_pos(const int& x, const int& y) {
        for (auto i : sprites) {
            i->setPos(glm::vec2(x, y));
        }
    };

    void rotate_all(const float& rot) {
        for (auto i : sprites) {
            i->setRotation(rot);
        }
    };

    void delete_all() {
        sprites.clear();
    };
};