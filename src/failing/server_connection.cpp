#include "server_connection.h"

ServerConnection::ServerConnection(int cfd, SockAddrWrapper* o, Server& s) : Connection(cfd, o), _server(s) {}

ServerConnection::~ServerConnection(){}

void ServerConnection::run() {
    while(!this->is_finished() && this->dog_is_alive()){
        if(this->receive_and_parse()){
            this->feed_dog();
            this->_server.feed_dog();
        }
        if(_server._new_update){
            p("New Update: Sending Client List").p('\n');
            Packet *packet = _server.get_clients();
            if(!this->_send_packet(packet)){
                delete packet;
                break;
            }
            delete packet;
        }
        sleep(500);
        if(!this->_keep_alive())
            break;
    } // while
    this->receive_and_parse();
    close(_conn_fd);
    _conn_fd = 0;
    this->_finished = true;
}

ParseResult ServerConnection::_parse_data(Packet& packet) {
    /* p("ServerConnection: parse data").p('\n'); */
    if(packet.type == REGISTER || packet.type == DEREGISTER) {
        p("Register/Deregister").p('\n');
        if(packet.length != sizeof(sockaddr_in)) {
            return ParseResult::ParseError;
        }
        SockAddrWrapper *saw = new SockAddrWrapper();
        memcpy(&saw->addr, packet.value, packet.length);

        if(packet.type == REGISTER){
            if(this->_conn_other)
                delete this->_conn_other;
            // use this to track which client this is
            this->_conn_other = new SockAddrWrapper(saw);

            _server.update_and_alert(saw);
        } else {
            _server.remove_client(saw);
            delete saw;
        }
        return ParseResult::Success;
    } else if(packet.type == SHUTDOWN){
        this->ask_to_finish();
        return ParseResult::Success;
    }

    return Connection::_parse_data(packet);
}
