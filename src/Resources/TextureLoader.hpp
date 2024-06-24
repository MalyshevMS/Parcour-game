#pragma once

#include <vector>
#include <memory>

#include "ResourceManager.hpp"
#include "../Renderer/Texture2D.hpp"

class TexLoader {
private:
    ResourceManager* resMan;
    std::vector <std::shared_ptr<Renderer::Texture2D>> textures;
public:
    TexLoader(ResourceManager* resMan = nullptr) {
        this->resMan = resMan;
    };

    void add_texture(const string& name, const string& path, GLenum mode = GL_NEAREST) {
        if (this->resMan->getTexture(name, true) == nullptr) {
            this->textures.push_back(this->resMan->loadTexture(name, path, mode));
        } else {
            std::cerr << "This texture already exists!" << std::endl;
        }
    };

    void bind_all() {
        for (int i = 0; i < this->textures.size(); i++) {
            this->textures[i]->bind();
        }
    };
};