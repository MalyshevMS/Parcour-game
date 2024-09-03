#include <SFML/Network.hpp>
#include <iostream>
#include <string>

using namespace std;

class Client {
private:
    sf::TcpSocket socket;
    sf::IpAddress ip;
    sf::Packet packet;
public:
    Client(string ip, unsigned short port) {
        this->ip = sf::IpAddress(ip);
        if (socket.connect(this->ip, port) != sf::Socket::Done) {
            std::cout << "Error: failed connect to server.\n";
            exit(1);
        }
        std::cout << "Connected!\n";
        socket.setBlocking(false);
    };

    string recv_msg() {
        if (socket.receive(packet) == sf::Socket::Done) {
            std::string msg;
            packet >> msg;
            return msg;
        }
        return string();
    };

    void send_msg(const string& msg) {
        packet.clear();
        packet << msg;
        socket.send(packet);
    };
};