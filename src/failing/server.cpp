#include <thread>
#include <cstring>

#include "server.h"
#include "server_connection.h"
#include "socket_util.h"


Server::Server(const char *ip, in_port_t port) : NetPort(ip, port), _clients(10), _passed_update(0), _expected_update(0), _new_update(false) {}

Server::~Server() {
    _connections_mutex.lock();
    for(size_t i = 0; i < _connections.size(); ++i) {
        _connections[i]->ask_to_finish();
        _connections[i]->join();
    }
    _connections.clear();
    _connections_mutex.unlock();
}

void Server::update_and_alert(sockaddr_in saddr) {
    // update
    this->_client_mutex.lock();
    this->_clients.push_back(saddr);
    this->_client_mutex.unlock();
    // clean-up & alert
    this->_clean_up_closed();

    _connections_mutex.lock();
    this->_expected_update = this->_connections.size();
    _connections_mutex.unlock();

    this->_new_update = true;
}

Packet* Server::get_clients(){
    Packet *packet = new Packet();
    packet->type = CLIENT_UPDATE;
    packet->length = 0;

    this->_client_mutex.lock();
    p("Clients:\n");
    for(size_t i = 0; i < _clients.size(); ++i){
        assert(packet->length + sizeof(_clients[i])  <= DATA_MAX);
        memcpy(packet->value + packet->length, &_clients[i], sizeof(_clients[i]));
        packet->length += sizeof(_clients[i]);
    }
    this->_client_mutex.unlock();

    if(++this->_passed_update >= _expected_update){
        this->_passed_update = 0;
        this->_new_update = false;
    }
    return packet;
}

void Server::remove_client(sockaddr_in client) {
    this->_client_mutex.lock();
    int index = this->_clients.index_of(client);
    if(index >= 0){
        this->_clients.remove(index);
    }
    this->_client_mutex.unlock();
    this->_new_update = true;
}

bool Server::new_client_update() {
    return _new_update;
}

Connection *Server::_new_connection(int new_connection_fd, SockAddrWrapper *other){
    return new ServerConnection(new_connection_fd, other, *this);
}

void Server::_on_clean_up(Connection *c) {
    SockAddrWrapper *osaw = c->get_conn_other();
    this->remove_client(osaw);
    delete osaw;
}
