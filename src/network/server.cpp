#include <thread>
#include <cstring>

#include "network/server.h"
#include "network/server_connection.h"
#include "network/socket_util.h"

// static methods
bool Server::init(const char* ip, in_port_t port) {
    std::unique_lock<std::mutex> lock(instance_lock);
    if(NetPort::np_instance){
        return false;
    }
    NetPort::np_instance = std::shared_ptr<Server>(new Server(ip, port));
    return true;
}

std::weak_ptr<Server> Server::get_instance() {
    std::unique_lock<std::mutex> lock(instance_lock);
    return std::dynamic_pointer_cast<Server>(NetPort::np_instance);
}

Server::Server(const char *ip, in_port_t port) : NetPort(ip, port), _clients(), _passed_update(0), _expected_update(0), _new_update(false) {}

Server::~Server() {}

void Server::update_and_alert(sockaddr_in saddr) {
    // update
    std::unique_lock<std::mutex> client_lock(_client_mutex);
    this->_clients.push_back(saddr);
    client_lock.unlock();
    // clean-up & alert
    this->_clean_up_closed();

    std::lock_guard<std::mutex> connection_lock(_connections_mutex);
    this->_expected_update = this->_connections.size();

    this->_new_update = true;
}

Packet Server::get_clients(){
    Packet packet;
    packet.type = Packet::Type::CLIENT_UPDATE;
    packet.value.clear();

    std::lock_guard<std::mutex> client_lock(_client_mutex);
    /* p("Clients:\n"); */
    for(size_t i = 0; i < _clients.size(); ++i){
        uint8_t *fptr = reinterpret_cast<uint8_t *>(&_clients[i]);
        packet.value.insert(packet.value.end(),
                            fptr,
                            fptr + sizeof(_clients[i]));
    }

    if(++this->_passed_update >= _expected_update){
        this->_passed_update = 0;
        this->_new_update = false;
    }
    return packet;
}

void Server::remove_client(sockaddr_in client) {
    std::lock_guard<std::mutex> client_lock(_client_mutex);
    auto it = this->_clients.begin();
    while(it != this->_clients.end()) {
        if(socket_util::sockaddr_eq(client, *it)) {
            this->_clients.erase(it);
            break;
        }
        ++it;
    }
    this->_new_update = true;
}

bool Server::new_client_update() {
    return _new_update;
}

void Server::request_teardown() {
    pln("request_teardown");
    std::lock_guard<std::mutex> conn_lock(this->_connections_mutex);
    pln("_connections lock aquired - teardown");
    for(auto conn : _connections){
        conn->send_teardown_request();
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));
    this->_running = false;
    pln("Tear down requested - shutting down");
}

std::shared_ptr<Connection> Server::_new_connection(int new_connection_fd, sockaddr_in other){
    pln("New Server Connection");
    return std::make_shared<ServerConnection>(new_connection_fd, other, *this);
}

void Server::_on_clean_up(std::shared_ptr<Connection> c) {
    this->remove_client(c->get_conn_other());
}

void Server::_initial() {
    // TODO
}

void Server::_on_new_connection() {
    // TODO
}

void Server::_main_loop_work() {
    // TODO
}
