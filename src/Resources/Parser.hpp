#include <fstream>
#include "json.hpp"

#include "ResourceManager.hpp"
#include "TextureLoader.hpp"
#include "SpriteGroup.hpp"

using json = nlohmann::json;
using namespace std;

class Parser {
private:
    ResourceManager *rm;
    TexLoader* tx;
    SprGroup* sg;
public:
    Parser(ResourceManager* rm = nullptr, TexLoader* tx = nullptr, SprGroup* sg = nullptr) {
        this->rm = rm;
        this->tx = tx;
        this->sg = sg;
    };

    void parse_lvl(string path, int* spr_size, std::shared_ptr<Renderer::ShaderProgram>* _default_shader,
        std::shared_ptr<Renderer::ShaderProgram>* _sprite_shader) {
        json file;
        std::ifstream inp;
        inp.open(rm->getExePath() + path);
        inp >> file;

        *spr_size = file["spite.size"];

        json shaders = file["shaders"];

        // Default shader
        auto default_shader = rm->loadShaders(shaders["default"]["name"], shaders["default"]["path.vert"], shaders["default"]["path.frag"]);
        if (!default_shader) {
            cerr << "Can't create shader program!" << endl;
            exit(-1);
        }
        default_shader->use();
        default_shader->setInt("tex", 0);
        *_default_shader = default_shader;

        // Sprite shader
        auto sprite_shader = rm->loadShaders(shaders["sprite"]["name"], shaders["sprite"]["path.vert"], shaders["sprite"]["path.frag"]);
        if (!sprite_shader) {
            cerr << "Can't create shader program!" << endl;
            exit(-1);
        }
        sprite_shader->use();
        sprite_shader->setInt("tex", 0);
        *_sprite_shader = sprite_shader;

        // Textures
        int tex_count = file["textures"]["count"];
        json tex_arr = file["textures"]["array"];
        for (int i = 0; i < tex_count; i++) {
            string _name = tex_arr[i]["name"];
            string _path = tex_arr[i]["path"];
            tx->add_texture(_name, _path);
        }

        // Level
        json lvl = file["level"];
        for (int i = 0;; i++) {
            json _elem = lvl[i];
            bool _end = _elem["end"];
            sg->add_sprite(_elem["tex"], shaders["sprite"]["name"], _elem["size.x"], _elem["size.y"], _elem["rot"], _elem["x"], _elem["y"]);
            if (_end == true) {
                break;
            }
        }
    };

    void parse_parameters(string path, int* max_speed, int* jump_speed, int* jump_height, int* gravity) {
        json file;
        std::ifstream inp;
        inp.open(rm->getExePath() + path);
        inp >> file;

        *max_speed = file["max_speed"];
        *jump_speed = file["jump_speed"];
        *jump_height = file["jump_height"];
        *gravity = file["gravity"];
    };
};