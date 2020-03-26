#include <errno.h>
#include <unistd.h>
#include <assert.h>
#include <cstring>

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

std::unique_ptr<Packet> Client::get_registration_packet() {
    auto packet = std::make_unique<Packet>();
    packet->type = Packet::Type::REGISTER;
    packet->value.resize(sizeof(_self));
    memcpy(packet->value.data(), &_self, sizeof(_self));
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
    std::unique_lock<std::mutex> oclient_lock(_oclient_mutex);
    sockaddr_in client = _other_clients[key.get_node()];
    oclient_lock.unlock();

    std::unique_lock<std::mutex> conn_lock(_connections_mutex);
    for(size_t i = 0; i < _connections.size(); ++i) {
        auto ctc_conn = std::dynamic_pointer_cast<CtCConnection>(_connections[i]);
        assert(ctc_conn);
        if(socket_util::sockaddr_eq(ctc_conn->get_conn_other(), client)){
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

void Client::_on_clean_up(std::shared_ptr<Connection> c) {
    // TODO
    assert(c);
}

void Client::_on_new_connection() {
    // TODO
}
