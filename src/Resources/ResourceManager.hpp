#pragma once

#include <string>
#include <memory>
#include <map>
#include <sstream>
#include <fstream>
#include <iostream>

#include "../Renderer/ShaderProgram.hpp"

class ResourceManager {
private:
    typedef std::map<const std::string, std::shared_ptr <Renderer::ShaderProgram>> ShaderProgramsMap;
    ShaderProgramsMap m_shaderPrograms;

    std::string m_path;

    std::string getFileStr(const std::string& path) const {
        std::ifstream f;
        f.open(m_path + path, std::ios::in | std::ios::binary);
        if (!f.is_open()) {
            std::cerr << "Failed to open file " << m_path + path << std::endl;
            return std::string();
        }

        std::stringstream buffer;
        buffer << f.rdbuf();

        return buffer.str();
    };
public:
    ResourceManager(const std::string& exePath) {
        size_t found = exePath.find_last_of("/\\");
        this->m_path = exePath.substr(0, found + 1);
    };

    ~ResourceManager() = default;
    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&&) = delete;
    ResourceManager(ResourceManager&&) = delete;

    std::shared_ptr <Renderer::ShaderProgram> loadShaders(const std::string& shaderName, const std::string& vertexPath, const std::string& fragmentPath) {
        std::string vertexStr = getFileStr(vertexPath);
        if (vertexStr.empty()) {
            std::cerr << "No vertex shader!" << std::endl;
            return nullptr;
        }

        std::string fragmentStr = getFileStr(fragmentPath);
        if (fragmentStr.empty()) {
            std::cerr << "No fragment shader!" << std::endl;
            return nullptr;
        }

        std::shared_ptr <Renderer::ShaderProgram>& newShader = m_shaderPrograms.emplace(shaderName, std::make_shared <Renderer::ShaderProgram> (vertexStr, fragmentStr)).first->second;

        if (newShader->isCompiled()) {
            return newShader;
        }

        std::cerr << "Can't load shader programm:\n" << "Vertex: " << vertexPath << "\nFragment: " << fragmentPath << std::endl;

        return nullptr;
    };

    std::shared_ptr<Renderer::ShaderProgram> getShader(const std::string shaderName) {
        ShaderProgramsMap::const_iterator it = m_shaderPrograms.find(shaderName);

        if (it != m_shaderPrograms.end()) {
            return it->second;
        }

        std::cerr << "Can't find the shader programm: " << shaderName << std::endl;

        return nullptr; 
    };
};