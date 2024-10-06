#pragma once

#include <vector>
#include <string>
#include <memory>
#include <map>
#include <thread>

#include "ResourceManager.hpp"
#include "../Renderer/Sprite.hpp"
#include "../Variables/Cursor.hpp"

void sleep(unsigned int milliseconds) {
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

class SprGroup {
protected:
    ResourceManager* resMan;
    std::vector <std::shared_ptr<Renderer::AnimatedSprite>> sprites;
    glm::vec2 origin = glm::vec2(0, 0);
    std::chrono::_V2::system_clock::time_point last;
    std::chrono::_V2::system_clock::time_point current;
public:
    SprGroup(ResourceManager* resMan = nullptr) {
        this->resMan = resMan;
    };

    void add_sprite(const std::string& tex, const std::string& subtex, const std::string& shader, const unsigned int& width, const unsigned int& height, const float& rotation, const int& pos_x, const int& pos_y) {
        auto new_spr = this->resMan->loadAnimSprite(tex, subtex, shader, width, height, rotation);
        new_spr->setPos(glm::vec2(pos_x, pos_y));

        if (sprites.size() == 0) origin = glm::vec2(pos_x, pos_y);

        sprites.push_back(new_spr);
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

    void add_animation(const int& sprite_num, const std::string& name, const std::initializer_list <std::pair <std::string, uint64_t>>& lst) {
        sprites[sprite_num]->insert_state(std::move(name), std::move(lst));
    };

    void set_animation(const int& sprite_num, const std::string& name) {
        sprites[sprite_num]->set_state(std::move(name));
    };

    void update_all() {
        current = std::chrono::high_resolution_clock::now();
        uint64_t duration = std::chrono::duration_cast<std::chrono::milliseconds>(current - last).count();
        last = current;

        for (auto i : sprites) {
            i->update(duration);
        }
    };

    void hide(int spr_num) {
        sprites[spr_num]->_render = false;
    };

    void show(int spr_num) {
        sprites[spr_num]->_render = true;
    };

    void set_timer() {
        last = std::chrono::high_resolution_clock::now();
    };

    bool hovered(int spr_num, Cursor& c) {
        if (!sprites[spr_num]->_render) return false;
        if (c.x >= sprites[spr_num]->getPos().x && c.x <= sprites[spr_num]->getPos().x + sprites[spr_num]->getSize().x && \
            c.y >= sprites[spr_num]->getPos().y && c.y <= sprites[spr_num]->getPos().y + sprites[spr_num]->getSize().y) {
                return true;
        } else return false;
    };

    void move_all(int x, int y) {
        for (int i = 0; i < sprites.size(); i++) {
            sprites[i]->setPos(glm::vec2(sprites[i]->getPos().x + x, sprites[i]->getPos().y + y));
            if (i == 0) origin = sprites[i]->getPos();
        }
    };

    void move(int spr_num, int x, int y) {
        sprites[spr_num]->setPos(glm::vec2(x, y));
    };

    void rotate(int spr_num, float rot) {
        sprites[spr_num]->setRotation(rot);
    };

    void set_pos(int x, int y) {
        this->move_all(x - origin.x, y - origin.y);
    };

    void rotate_all(float cam_rot) {
        for (int i = 0; i < this->sprites.size(); i++) {
            this->sprites[i]->setRotation(cam_rot);
        }
    };

    void delete_all() {
        sprites.clear();
    };

    void delete_sprite(int spr_num) {
        sprites.erase(sprites.begin() + spr_num);
    };

    std::vector <std::shared_ptr<Renderer::AnimatedSprite>> get_sprites() {
        return this->sprites;
    };
};