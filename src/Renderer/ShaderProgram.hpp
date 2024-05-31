#pragma once

#include <glad/glad.h>
#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <iostream>

namespace Renderer {
    class ShaderProgram {
    private:
        bool createShader(const std::string& source, const GLenum shaderType, GLuint& shaderID) {
            shaderID = glCreateShader(shaderType);
            const char* code = source.c_str();
            glShaderSource(shaderID, 1, &code, nullptr);
            glCompileShader(shaderID);

            GLint success;
            glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success);

            if (!success) {
                GLchar infolog[1024];

                glGetShaderInfoLog(shaderID, 1024, nullptr, infolog);

                std::cerr << "ERROR::SHADER: Compile-time error:\n" << infolog << std::endl;
                return false;
            }
            return true;
        };

        bool _isCompiled = false;
        GLuint _ID = 0;
    public:
        ShaderProgram(const std::string& vertexShader, const std::string& fragmentShader) {
            GLuint vsID;
            if(!createShader(vertexShader, GL_VERTEX_SHADER, vsID)) {
                return;
            }

            GLuint fsID;
            if(!createShader(fragmentShader, GL_FRAGMENT_SHADER, fsID)) {
                glDeleteShader(vsID);
                return;
            }

            _ID = glCreateProgram();
            glAttachShader(_ID, vsID);
            glAttachShader(_ID, fsID);
            glLinkProgram(_ID);

            GLint success;
            glGetProgramiv(_ID, GL_LINK_STATUS, &success);

            if (!success) {
                GLchar infolog[1024];

                glGetShaderInfoLog(_ID, 1024, nullptr, infolog);

                std::cerr << "ERROR::SHADER: LINK error:\n" << infolog << std::endl;
            } else {
                _isCompiled = true;
            }

            glDeleteShader(vsID);
            glDeleteShader(fsID);
        };

        ~ShaderProgram() {
            glDeleteProgram(_ID);
        };

        bool isCompiled() const {
            return _isCompiled;
        };

        void use() const {
            glUseProgram(_ID);
        };

        void setInt(const std::string& name, const GLint value) {
            glUniform1i(glGetUniformLocation(_ID, name.c_str()), value);
        };

        void setMat4(const std::string& name, const glm::mat4& matrix) {
            glUniformMatrix4fv(glGetUniformLocation(_ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(matrix));
        };

        ShaderProgram() = delete;
        ShaderProgram(ShaderProgram&) = delete;
        ShaderProgram& operator=(const ShaderProgram&) = delete;

        ShaderProgram& operator=(ShaderProgram&& Shader_Program) noexcept {
            glDeleteProgram(_ID);
            _ID = Shader_Program._ID;
            _isCompiled = Shader_Program._isCompiled;

            Shader_Program._ID = 0;
            Shader_Program._isCompiled = false;

            return *this;
        };

        ShaderProgram(ShaderProgram&& Shader_Program) noexcept {
            _ID = Shader_Program._ID;
            _isCompiled = Shader_Program._isCompiled;

            Shader_Program._ID = 0;
            Shader_Program._isCompiled = false;
        };
    };
} // namespace Renderer

