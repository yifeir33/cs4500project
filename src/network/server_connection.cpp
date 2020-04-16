#include <unistd.h>
#include <cstring>

#include "network/server_connection.h"

ServerConnection::ServerConnection(int cfd, sockaddr_in caddr, Server& s) : Connection(cfd, caddr), _server(s) {}

void ServerConnection::run() {
    pln("ServerConnection Started!");
    while(!this->is_finished() && this->dog_is_alive()){
        if(this->receive_and_parse()){
            this->feed_dog();
            this->_server.feed_dog();
        }
        if(_server._new_update){
            p("New Update: Sending Client List").p('\n');
            Packet packet = _server.get_clients();
            if(!this->_send_packet(packet)){
                break;
            }
        }
        /* std::this_thread::sleep_for(std::chrono::milliseconds(500)); */
        if(!this->_keep_alive()) break;
        std::this_thread::yield();
    } // while
    this->receive_and_parse();
    pln("ServerConnection Finished");
    this->_finished = true;
}

ParseResult ServerConnection::_parse_data(Packet& packet) {
    /* p("ServerConnection: parse data").p('\n'); */
    if(packet.type == Packet::Type::REGISTER || packet.type == Packet::Type::DEREGISTER) {
        p("Register/Deregister").p('\n');
        if(packet.value.size() != sizeof(sockaddr_in)) {
            return ParseResult::ParseError;
        }
        sockaddr_in saddr{};
        memcpy(&saddr, packet.value.data(), packet.value.size());

        if(packet.type == Packet::Type::REGISTER){
            // use this to track which client this is
            this->_conn_other = saddr;
            _server.update_and_alert(saddr);
        } else {
            _server.remove_client(saddr);
        }
        return ParseResult::Success;
    } else if(packet.type == Packet::Type::SHUTDOWN){
        this->ask_to_finish();
        return ParseResult::Success;
    } else if(packet.type == Packet::Type::TEARDOWN){
        pln("Teardown request received");
        _server.request_teardown();
        this->ask_to_finish();
        return ParseResult::Success;
    }

    return Connection::_parse_data(packet);
}
