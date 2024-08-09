#include "ResourceManager.hpp"
#include "SpriteGroup.hpp"
#include "TextureLoader.hpp"
#include "json.hpp"

using JSON = nlohmann::json;

class Parser {
private:
    ResourceManager* rm;
    TexLoader* tx;
public:
    Parser(ResourceManager* rm, TexLoader* tx) {
        this->rm = rm;
        this->tx = tx;
    };

    void parse_lvl(string path) {
        std::fstream f;
        f.open(rm->getExePath() + path);
        JSON file;
        f >> file;
        auto tex_array = file["textures"]["array"];
        for (int i = 0; i < file["textures"]["count"]; i++) {
            tx->add_texture(tex_array[i]["name"], tex_array[i]["path"]);
        }
    };
};