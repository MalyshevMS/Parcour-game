#include <vector>
#include <memory>

#include "ResourceManager.hpp"
#include "../Renderer/Sprite.hpp"

class SprLoader {
private:
    ResourceManager* resMan;
    std::vector <std::shared_ptr<Renderer::Sprite>> sprites;
public:
    SprLoader(ResourceManager* resMan) {
        this->resMan = resMan;
    };

    void add_sprite(const string& name, const string& tex, const string& shader, unsigned int width, unsigned int height, float rotation, unsigned int pos_x, unsigned int pos_y) {
        auto new_spr = this->resMan->loadSprite(name, tex, shader, width, height, rotation);
        new_spr->setPos(glm::vec2(pos_x, pos_y));

        sprites.push_back(new_spr);
    };

    void render_all() {
        for (int i = 0; i < this->sprites.size(); i++) {
            this->sprites[i]->render();
        }
    };
};