#include "ResourceManager.hpp"
#include "SpriteGroup.hpp"
#include "TextureLoader.hpp"
#include "../Variables/OpenGL.hpp"
#include "json.hpp"


class Parser {
    private:
    using json = nlohmann::json;
    using DPair = std::pair<std::string, uint64_t>;
    ResourceManager* rm = nullptr;
    TexLoader* tx = nullptr;
    SprGroup* sg = nullptr;
    OpenGL* gl = nullptr;
public:
    json get_json(const std::string& path) {
        std::fstream f;
        f.open(rm->getExePath() + path);
        json file;
        f >> file;
        return file;
    }

    Parser(ResourceManager* rm = nullptr, TexLoader* tx = nullptr, SprGroup* sg = nullptr, OpenGL* gl = nullptr) {
        this->rm = rm;
        this->tx = tx;
        this->sg = sg;
        this->gl = gl;
    };

    void parse_conf(const std::string& path) {
        json file = get_json(std::move(path));


    }

    void parse_player(std::string path, SprGroup* sg_p, const glm::vec2& spawn_pos) {
        json file = get_json(std::move(path));

        std::vector <json> atl_arr = file["textures"]["atlas"];
        for (auto i : atl_arr) {
            tx->add_textures_from_atlas(i["name"], i["path"], i["subtexs"], glm::vec2(i["size.x"], i["size.y"]));
        }

        sg_p->add_sprite("Player", "default", gl->sprite_shader, gl->sprite_size, gl->sprite_size, 0.f, spawn_pos.x, spawn_pos.y);

        std::vector <json> anims = file["animations"];
        for (auto i : anims) {
            std::vector <DPair> result;
            std::vector <json> playback = i["playback"];
            for (auto j : playback) {
                result.push_back(DPair(j["tex"], j["duration"]));
            }
            sg_p->add_animation(0, i["name"], result);
        }
    }

    void parse_lvl(std::string path, int* spr_size) {
        json file = get_json(std::move(path));

        *spr_size = file["sprite.size"];
        
        std::vector <json> tex_arr = file["textures"]["array"];
        for (auto i : tex_arr) {
            std::string _name = i["name"];
            std::string _path = i["path"];
            tx->add_texture(_name, _path);
        }

        std::vector <json> atl_arr = file["textures"]["atlas"];
        for (auto i : atl_arr) {
            tx->add_textures_from_atlas(i["name"], i["path"], i["subtexs"], glm::vec2(i["size.x"], i["size.y"]));
        }

        std::vector <json> lvl = file["level"];
        for (int i = 0; i < lvl.size(); i++) {
            json _elem = lvl[i];
            sg->add_sprite(_elem["tex"], "default", "SpriteShader", *spr_size, *spr_size, _elem["rot"], _elem["x"], _elem["y"]);
        }
    };
};