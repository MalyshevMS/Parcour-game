#include <vector>
#include <memory>

#include "ResourceManager.hpp"
#include "../Renderer/Sprite.hpp"

class SprLoader {
private:
    ResourceManager* resMan;
    std::vector <std::shared_ptr<Renderer::Sprite>> sprites;
    std::vector <glm::vec2> default_positions;
public:
    SprLoader(ResourceManager* resMan) {
        this->resMan = resMan;
    };

    void add_sprite(const string& name, const string& tex, const string& shader, unsigned int width, unsigned int height, float rotation, unsigned int pos_x, unsigned int pos_y) {
        auto new_spr = this->resMan->loadSprite(name, tex, shader, width, height, rotation);
        new_spr->setPos(glm::vec2(pos_x, pos_y));

        sprites.push_back(new_spr);
        default_positions.push_back(glm::vec2(pos_x, pos_y));
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
        }
    };

    void rotate_all(float cam_rot) {
        for (int i = 0; i < this->sprites.size(); i++) {
            this->sprites[i]->setRotation(cam_rot);
        }
    };
};