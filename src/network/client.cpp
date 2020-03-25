#include <errno.h>
#include <unistd.h>
#include <assert.h>
#include <cstring>

#include "network/client.h"
#include "network/socket_util.h"
#include "network/cts_connection.h"
#include "network/ctc_connection.h"

Client::Client(const char *client_ip, const char *server_ip, in_port_t port)
    : NetPort(client_ip, 0),_server_connection(nullptr), _client_update(false) {
    _server.sin_family = AF_INET;
    if(inet_pton(AF_INET, server_ip, &(_server.sin_addr)) <= 0){
        perror("Error converting address: ");
        exit(1);
    } // convert to proper format
    _server.sin_port = htons(port);
}

Client::~Client(){
    if(_server_connection){
        _server_connection->ask_to_finish();
        _server_connection->join();
    }
}

std::unique_ptr<Packet> Client::get_registration_packet() {
    auto packet = std::make_unique<Packet>();
    packet->type = REGISTER;
    packet->value.resize(sizeof(_self));
    memcpy(packet->value.data(), &_self, sizeof(_self));
    return packet;
}

std::unique_ptr<Connection> Client::_new_connection(int new_conn_fd, sockaddr_in other) {
    return std::make_unique<CtCConnection>(new_conn_fd, other, *this, true);
}

void Client::_initial() {
    sockaddr_in cts_saddr;
    socket_util::clone_sockaddr(cts_saddr, _self);
    cts_saddr.sin_port = 0;

    int sock = socket_util::create_socket(cts_saddr, true);
    if(sock < 0){
        this->_running = false;
        return;
    }

    _server_connection = std::make_unique<CtSConnection>(sock, *this, cts_saddr);
    _server_connection->start();
}

void Client::_main_loop_work() {
    // TODO
}
