#include <random>
#include <time.h>

#include "server.h"
#include "utils.h"
#include "network/packet.h"

#define TICK_INTERVAL 1000/20

using chisel::server::Server;

Server::Server( Config* config, std::vector<std::string>* ops ):
    _salt      (rand_b62_str(16)),
    _config    (config),
    _logger    ("LOG_" + std::to_string(time(NULL)) + ".txt"),
    _threadPool(_config->maxPlayers + 5),
    _operators (ops)
{
    _threadPool.start();
}

Server::~Server() {
    _threadPool.stop();

    delete _config;
}

void Server::start() {
    _socket.listen_port(_config->port);
    _logger.log(LL_INFO, "Listening for clients...");

    _threadPool.queue([this] { 
        while(true) {
            std::this_thread::sleep_for(std::chrono::milliseconds(TICK_INTERVAL));
            this->tick();
        }
    });

    while(true) {
        auto client = _socket.accept_cl();
        _players.push_back(Player(client, &_socket));
    }
}

void Server::tick() {
    for(auto& p : _players) {
        if(p.active == false) {
            // TODO 27/10/22: Removing player from vector.
            continue;
        }

        p.tick(_config, _operators);
    }
}