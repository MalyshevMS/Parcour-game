#include <vector>
#include <memory>

#include "ResourceManager.hpp"
#include "../Renderer/Texture2D.hpp"

class TexLoader {
private:
    ResourceManager* resMan;
    std::vector <std::shared_ptr<Renderer::Texture2D>> textures;
public:
    TexLoader(ResourceManager* resMan) {
        this->resMan = resMan;
    };

    void add_texture(const string& name, const string& path) {
        this->textures.push_back(this->resMan->loadTexture(name, path));
    };

    void bind_all() {
        for (int i = 0; i < this->textures.size(); i++) {
            this->textures[i]->bind();
        }
    };
};