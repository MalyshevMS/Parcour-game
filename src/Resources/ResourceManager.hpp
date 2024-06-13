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

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#include "stb_image.hpp"

#define string std::string
#define nl std::endl

class ResourceManager {
private:
    typedef std::map <const string, std::shared_ptr <Renderer::ShaderProgram>> ShaderProgramsMap;
    typedef std::map <const string, std::shared_ptr<Renderer::Texture2D>> TexturesMap;
    typedef std::map <const string, std::shared_ptr<Renderer::Sprite>> SpritesMap;

    ShaderProgramsMap _shaderPrograms;
    TexturesMap _textures;
    SpritesMap _sprites;

    string _path;

    string getFileStr(const string& path) const {
        std::ifstream f;
        f.open(_path + path, std::ios::in | std::ios::binary);
        if (!f.is_open()) {
            std::cerr << "Failed to open file " << _path + path << nl;
            return string();
        }

        std::stringstream buffer;
        buffer << f.rdbuf();

        return buffer.str();
    };
public:
    ResourceManager(const string& exePath = "") {
        size_t found = exePath.find_last_of("/\\");
        this->_path = exePath.substr(0, found + 1);
    };

    ~ResourceManager() = default;

    std::shared_ptr <Renderer::ShaderProgram> loadShaders(const string& shaderName, const string& vertexPath, const string& fragmentPath) {
        string vertexStr = getFileStr(vertexPath);
        if (vertexStr.empty()) {
            std::cerr << "No vertex shader!" << nl;
            return nullptr;
        }

        string fragmentStr = getFileStr(fragmentPath);
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

    std::shared_ptr <Renderer::ShaderProgram> getShader(const string shaderName) {
        ShaderProgramsMap::const_iterator it = _shaderPrograms.find(shaderName);

        if (it != _shaderPrograms.end()) {
            return it->second;
        }

        std::cerr << "Can't find the shader programm: " << shaderName << nl;

        return nullptr; 
    };

     std::shared_ptr <Renderer::Texture2D> loadTexture(const string& texture, const string& path, GLenum mode) {
        int channels = 0, width = 0, height = 0;

        stbi_set_flip_vertically_on_load(true);

        unsigned char* pixs = stbi_load(string(_path + "/" + path).c_str(), &width, &height, &channels, 0);

        if (!pixs) {
            std::cerr << "Texture Loading Error: " << path << " does not exists" << nl;
            return nullptr;
        }

        std::shared_ptr <Renderer::Texture2D> newTexture = _textures.emplace(texture, std::make_shared <Renderer::Texture2D> (width, height, pixs, channels, mode, GL_CLAMP_TO_EDGE)).first->second;

        return newTexture;

        stbi_image_free(pixs);
    };

    std::shared_ptr <Renderer::Texture2D> getTexture(const string textureName, bool noWarn = false) {
        TexturesMap::const_iterator it = _textures.find(textureName);

        if (it != _textures.end()) {
            return it->second;
        }

        if (!noWarn) {
            std::cerr << "Can't find the texture: " << textureName << nl;
        }

        return nullptr; 
    };

    std::shared_ptr <Renderer::Sprite> loadSprite(const string spriteName, const string& texName, const string& shaderName, const unsigned int Width, const unsigned int Height, const float Rotation) {
        auto tex = getTexture(texName);

        if (!tex) {
            std::cerr << "Can't find the texture: " << texName << " for the sprite" << nl;
        }

        auto shader = getShader(shaderName);

        if (!shader) {
            std::cerr << "Can't find the Shader: " << shaderName << " for the sprite" << nl;
        }

        std::shared_ptr <Renderer::Sprite> newSprite = _sprites.emplace(spriteName, std::make_shared <Renderer::Sprite> (tex, shader, glm::vec2(0.f, 0.f), glm::vec2(Width, Height), Rotation)).first->second;

        return newSprite;
    };

    /*std::shared_ptr <Renderer::Sprite> getSprite(const string spriteName) {
        SpritesMap::const_iterator it = _sprites.find(spriteName);

        if (it != _sprites.end()) {
            return it->second;
        }

        std::cerr << "Can't find the sprite: " << spriteName << nl;

        return nullptr; 
    };*/
};