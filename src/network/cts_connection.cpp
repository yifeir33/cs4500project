#include "cts_connection.h"

CtSConnection::CtSConnection(int fd, Client& c, sockaddr_in server) : Connection(fd, server), _client(c) {
    p("CTS Created").p('\n');
}

void CtSConnection::run() {
    p("CTS Run").p('\n');
    assert(this->_conn_other);

    this->connect_to_target(this->_conn_other);
    this->register_with_server();

    while(!this->is_finished() && this->dog_is_alive()) {
        if(this->receive_and_parse()) this->feed_dog();
        if(!this->_keep_alive())
            break;

        _client.feed_dog();
        sleep(500);
    }
    this->deregister_and_shutdown();
    this->_finished = true;
}

void CtSConnection::register_with_server() {
    Packet *packet = _client.get_registration_packet();
    if(!this->_send_packet(packet)){
        p("Unable to register!");
        this->_finished = true;
    } else {
        p("Registered with server").p('\n');
    }
    delete packet;
}

void CtSConnection::deregister_and_shutdown(){
    Packet *packet = _client.get_registration_packet();
    packet->type = DEREGISTER;
    if(!this->_send_packet(packet)){
        p("Unable to deregister!").p('\n');
        this->_finished = true;
    } else {
        p("Deregistered!").p('\n');
    }
    delete packet;
    this->_send_shutdown();
}

ParseResult CtSConnection::_parse_data(Packet &packet) {
    if(packet.type == CLIENT_UPDATE){
        p("Client Update!").p('\n');
        // reset other clients
        _client._oclient_mutex.lock();
        while(_client._other_clients.length() > 0){
            delete _client._other_clients.pop();
        }

        size_t pos = 0;
        while(pos + sizeof(sockaddr_in) <= packet.length){
            sockaddr_in saddr;
            memcpy(&saddr, packet.value + pos, sizeof(saddr));
            pos += sizeof(saddr);
            if(!_client._self == saddr){
                _client._other_clients.push(saw);
                char *saw_c_str = saw->c_str();
                p("Client: ").p(saw_c_str).p('\n');
                delete saw_c_str;
            } else {
                delete saw;
            }
        }
        _client._oclient_mutex.unlock();
        return ParseResult::Success;
    }
    return Connection::_parse_data(packet);
}
