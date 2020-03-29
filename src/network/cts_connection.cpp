#include <cstring>
#include <thread>
#include <chrono>
#include <assert.h>

#include "network/cts_connection.h"
#include "network/socket_util.h"

CtSConnection::CtSConnection(int fd, Client& c, sockaddr_in server) : Connection(fd, server), _client(c) {
    p("CTS Created").p('\n');
}

void CtSConnection::run() {
    p("CTS Run").p('\n');

    this->connect_to_target(this->_conn_other);
    this->register_with_server();

    while(!this->is_finished() && this->dog_is_alive()) {
        if(this->receive_and_parse()) this->feed_dog();
        if(!this->_keep_alive())
            break;

        _client.feed_dog();
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    this->deregister_and_shutdown();
    this->_finished = true;
}

void CtSConnection::register_with_server() {
    Packet packet = _client.get_registration_packet();
    if(!this->_send_packet(packet)){
        p("Unable to register!");
        this->_finished = true;
    } else {
        p("Registered with server").p('\n');
    }
}

void CtSConnection::deregister_and_shutdown(){
    auto packet = _client.get_registration_packet();
    packet.type = Packet::Type::DEREGISTER;
    if(!this->_send_packet(packet)){
        p("Unable to deregister!").p('\n');
        this->_finished = true;
    } else {
        p("Deregistered!").p('\n');
    }
    this->_send_shutdown();
}

ParseResult CtSConnection::_parse_data(Packet &packet) {
    if(packet.type == Packet::Type::CLIENT_UPDATE){
        p("Client Update!").p('\n');
        // reset other clients
        std::lock_guard<std::mutex> client_lock(_client._oclient_mutex);
        _client._other_clients.clear();

        size_t pos = 0;
        while(pos + sizeof(sockaddr_in) <= packet.value.size()){
            sockaddr_in saddr;
            memcpy(&saddr, packet.value.data() + pos, sizeof(saddr));
            pos += sizeof(saddr);
            if(!socket_util::sockaddr_eq(_client._self, saddr)){
                _client._other_clients.push_back(saddr);
            }
        }
        _client._client_update = true;
        return ParseResult::Success;
    }
    return Connection::_parse_data(packet);
}

