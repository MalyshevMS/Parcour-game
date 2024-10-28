#pragma once

#include "../Variables/Player.hpp"
#include "../Variables/Client.hpp"
#include <string>
#include <iostream>

class Console {
private:
    Client* client;
public:
    Console(Client* client) {
        this->client = client;
    };

    void player(Player* p, const std::string& command, const std::string& args) {
        if (command == "move") {
            p->x = stoi(args.substr(0, args.find(",")));
            p->y = stoi(args.substr(args.find(",") + 1, args.rfind(",")));
            p->current_anim = args.substr(args.rfind(",") + 1);
        }

        if (command == "block") {
            p->cur.x = stoi(args.substr(0, args.find(",")));
            p->cur.y = stoi(args.substr(args.find(",") + 1));
            client->placed_block = true;
        }
    };

    void echo(const std::string& msg) {
        std::cout << msg << std::endl;
    };

    void action(std::string msg, Player* p) {
        switch (msg[0]) {
            case '+': {
                msg.erase(msg.begin());
                auto com = msg.substr(0, msg.find(":"));
                auto args = msg.substr(msg.find(":") + 1);
                
                this->player(p, com, args);
            } break;

            case '!': {
                msg.erase(msg.begin());
                this->echo(msg);
            } break;
        }
    };
};