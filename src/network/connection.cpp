#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstring>
#include <string>

#include "network/socket_util.h"
#include "network/connection.h"

Connection::Connection(int cfd, sockaddr_in c) : _fd_mutex(), _conn_fd(cfd), _conn_other(c),
_finished(false), _r_buf_pos(0), _watchdog(std::chrono::steady_clock::now()) {
    memset(_r_buffer, 0, BUFFER_SIZE);
    /* p("New Connection!\n"); */
    std::lock_guard<std::mutex> fd_lock(_fd_mutex);
    socket_util::make_non_blocking(_conn_fd);
}

Connection::~Connection() {
    if(_conn_fd > 0){
        std::lock_guard<std::mutex> fd_lock(_fd_mutex);
        close(_conn_fd);
        _conn_fd = -1;
    }
}

void Connection::start() {
    _thread = std::thread(&Connection::run, this);
}

void Connection::join() {
    if(_thread.joinable()){
        _thread.join();
    } else {
        pln("Bad Join Call!");
    }
}

void Connection::feed_dog(){
    _watchdog = std::chrono::steady_clock::now();
}

bool Connection::dog_is_alive() const {
    return true; // turn off dog
    bool out =  std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - _watchdog).count()
                < WATCHDOG_TIMEOUT;
    if(!out){
        this->pln("Watchdog Timed Out! (Connection)");
    }
    return out;
}

bool Connection::_keep_alive() {
    Packet packet;
    packet.type = Packet::Type::KEEP_ALIVE;
    packet.value.clear();
    return this->_send_packet(packet);
}

void Connection::_send_shutdown() {
    pln("_send_shutdown");
    Packet packet;
    packet.type = Packet::Type::SHUTDOWN;
    packet.value.clear();
    this->_send_packet(packet); // regardless we shutdown
    this->_finished = true;
}

bool Connection::_send_packet(Packet& packet) {
    std::vector packed = packet.pack();
    int attempt = 0;
    ssize_t sent = 0;

    std::unique_lock<std::mutex> fd_lock(_fd_mutex);
    while(attempt < 5){
        fd_lock.unlock();
        if(this->_check_for_socket_errors() > 0){
            if(errno != EWOULDBLOCK && errno != EAGAIN && errno != 0) {
                perror("Socket Errors Found: ");
                p("errno value: ").pln(errno);
                return false;
            }
        }
        fd_lock.lock();

        if((sent = send(_conn_fd, packed.data(), packed.size(), 0)) < 0){
            if(errno != EWOULDBLOCK && errno != EAGAIN) {
                perror("Error sending packet: ");
                return false;
            } else {
                // try to clear send buffer
                std::this_thread::yield();
            }
        } else if(sent > 0) {
            this->feed_dog();
            return true;
        }
        std::this_thread::yield();
        /* sleep(100); // sleep 100 milliseconds */
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
    std::lock_guard<std::mutex> fd_lock(_fd_mutex);
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
    std::lock_guard<std::mutex> fd_lock(_fd_mutex);

    while (attempt < 5) {
        if((received = recv(_conn_fd, _r_buffer + _r_buf_pos, BUFFER_SIZE - _r_buf_pos, 0)) < 0) {
            if(errno != EWOULDBLOCK && errno != EAGAIN) {
                perror("Error receiving data: ");
                return received;
            }
            std::this_thread::yield();
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
    bool finished = false;
    parsed_pos = packet.partial_unpack(true, _r_buffer, BUFFER_SIZE, finished);

    while(!finished && parsed_pos > 0){
        if(parsed_pos >= BUFFER_SIZE){
            memset(_r_buffer, 0, BUFFER_SIZE);
            _r_buf_pos = 0;
        } else {
            memmove(_r_buffer, _r_buffer + parsed_pos, BUFFER_SIZE - parsed_pos);
            _r_buf_pos -= parsed_pos;
            memset(_r_buffer + _r_buf_pos, 0, BUFFER_SIZE - _r_buf_pos); // 0 out rest of buffer
        }
        if(this->receive_data()) this->feed_dog();
        parsed_pos = packet.partial_unpack(false, _r_buffer, BUFFER_SIZE, finished);
    }

    if(parsed_pos >= BUFFER_SIZE){
        memset(_r_buffer, 0, BUFFER_SIZE);
        _r_buf_pos = 0;
    } else {
        memmove(_r_buffer, _r_buffer + parsed_pos, BUFFER_SIZE - parsed_pos);
        _r_buf_pos -= parsed_pos;
        memset(_r_buffer + _r_buf_pos, 0, BUFFER_SIZE - _r_buf_pos); // 0 out rest of buffer
    }
    return finished;
}

ParseResult Connection::_parse_data(Packet& packet){
    /* pln("_parse_data"); */

    switch(packet.type){
        case Packet::Type::CLIENT_UPDATE:
            break;
        case Packet::Type::REGISTER:
            break;
        case Packet::Type::KEEP_ALIVE:
            this->_respond(packet);
            break;
        case Packet::Type::ASK_FOR_ID:
            return ParseResult::Response;
        case Packet::Type::ID:
            {
                if(packet.value.size() != sizeof(sockaddr_in)) {
                    return ParseResult::ParseError;
                }
                memcpy(&this->_conn_other, packet.value.data(), packet.value.size());
                break;
            }
        case Packet::Type::CHAR_MSG:
        case Packet::Type::ERROR_MSG:
            {
                if(packet.type == Packet::Type::CHAR_MSG)
                    pln("Message Received:");
                else 
                    pln("Error Received:");

                std::string msg;
                for(size_t i = 0; i < packet.value.size(); ++i) {
                    msg += packet.value[i];
                }
                pln(msg);
                break;
            }
        case Packet::Type::SHUTDOWN:
        case Packet::Type::TEARDOWN:
            pln("Shutdown/Teardown Received!");
            this->ask_to_finish();
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
    std::unique_lock<std::mutex> fd_lock(_fd_mutex);

    if(connect(_conn_fd, reinterpret_cast<sockaddr*>(&target), sizeof(target)) < 0) {
        // Needed cause the socket is non-blocking
        if(errno == EINPROGRESS || errno == EAGAIN){
            std::this_thread::yield();
            fd_set fdset{0};
            FD_SET(_conn_fd, &fdset);
            if(select(_conn_fd + 1, nullptr, &fdset, nullptr, nullptr) < 0){ 
                perror("Error on select: ");
                this->_finished = true;
                return;
            }
            if(FD_ISSET(_conn_fd, &fdset)){
                fd_lock.unlock();
                if(this->_check_for_socket_errors() != 0){
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
    pln("Connected - function returning");
}


void Connection::ask_to_finish(){
    pln("ask_to_finish");
    this->_finished = true;
}

void Connection::send_teardown_request() {
    pln("send_teardown_request");
    Packet packet;
    packet.type = Packet::Type::TEARDOWN;
    packet.value.clear();
    if(!this->_send_packet(packet)){
        pln("Failed to send teardown");
        this->_finished = true;
    }
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

std::shared_ptr<Object> Connection::clone() const {
    return nullptr;
}

bool Connection::equals(const Object* other) const {
    return this == dynamic_cast<const Connection *>(other);
}

size_t Connection::hash() const {
    return reinterpret_cast<size_t>(this);
}
