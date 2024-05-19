#pragma once

#include <glad/glad.h>
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

        bool m_isCompiled = false;
        GLuint m_ID = 0;
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

            m_ID = glCreateProgram();
            glAttachShader(m_ID, vsID);
            glAttachShader(m_ID, fsID);
            glLinkProgram(m_ID);

            GLint success;
            glGetProgramiv(m_ID, GL_LINK_STATUS, &success);

            if (!success) {
                GLchar infolog[1024];

                glGetShaderInfoLog(m_ID, 1024, nullptr, infolog);

                std::cerr << "ERROR::SHADER: LINK error:\n" << infolog << std::endl;
            } else {
                m_isCompiled = true;
            }

            glDeleteShader(vsID);
            glDeleteShader(fsID);
        };

        ~ShaderProgram() {
            glDeleteProgram(m_ID);
        };

        bool isCompiled() const {
            return m_isCompiled;
        };

        void use() const {
            glUseProgram(m_ID);
        };

        ShaderProgram() = delete;
        ShaderProgram(ShaderProgram&) = delete;
        ShaderProgram& operator=(const ShaderProgram&) = delete;

        ShaderProgram& operator=(ShaderProgram&& Shader_Program) noexcept {
            glDeleteProgram(m_ID);
            m_ID = Shader_Program.m_ID;
            m_isCompiled = Shader_Program.m_isCompiled;

            Shader_Program.m_ID = 0;
            Shader_Program.m_isCompiled = false;

            return *this;
        };

        ShaderProgram(ShaderProgram&& Shader_Program) noexcept {
            m_ID = Shader_Program.m_ID;
            m_isCompiled = Shader_Program.m_isCompiled;

            Shader_Program.m_ID = 0;
            Shader_Program.m_isCompiled = false;
        };
    };
} // namespace Renderer

