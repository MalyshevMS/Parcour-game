#pragma once

#include <glad/glad.h>
#include <string>

namespace Renderer {
    class Texture2D {
    private:
        GLenum m_mode;
        GLuint m_ID;
        unsigned int m_width, m_height;
    public:
        Texture2D(const GLuint width, const GLuint height, 
        const unsigned char* data, const unsigned int channels, 
        const GLenum filter, const GLenum wrap_mode) {
            m_width = width;
            m_height = height;

            switch (channels) {
            case 4:
                m_mode = GL_RGBA;
                break;

            case 3:
                m_mode = GL_RGB;
                break;
            
            default:
                break;
            }

            glGenTextures(1, &m_ID);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, m_ID);
            glTexImage2D(GL_TEXTURE_2D, 0, m_mode, m_width, m_height, 0, m_mode, GL_UNSIGNED_BYTE, data);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_mode);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_mode);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);

            glBindTexture(GL_TEXTURE_2D, 0);
        };

        ~Texture2D() {
            glDeleteTextures(1, &m_ID);
        };

        Texture2D() = delete;
        Texture2D(const Texture2D&) = delete;
        Texture2D& operator=(const Texture2D&) = delete;

        Texture2D& operator=(Texture2D&& text2d) {
            glDeleteTextures(1, &m_ID);

            m_ID = text2d.m_ID;
            text2d.m_ID = 0;

            m_mode = text2d.m_mode;
            m_width = text2d.m_width;
            m_height = text2d.m_height;

            return *this;
        };

        Texture2D(Texture2D&& text2d) {
            m_ID = text2d.m_ID;
            text2d.m_ID = 0;

            m_mode = text2d.m_mode;
            m_width = text2d.m_width;
            m_height = text2d.m_height;
        };

        void bind() const {
            glBindTexture(GL_TEXTURE_2D, m_ID);
        };
    };
} // namespace Renderer
