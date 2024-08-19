#include "ResourceManager.hpp"
#include "SpriteGroup.hpp"
#include "TextureLoader.hpp"
#include "json.hpp"

using json = nlohmann::json;

class Parser {
private:
    ResourceManager* rm = nullptr;
    TexLoader* tx = nullptr;
    SprGroup* sg = nullptr;
public:
    Parser(ResourceManager* rm = nullptr, TexLoader* tx = nullptr, SprGroup* sg = nullptr) {
        this->rm = rm;
        this->tx = tx;
        this->sg = sg;
    };

    void parse_lvl(std::string path) {
        std::fstream f;
        f.open(rm->getExePath() + path);
        json file;
        f >> file;
        
        int tex_count = file["textures"]["count"];
        json tex_arr = file["textures"]["array"];
        for (int i = 0; i < tex_count; i++) {
            std::string _name = tex_arr[i]["name"];
            std::string _path = tex_arr[i]["path"];
            tx->add_texture(_name, _path);
        }

        json lvl = file["level"];
        for (int i = 0;; i++) {
            json _elem = lvl[i];
            bool _end = _elem["end"];
            sg->add_sprite(_elem["tex"], "SpriteShader", _elem["size.x"], _elem["size.y"], _elem["rot"], _elem["x"], _elem["y"]);
            if (_end == true) {
                break;
            }
        }
    };
};