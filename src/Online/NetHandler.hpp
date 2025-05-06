#include <SFML/Network.hpp>
#include <iostream>
#include <string>

using namespace std;

class NetHandler {
private:
    sf::TcpSocket socket = sf::TcpSocket();
    sf::IpAddress ip = sf::IpAddress::LocalHost;
    sf::Packet packet = sf::Packet();
public:
    NetHandler(string ip, unsigned short port, bool srv = false) {
        this->packet = sf::Packet();
        this->ip = sf::IpAddress(*sf::IpAddress::resolve(ip));

        if (srv) {
            sf::TcpListener listener = sf::TcpListener();
            listener.listen(port);

            if (listener.accept(socket) != sf::Socket::Status::Done) {
                std::cerr << "Error!\n";
            }
        } else if (socket.connect(this->ip, port) != sf::Socket::Status::Done) {
            std::cerr << "Error: failed connect to server.\n";
            return;
        }
        std::cout << "Connected!\n";
        socket.setBlocking(true); 
    };

    string recv_msg() {
        if (socket.receive(packet) == sf::Socket::Status::Done) {
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

    ~NetHandler() {};
};