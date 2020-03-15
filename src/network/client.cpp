#include "client.h"
#include "cts_connection.h"
#include "ctc_connection.h"

Client::Client(const char *client_ip, const char *server_ip, in_port_t port)
    : NetPort(client_ip, 0), _client_update(false) {
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
        delete _server_connection;
    }
}

Packet *Client::get_registration_packet() {
    Packet *packet = new Packet();
    packet->type = REGISTER;
    packet->length = _self.addrlen;
    memcpy(&packet->value, &_self.addr, _self.addrlen);
    return packet;
}

Connection *Client::_new_connection(int new_conn_fd, SockAddrWrapper *other) {
    return new CtCConnection(new_conn_fd, other, *this, true);
}

void Client::_initial() {
    SockAddrWrapper cts_saw(_self);
    cts_saw.addr.sin_port = 0;

    int sock = create_socket(cts_saw, true);
    if(sock < 0){
        this->_running = false;
        return;
    }

    _server_connection = new CtSConnection(sock, *this, new SockAddrWrapper(_server));
    _server_connection->start();
}

void Client::_main_loop_work() {
    // TODO
}
