#pragma once

#include <glad/glad.h>
#include <string>
#include <map>

namespace Renderer {
    class Texture2D {
    public: struct SubTexture2D;
    private:
        GLenum _mode;
        GLuint _ID;
        unsigned int _width, _height;
        std::map <std::string, SubTexture2D> sub_textures;
    public:
        struct SubTexture2D {
            glm::vec2 LB, RT;
            SubTexture2D(const glm::vec2& lb, const glm::vec2& rt) : LB(lb), RT(rt) {};
            SubTexture2D() : LB(glm::vec2(0.f)), RT(glm::vec2(1.f)) {};
        };

        Texture2D(const GLuint width, const GLuint height, 
        const unsigned char* data, const unsigned int channels, 
        const GLenum filter, const GLenum wrap_mode) {
            _width = width;
            _height = height;

            switch (channels) {
            case 4:
                _mode = GL_RGBA;
                break;

            case 3:
                _mode = GL_RGB;
                break;
            
            default:
                break;
            }

            glGenTextures(1, &_ID);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, _ID);
            glTexImage2D(GL_TEXTURE_2D, 0, _mode, _width, _height, 0, _mode, GL_UNSIGNED_BYTE, data);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_mode);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_mode);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);

            glBindTexture(GL_TEXTURE_2D, 0);
        };

        ~Texture2D() {
            glDeleteTextures(1, &_ID);
        };

        Texture2D() = delete;
        Texture2D(const Texture2D&) = delete;
        Texture2D& operator=(const Texture2D&) = delete;

        Texture2D& operator=(Texture2D&& text2d) {
            glDeleteTextures(1, &_ID);

            _ID = text2d._ID;
            text2d._ID = 0;

            _mode = text2d._mode;
            _width = text2d._width;
            _height = text2d._height;

            return *this;
        };

        Texture2D(Texture2D&& text2d) {
            _ID = text2d._ID;
            text2d._ID = 0;

            _mode = text2d._mode;
            _width = text2d._width;
            _height = text2d._height;
        };

        void bind() const {
            glBindTexture(GL_TEXTURE_2D, _ID);
        };

        glm::vec2 get_size() { return glm::vec2(_width, _height); };

        void add_subtex(const std::string& name, const glm::vec2& lb, const glm::vec2& rt) {
            sub_textures.emplace(name, SubTexture2D(lb, rt));
        };

        const SubTexture2D& get_subtex(const std::string& name) const {
            auto it = sub_textures.find(name);
            if (it != sub_textures.end()) {
                return it->second;
            }
            const static SubTexture2D default_;
            return default_;
        };
    };
} // namespace Renderer
