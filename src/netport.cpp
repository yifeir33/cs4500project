#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <algorithm>

#include "netport.h"
#include "socket_util.h"

NetPort::NetPort(const char *ip, in_port_t port) : _sock_fd(0), _connections(10), _running(true), _watchdog(std::chrono::steady_clock::now()){
    if((_sock_fd = socket_util::create_socket(ip, port, _self, true)) < 0){
        assert(false);
    }
    // TODO
    /* char *sock_info = _self.c_str(); */
    /* p("NetPort bound to: ").p(sock_info).p('\n'); */
}

NetPort::~NetPort(){
    this->close_all();
    if(_sock_fd > 0){
        close(_sock_fd);
    }
}

void NetPort::feed_dog(){
    _watchdog = std::chrono::steady_clock::now();
}

bool NetPort::dog_is_alive() const {
    std::chrono::time_point<std::chrono::steady_clock> wd = _watchdog;

    bool out =  std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - wd).count()
                < WATCHDOG_TIMEOUT;
    if(!out){
        p("Watchdog Timed Out!").p('\n');
    }
    return out;
}

void NetPort::close_all(){
    _connections_mutex.lock();
    for(size_t i = 0; i < _connections.size(); ++i){
        _connections[i]->ask_to_finish();
        _connections[i]->join();
    }
    _connections.clear();
    _connections_mutex.unlock();
}

bool NetPort::_accept_connection(){
    int new_connection_fd = 0;
    sockaddr_in saddr;
    socklen_t saddr_size = sizeof(saddr);
    if((new_connection_fd = accept(_sock_fd, reinterpret_cast<sockaddr*>(&saddr), &saddr_size)) < 0){
        if(errno != EAGAIN && errno != EWOULDBLOCK) { // Since _server_fd is non-blocking to allow clean_up of threads
            perror("Failed to Accept Connection");
            this->_running = false;
        }
        return false;
    } else {
        assert(saddr_size == sizeof(sockaddr_in));
        auto c = _new_connection(new_connection_fd, saddr);
        c->start();
        _connections_mutex.lock();
        _connections.push_back(std::move(c));
        _connections_mutex.unlock();
        return true;
    }
}

void NetPort::_clean_up_closed() {
    struct closed_predicate {
        bool operator()(std::unique_ptr<Connection>& c) {
            if(c->is_finished()){
                c->join();
                return true;
            }
            return false;
        }
    };
    auto p = closed_predicate();
    _connections_mutex.lock();
    std::remove_if(_connections.begin(), _connections.end(), p);
    _connections_mutex.unlock();
}

void NetPort::listen_on_socket(int conn_count){
    if(listen(_sock_fd, conn_count) < 0){
        perror("Error listening on socket: ");
        this->_running = false;
    }
    this->_running = true;
    this->_initial();

    while(this->_running && this->dog_is_alive()){
        if(this->_accept_connection()){
            this->_on_new_connection();
            this->feed_dog();
        }
        this->_clean_up_closed();
        this->_main_loop_work();
    }
    p("Exiting").p('\n');
    close(_sock_fd);
    this->_running = false;
}
