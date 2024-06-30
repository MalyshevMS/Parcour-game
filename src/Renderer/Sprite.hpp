#pragma once

#include <glad/glad.h>
#include <memory>
#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Texture2D.hpp"
#include "ShaderProgram.hpp"

namespace Renderer  {
    class Sprite {
    private:
        std::shared_ptr<Texture2D> _tex;
        std::shared_ptr<ShaderProgram> _shader_prog;
        glm::vec2 _pos, _size;
        float _rotation;
        GLuint _VAO;
        GLuint _vertexCoordsVBO;
        GLuint _texCoordsVBO;
    public:
        Sprite(const std::shared_ptr<Texture2D> tex, const std::shared_ptr<ShaderProgram> shader_prog, const glm::vec2& pos, glm::vec2& size, const float rotation) {
            _tex = std::move(tex);
            _shader_prog = std::move(shader_prog);
            _size = size;
            _pos = pos;
            _rotation = rotation;

            const GLfloat vertCoords[] = {
                0, 0,
                0, 1,
                1, 1,

                1, 1,
                1, 0,
                0, 0
            };

            const GLfloat texCoords[] {
                0, 0,
                0, 1,
                1, 1,

                1, 1,
                1, 0,
                0, 0
            };

            glGenVertexArrays(1, &_VAO);
            glBindVertexArray(_VAO);

            glGenBuffers(1, &_vertexCoordsVBO);
            glBindBuffer(GL_ARRAY_BUFFER, _vertexCoordsVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertCoords), &vertCoords, GL_STATIC_DRAW);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

            glGenBuffers(1, &_texCoordsVBO);
            glBindBuffer(GL_ARRAY_BUFFER, _texCoordsVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(texCoords), &texCoords, GL_STATIC_DRAW);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindVertexArray(0);
        };

        ~Sprite() {
            glDeleteBuffers(1, &_vertexCoordsVBO);
            glDeleteBuffers(1, &_texCoordsVBO);

            glDeleteVertexArrays(1, &_VAO);
        };

        Sprite(const Sprite&) = delete;
        Sprite& operator=(const Sprite&) = delete;

        void render() const {
            _shader_prog->use();

            glm::mat4 model(1.f);

            model = glm::translate(model, glm::vec3(_pos, 0.f));
            model = glm::translate(model, glm::vec3(0.5f * _size.x, 0.5f * _size.y, 0.f));
            model = glm::rotate(model, glm::radians(_rotation), glm::vec3(0.f, 0.f, 1.f));
            model = glm::translate(model, glm::vec3(-0.5f * _size.x, -0.5f * _size.y, 0.f));
            model = glm::scale(model, glm::vec3(_size, 1.f));

            glBindVertexArray(_VAO);
            _shader_prog->setMat4("modelMat", model);

            glActiveTexture(GL_TEXTURE0);
            _tex->bind();

            glDrawArrays(GL_TRIANGLES, 0 ,6);
            glBindVertexArray(0);
        };

        void move(glm::vec2& xy) {
            setPos(_pos + xy);
        };

        void setPos(const glm::vec2& pos) {
            _pos = pos;
        };

        glm::vec2 getPos() { return _pos; };

        void setSize(glm::vec2& size) {
            _size = size;
        };

        glm::vec2 getSize() { return _size; };

        void setRotation(const float rotation) {
            _rotation = rotation;
        };

        float getRotation() { return _rotation; };
    };
}