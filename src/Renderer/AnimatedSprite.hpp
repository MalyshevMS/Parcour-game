#pragma once

#include <glad/glad.h>
#include <glm/vec2.hpp>

#include <memory>
#include <map>
#include <vector>
#include <string>

#include "Sprite.hpp"

namespace Renderer {
    class AnimatedSprite : public Sprite {
    private:
        std::map <std::string, std::vector <std::pair <std::string, uint64_t>>> states_map;
        std::map <std::string, std::vector <std::pair <std::string, uint64_t>>>::const_iterator anim_durations;
        size_t current_frame = 0;
        uint64_t anim_time = 0;
        mutable bool dirty = false;
    public:
        AnimatedSprite(const std::shared_ptr<Texture2D> tex, const std::string& subtex, const std::shared_ptr<ShaderProgram> shader_prog, const glm::vec2& pos, const glm::vec2& size, const float& rotation) : \
        Sprite(std::move(tex), std::move(subtex), std::move(shader_prog), pos, size, rotation) {
            anim_durations = states_map.end();
        };

        void insert_state(const std::string& state,  const std::initializer_list <std::pair <std::string, uint64_t>>& subtex_duration) {
            states_map.emplace(std::move(state), std::vector <std::pair <std::string, uint64_t>>(subtex_duration));
        };

        void render() const override {
            if (dirty) {
                auto subtex = _tex->get_subtex(anim_durations->second[current_frame].first);

                const GLfloat texCoords[] {
                    subtex.LB.x, subtex.LB.y,
                    subtex.LB.x, subtex.RT.y,
                    subtex.RT.x, subtex.RT.y,

                    subtex.RT.x, subtex.RT.y,
                    subtex.RT.x, subtex.LB.y,
                    subtex.LB.x, subtex.LB.y
                };

                glBindBuffer(GL_ARRAY_BUFFER, _texCoordsVBO);
                glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(texCoords), &texCoords);
                glBindBuffer(GL_ARRAY_BUFFER, 0);

                dirty = false;
            }

            Sprite::render();
        };

        void update(const uint64_t& delta) {
            if (anim_durations != states_map.end()) {
                anim_time += delta;

                for (;anim_time >= anim_durations->second[current_frame].second;) { 
                    anim_time -= anim_durations->second[current_frame].second; current_frame++; dirty = true;
                    if (current_frame == anim_durations->second.size()) current_frame = 0;
                }
                
            }
        };

        void set_state(const std::string& new_state) {
            auto it = states_map.find(new_state);
            if (it == states_map.end()) {
                std::cerr << "Can't find the state " << new_state << std::endl; return;
            }

            if (it != anim_durations) {
                anim_time = 0;
                current_frame = 0;
                anim_durations = it;
                dirty = true;
            }
        }
    };
};