#include <cstring>
#include <assert.h>

#include "network/ctc_connection.h"

CtCConnection::CtCConnection(int fd, sockaddr_in other, Client& c, bool r) : Connection(fd, other),
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
    p("As Client").p('\n');
    this->connect_to_target(this->_conn_other);
    p("Connected to target!").p('\n');
    // TODO
    while(!this->is_finished() && this->dog_is_alive()){
        if(this->receive_and_parse()) this->feed_dog();
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

void CtCConnection::_as_receiver(){
    p("As Receiver").p('\n');
    while(!this->is_finished() && this->dog_is_alive()){
        if(this->receive_and_parse()) {
            this->feed_dog();
            this->_client.feed_dog();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    this->receive_and_parse();
}

ParseResult CtCConnection::_parse_data(Packet& packet) {
    // TODO
    return Connection::_parse_data(packet);
}

int CtCConnection::_respond(Packet& msg) {
    if(msg.type == ASK_FOR_ID){
        auto packet = _client.get_registration_packet();
        packet->type = ID;
        if(!this->_send_packet(*packet)){
            p("Failed to respond!").p('\n');
        }
        return 0;
    }
    return -1;
}

size_t CtCConnection::hash() const {
    return reinterpret_cast<uintptr_t>(this);
}
