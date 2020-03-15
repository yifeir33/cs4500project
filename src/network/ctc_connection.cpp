#include "ctc_connection.h"

CtSConnection::CtCConnection(int fd, SockAddrWrapper *other, Client& c, bool r) : Connection(fd, other),
_client(c), _receiver(r) {
    p("CTC Created!").p('\n');
}

void CtCConnection::run() {
    if(_receiver){
        this->_as_receiver();
    } else {
        this->_as_client();
    }
    this->_finished = true;
}

void CtCConnection::_as_client() {
    assert(this->_conn_other);
    p("As Client").p('\n');
    this->connect_to_target(this->_conn_other);
    p("Connected to target!").p('\n');
    this->_say_hello();
    while(!this->is_finished() && this->dog_is_alive()){
        if(this->receive_and_parse()) this->feed_dog();
        sleep(100);
    }
}

void CtCConnection::_say_hello(){
    std::string hello("Hello");
    Packet packet;
    packet.type = CHAR_MSG;
    packet.length = hello.size();
    memcpy(&packet.value, hello.c_str(), hello.size());
    if(!this->_send_packet(&packet)){
        this->_finished = true;
    }
}

void CtCConnection::_as_receiver(){
    p("As Receiver").p('\n');
    while(!this->is_finished() && this->dog_is_alive()){
        if(this->receive_and_parse()) {
            this->feed_dog();
            this->_client.feed_dog();
        }
        sleep(100);
    }
    this->receive_and_parse();
}

ParseResult CtCConnection::_parse_data(Packet& packet) {
    if(packet.type == CHAR_MSG){
        ParseResult r = Connection::_parse_data(packet);
        if(r == ParseResult::Success){
            _client._received += 1;
            this->_send_shutdown();
            return r;
        }
    } 
    return Connection::_parse_data(packet);
}

int CtCConnection::_respond(Packet& msg) {
    if(msg.type == ASK_FOR_ID){
        Packet *packet = _client.get_registration_packet();
        packet->type = ID;
        if(!this->_send_packet(packet)){
            p("Failed to respond!").p('\n');
        }
        return 0;
    }
    return -1;
}
