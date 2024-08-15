#include "json.hpp"
#include "ResourceManager.hpp"
#include "SpriteGroup.hpp"
#include "TextureLoader.hpp"
#include <fstream>

using json = nlohmann::json;

class Parser {
private:
    ResourceManager* rm;
    TexLoader* tx;
    SprGroup* sg;
public:
    Parser(ResourceManager* rm = nullptr, TexLoader* tx = nullptr, SprGroup* sg = nullptr) {
        this->rm = rm;
        this->tx = tx;
        this->sg = sg;
    };

    void parse_lvl(string path, string shader, int* spr_size) {
        json file;
        std::ifstream inp;
        inp.open(rm->getExePath() + path);
        inp >> file;

        *spr_size = file["spite.size"];
        
        int tex_count = file["textures"]["count"];
        json tex_arr = file["textures"]["array"];
        for (int i = 0; i < tex_count; i++) {
            string _name = tex_arr[i]["name"];
            string _path = tex_arr[i]["path"];
            tx->add_texture(_name, _path);
        }

        json lvl = file["level"];
        for (int i = 0;; i++) {
            json _elem = lvl[i];
            bool _end = _elem["end"];
            sg->add_sprite(_elem["tex"], shader, _elem["size.x"], _elem["size.y"], _elem["rot"], _elem["x"], _elem["y"]);
            if (_end == true) {
                break;
            }
        }

    };
};
