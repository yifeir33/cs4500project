#include <thread>

#include "server.h"
#include "server_connection.h"


Server::Server(const char *ip, in_port_t port) : NetPort(ip, port), _clients(10), _passed_update(0), _expected_update(0), _new_update(false) {}

Server::~Server() {
    _connections_mutex.lock();
    while(_connections.length() > 0) {
        ServerConnection *sc = dynamic_cast<ServerConnection*>(_connections.pop());
        if(sc) {
            sc->ask_to_finish();
            sc->join();
            delete sc;
        }
    }
    _connections_mutex.unlock();
    while(_clients.length() > 0) {
        delete _clients.pop();
    }
}

void Server::update_and_alert(SockAddrWrapper *saw) {
    if(saw){
        // update
        this->_client_mutex.lock();
        this->_clients.push(saw);
        this->_client_mutex.unlock();
    }
    // clean-up & alert
    this->_clean_up_closed();

    _connections_mutex.lock();
    this->_expected_update = this->_connections.length();
    _connections_mutex.unlock();

    this->_new_update = true;
}

Packet* Server::get_clients(){
    Packet *packet = new Packet();
    packet->type = CLIENT_UPDATE;
    packet->length = 0;

    this->_client_mutex.lock();
    p("Clients:\n");
    for(size_t i = 0; i < _clients.length(); ++i){
        SockAddrWrapper *saw = dynamic_cast<SockAddrWrapper*>(_clients.get(i));
        assert(saw);

        char *saw_c_str = saw->c_str();
        p(saw_c_str).p('\n');
        delete saw_c_str;

        assert(packet->length + saw->addrlen <= DATA_MAX);
        assert(saw->addrlen == sizeof(saw->addr));
        memcpy(packet->value + packet->length, &saw->addr, saw->addrlen);
        packet->length += saw->addrlen;
    }
    this->_client_mutex.unlock();

    if(++this->_passed_update >= _expected_update){
        this->_passed_update = 0;
        this->_new_update = false;
    }
    return packet;
}

void Server::remove_client(SockAddrWrapper* client) {
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
