#pragma once

#include <string>
#include <memory>
#include <map>
#include <sstream>
#include <fstream>
#include <iostream>

#include "../Renderer/ShaderProgram.hpp"
#include "../Renderer/Texture2D.hpp"
#include "../Renderer/Sprite.hpp"
#include "../Renderer/AnimatedSprite.hpp"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#include "stb_image.hpp"
#define nl std::endl

class ResourceManager {
private:
    typedef std::map <const std::string, std::shared_ptr <Renderer::ShaderProgram>> ShaderProgramsMap;
    typedef std::map <const std::string, std::shared_ptr<Renderer::Texture2D>> TexturesMap;

    ShaderProgramsMap _shaderPrograms;
    TexturesMap _textures;

    std::string _path;

public:
    ResourceManager(const std::string& exePath = "") {
        size_t found = exePath.find_last_of("/\\");
        this->_path = exePath.substr(0, found + 1);
    };

    ~ResourceManager() = default;
    
    std::string getFileStr(const std::string& path) const {
        std::ifstream f;
        f.open(_path + path, std::ios::in | std::ios::binary);
        if (!f.is_open()) {
            std::cerr << "Failed to open file " << _path + path << nl;
            return std::string();
        }

        std::stringstream buffer;
        buffer << f.rdbuf();

        return buffer.str();
    };

    std::string getExePath() {
        return _path;
    };

    std::shared_ptr <Renderer::ShaderProgram> loadShaders(const std::string& shaderName, const std::string& vertexPath, const std::string& fragmentPath) {
        std::string vertexStr = getFileStr(vertexPath);
        if (vertexStr.empty()) {
            std::cerr << "No vertex shader!" << nl;
            return nullptr;
        }

        std::string fragmentStr = getFileStr(fragmentPath);
        if (fragmentStr.empty()) {
            std::cerr << "No fragment shader!" << nl;
            return nullptr;
        }

        std::shared_ptr <Renderer::ShaderProgram>& newShader = _shaderPrograms.emplace(shaderName, std::make_shared <Renderer::ShaderProgram> (vertexStr, fragmentStr)).first->second;

        if (newShader->isCompiled()) {
            return newShader;
        }

        std::cerr << "Can't load shader programm:\n" << "Vertex: " << vertexPath << "\nFragment: " << fragmentPath << nl;

        return nullptr;
    };

    std::shared_ptr <Renderer::ShaderProgram> getShader(const std::string shaderName) {
        ShaderProgramsMap::const_iterator it = _shaderPrograms.find(shaderName);

        if (it != _shaderPrograms.end()) {
            return it->second;
        }

        std::cerr << "Can't find the shader programm: " << shaderName << nl;

        return nullptr; 
    };

    std::shared_ptr <Renderer::Texture2D> loadTexture(const std::string& texture, const std::string& path, GLenum mode = GL_NEAREST) {
        int channels = 0, width = 0, height = 0;

        stbi_set_flip_vertically_on_load(true);

        unsigned char* pixs = stbi_load(std::string(_path + "/" + path).c_str(), &width, &height, &channels, 0);

        if (!pixs) {
            std::cerr << "Texture Loading Error: " << path << " does not exists" << nl;
            return nullptr;
        }

        std::shared_ptr <Renderer::Texture2D> newTexture = _textures.emplace(texture, std::make_shared <Renderer::Texture2D> (width, height, pixs, channels, mode, GL_CLAMP_TO_EDGE)).first->second;

        stbi_image_free(pixs);
        
        return newTexture;
    };

    std::shared_ptr <Renderer::Texture2D> getTexture(const std::string textureName, bool noWarn = false) {
        TexturesMap::const_iterator it = _textures.find(textureName);

        if (it != _textures.end()) {
            return it->second;
        }

        if (!noWarn) {
            std::cerr << "Can't find the texture: " << textureName << nl;
        }

        return nullptr; 
    };

    std::shared_ptr <Renderer::Sprite> loadSprite(const std::string& texName, const std::string& subtexName, const std::string& shaderName, const unsigned int Width, const unsigned int Height, const float Rotation) {
        auto tex = getTexture(texName);

        if (!tex) {
            std::cerr << "Can't find the texture: " << texName << " for the sprite" << nl;
        }

        auto shader = getShader(shaderName);

        if (!shader) {
            std::cerr << "Can't find the Shader: " << shaderName << " for the sprite" << nl;
        }

        std::shared_ptr <Renderer::Sprite> newSprite = std::make_shared <Renderer::Sprite> (tex, subtexName, shader, glm::vec2(0.f, 0.f), glm::vec2(Width, Height), Rotation);

        return newSprite;
    };

    std::shared_ptr <Renderer::Texture2D> loadTexAtlas(const std::string& name, const std::string& path, const std::initializer_list <std::string>& subtextures, const int& width, const int& height) {
        auto tex = this->loadTexture(std::move(name), std::move(path));
        if (tex) {
            int tWidth = tex->get_size().x, tHeight = tex->get_size().y;
            glm::vec2 offset = glm::vec2(0.f, tHeight);

            for (auto name : subtextures) {
                glm::vec2 lb(0.f, 0.f);
                glm::vec2 rt(0.f, 0.f);

                lb.x = (float)(offset.x / tWidth);
                lb.y = (float)((offset.y - height) / tHeight);
                rt.x = (float)((offset.x + width) / tWidth);
                rt.y = (float)(offset.y / tHeight);

                tex->add_subtex(name, lb, rt);

                offset.x += width;
                if (offset.x >= tWidth) {
                    offset.x = 0;
                    offset.y -= height;
                }
            }
        }
        return tex;
    };

    std::shared_ptr <Renderer::AnimatedSprite> loadAnimSprite(const std::string& texName, const std::string& subtexName, const std::string& shaderName, const unsigned int Width, const unsigned int Height, const float Rotation) {
        auto tex = getTexture(texName);

        if (!tex) {
            std::cerr << "Can't find the texture: " << texName << " for the sprite" << nl;
        }

        auto shader = getShader(shaderName);

        if (!shader) {
            std::cerr << "Can't find the Shader: " << shaderName << " for the sprite" << nl;
        }

        std::shared_ptr <Renderer::AnimatedSprite> newSprite = std::make_shared <Renderer::AnimatedSprite> (tex, subtexName, shader, glm::vec2(0.f, 0.f), glm::vec2(Width, Height), Rotation);

        return newSprite;
    };
};