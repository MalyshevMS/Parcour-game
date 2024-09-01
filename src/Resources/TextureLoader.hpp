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

    void add_texture(const std::string& name, const std::string& path, GLenum mode = GL_NEAREST) {
        if (resMan->getTexture(name, true) == nullptr) {
            textures.push_back(resMan->loadTexture(name, path, mode));
        } else {
            std::cerr << "This texture already exists!" << std::endl;
        }
    };

    void add_textures_from_atlas(const std::string& name, const std::string& path, const std::initializer_list<std::string>& subtex_names, const glm::vec2& tex_size) {
        auto atlas = resMan->loadTexAtlas(name, path, subtex_names, tex_size.x, tex_size.y);
        textures.push_back(atlas);
    };

    void bind_all() {
        for (int i = 0; i < textures.size(); i++) {
            textures[i]->bind();
        }
    };
};