#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstring>

#include "socket_util.h"
#include "connection.h"

Connection::Connection(int cfd, sockaddr_in c) : _conn_fd(cfd), _conn_other(c),
_finished(false), _r_buf_pos(0), _watchdog(std::chrono::steady_clock::now()) {
    memset(_r_buffer, 0, BUFFER_SIZE);
    /* p("New Connection!\n"); */
    socket_util::make_non_blocking(_conn_fd);
}

Connection::~Connection() {
    if(_conn_fd > 0){
        close(_conn_fd);
    }
}

void Connection::start() {
    _thread = std::thread([this](){this->run();});
}

void Connection::join() {
    _thread.join();
}

void Connection::feed_dog(){
    _watchdog = std::chrono::steady_clock::now();
}

bool Connection::dog_is_alive() const {
    bool out =  std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - _watchdog).count()
                < WATCHDOG_TIMEOUT;
    if(!out){
        this->pln("Watchdog Timed Out!");
    }
    return out;
}

bool Connection::_keep_alive() {
    Packet packet;
    packet.type = KEEP_ALIVE;
    packet.length = 0;
    return this->_send_packet(&packet);
}

void Connection::_send_shutdown() {
    Packet packet;
    packet.type = SHUTDOWN;
    packet.length = 0;
    this->_send_packet(&packet); // regardless we shutdown
    this->_finished = true;
}

bool Connection::_send_packet(Packet *packet) {
    assert(packet);
    uint8_t w_buffer[PACKET_MAX_SIZE];
    int p_size = packet->pack(w_buffer, PACKET_MAX_SIZE);
    if(p_size < 0) return false;
    int attempt = 0;
    ssize_t sent = 0;

    while(attempt < 5){
        if(this->_check_for_socket_errors() > 0){
            if(errno != EWOULDBLOCK && errno != EAGAIN && errno != 0) {
                perror("Socket Errors Found: ");
                p("errno value: ").pln(errno);
                return false;
            }
        }
        if((sent = send(_conn_fd, w_buffer, p_size, 0)) < 0){
            if(errno != EWOULDBLOCK && errno != EAGAIN) {
                perror("Error sending packet: ");
                return false;
            }
        } else if(sent > 0) {
            /* p("Sent Packet!").p('\n'); */
            return true;
        }
        sleep(100); // sleep 100 milliseconds
        ++attempt;
    }
    return false;
}

sockaddr_in Connection::get_conn_other() const {
    return _conn_other;
}

int Connection::_check_for_socket_errors() {
    int opt = 0;
    socklen_t optlen = sizeof(opt);
    if(getsockopt(_conn_fd, SOL_SOCKET, SO_ERROR, &opt, &optlen) < 0){
        if(errno != EWOULDBLOCK && errno != EAGAIN && errno != 0) {
            perror("Error checking socket: ");
            p("Errno Value: ").pln(errno);
            this->_finished = true;
        }
    }
    return opt;
}

int Connection::receive_data() {
    int received = 0;
    int attempt = 0;

    while (attempt < 5) {
        if((received = recv(_conn_fd, _r_buffer + _r_buf_pos, BUFFER_SIZE - _r_buf_pos, 0)) < 0) {
            if(errno != EWOULDBLOCK && errno != EAGAIN) {
                perror("Error receiving data: ");
                return received;
            }
        } else {
            /* p("Received: ").pln(received); */
            _r_buf_pos += received;
            return received;
        }
        ++attempt;
    } // while
    return -1;
}

bool Connection::unpack(Packet& packet){
    int parsed_pos = 0;
    if((parsed_pos = packet.unpack(_r_buffer, BUFFER_SIZE)) < 0){
        return false;
    } else {
        memmove(_r_buffer, _r_buffer + parsed_pos, BUFFER_SIZE - parsed_pos);
        _r_buf_pos -= parsed_pos;
        memset(_r_buffer + _r_buf_pos, 0, BUFFER_SIZE - _r_buf_pos); // 0 out rest of buffer
        return true;
    }
}

ParseResult Connection::_parse_data(Packet& packet){
    /* pln("_parse_data"); */

    switch(packet.type){
        case CLIENT_UPDATE:
            break;
        case REGISTER:
            break;
        case KEEP_ALIVE:
            this->_respond(packet);
            break;
        case ASK_FOR_ID:
            return ParseResult::Response;
        case ID:
            {
                if(packet.length != sizeof(sockaddr_in)) {
                    return ParseResult::ParseError;
                }
                memcpy(&this->_conn_other, packet.value, packet.length);
                break;
            }
        case CHAR_MSG:
        case ERROR_MSG:
            {
                if(packet.type == CHAR_MSG)
                    pln("Message Received:");
                else 
                    pln("Error Received:");

                char msg[DATA_MAX] = {0};
                memcpy(msg, &packet.value, packet.length);
                msg[packet.length] = '\0';
                pln(msg);
                /* p("Sender:\n"); */
                /* char addr_str[INET_ADDRSTRLEN]; */
                /* p(inet_ntop(AF_INET, &(_conn_other->addr.sin_addr), addr_str, INET_ADDRSTRLEN)); */
                /* p(":").p(ntohs((_conn_other->addr.sin_port))); */
                /* p("\n"); */
                break;
            }
        case SHUTDOWN:
            pln("Shutdown Received!");
            this->_finished = true;
            break;
        default:
            p("Unrecognized Type: ").pln(packet.type);
            return ParseResult::ParseError;
    }
    return ParseResult::Success;
}

int Connection::_respond([[maybe_unused]] Packet& msg){ return 0; }

bool Connection::receive_and_parse() {
    if(this->receive_data() < 0) {
        return false;
    }
    /* p("Data Received!").p('\n'); */
    /* this->print_read_buffer(); */

    Packet packet;
    while(_r_buf_pos > 0){
        if(!this->unpack(packet)){
            pln("Failed to unpack data!");
            break;
        }

        switch(this->_parse_data(packet)){
            case Success:
                return true;
            case Response:
                this->_respond(packet);
                break;
            case ParseError:
                    return false;
            default:
                return false;
        }
    }
    return true;
}

void Connection::connect_to_target(sockaddr_in target){
    if(connect(_conn_fd, reinterpret_cast<sockaddr*>(&target), sizeof(target)) < 0) {
        // Needed cause the socket is non-blocking
        if(errno == EINPROGRESS){
            fd_set fdset{0};
            FD_SET(_conn_fd, &fdset);
            if(select(_conn_fd + 1, nullptr, &fdset, nullptr, nullptr) < 0){ 
                perror("Error on select: ");
                this->_finished = true;
                return;
            }
            if(FD_ISSET(_conn_fd, &fdset)){
                if(this->_check_for_socket_errors()){
                    perror("Error in socket: ");
                    this->_finished = true;
                    return;
                }
            }
        } else {
            perror("Failed to connect to target: ");
            this->_finished = true;
            return;
        }
    }
}


void Connection::ask_to_finish(){
    this->_finished = true;
}

bool Connection::is_finished(){
    return _finished;
}

void Connection::print_read_buffer(){
    p("Read Buffer (Length: ").p(_r_buf_pos).pln("): ");
    for(size_t i = 0; i < _r_buf_pos; ++i){
        p(_r_buffer[i]);
        if(i > 0 && i % 8 == 0) {
            pln();
        }else {
            p(' ');
        }
    }
    pln();
}

Object *Connection::clone() const {
    return nullptr;
}

bool Connection::equals(const Object* other) const {
    return this == other;
}

size_t Connection::hash() const {
    return reinterpret_cast<size_t>(this);
}