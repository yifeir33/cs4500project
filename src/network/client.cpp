#include <errno.h>
#include <unistd.h>
#include <assert.h>
#include <cstring>
#include <list>

#include "network/client.h"
#include "network/socket_util.h"
#include "network/cts_connection.h"
#include "network/ctc_connection.h"

// static methods
bool Client::init(const char* client_ip, const char* server_ip, in_port_t server_port) {
    std::unique_lock<std::mutex> lock(instance_lock);
    if(NetPort::np_instance){
        return false;
    }
    NetPort::np_instance = std::shared_ptr<Client>(new Client(client_ip, server_ip, server_port));
    return true;
}

std::weak_ptr<Client> Client::get_instance() {
    std::unique_lock<std::mutex> lock(instance_lock);
    return std::dynamic_pointer_cast<Client>(NetPort::np_instance);
}

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

Packet Client::get_registration_packet() {
    Packet packet;
    packet.type = Packet::Type::REGISTER;
    uint8_t *fptr = reinterpret_cast<uint8_t *>(&_self);
    packet.value.insert(packet.value.end(),
                        fptr,
                        fptr + sizeof(_self));
    return packet;
}

std::shared_ptr<DataFrame> Client::_get_value_helper(std::shared_ptr<CtCConnection> c, const std::string& key) {
    auto request = std::make_shared<CtCConnection::ValueRequest>(key);
    std::unique_lock<std::mutex> lk(request->mutex);
    c->add_request(request);
    request->cv.wait(lk);
    return request->value;
}

std::shared_ptr<DataFrame> Client::get_value(const KVStore::Key& key) {
    std::unique_lock<std::mutex> conn_lock(_connections_mutex);
    for(size_t i = 0; i < _connections.size(); ++i) {
        auto ctc_conn = std::dynamic_pointer_cast<CtCConnection>(_connections[i]);
        assert(ctc_conn);
        if(socket_util::sockaddr_eq(ctc_conn->get_conn_other(), key.get_node())){
            conn_lock.unlock();
            return this->_get_value_helper(ctc_conn, key.get_name());
        }
    }
    pln("Failed to find connection!");
    return nullptr;
}

std::shared_ptr<Connection> Client::_new_connection(int new_conn_fd, sockaddr_in other) {
    return std::make_shared<CtCConnection>(new_conn_fd, other, *this, true);
}

void Client::_initial() {
    sockaddr_in cts_self;
    socket_util::clone_sockaddr(cts_self, _self);
    cts_self.sin_port = 0;

    int sock = socket_util::create_socket(cts_self, true);
    if(sock < 0){
        this->_running = false;
        return;
    }

    _server_connection = std::make_unique<CtSConnection>(sock, *this, _server);
    _server_connection->start();
}


void Client::_check_client_updates() {
    if(!this->_client_update) return;

    std::list<sockaddr_in> to_connect;
    { // new scope to control scoped lock
        std::scoped_lock lock(this->_connections_mutex, this->_oclient_mutex);
        for(auto client : _other_clients){
            bool existing_conn = false;
            for(auto conn : _connections){
                if(socket_util::sockaddr_eq(conn->get_conn_other(), client)){
                    existing_conn = true;
                    break;
                }
            }
            if(!existing_conn) to_connect.push_front(client);
        }
    }
    for(auto c : to_connect){
        this->_connect_to_client(c);
    }
}

void Client::_connect_to_client(sockaddr_in client){
    sockaddr_in new_self = this->_self;
    new_self.sin_port = 0;
    int new_fd = socket_util::create_socket(new_self, true);
    if(new_fd < 0) this->_running = false;

    std::lock_guard<std::mutex> lock(this->_connections_mutex);
    _connections.push_back(std::make_shared<CtCConnection>(new_fd, client, *this, false));
    _connections.back()->start();
}

void Client::_main_loop_work() {
    this->_check_client_updates();
}

void Client::_on_clean_up(std::shared_ptr<Connection> c) {
    // TODO
    assert(c);
}

void Client::_on_new_connection() {
    // TODO
}
