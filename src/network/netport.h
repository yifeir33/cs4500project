#pragma once

#include <netinet/ip.h>
#include <chrono>
#include <vector>

#include "../common/object.h"
#include "connection.h"

#define WATCHDOG_TIMEOUT  60 // seconds

class NetPort : public Object {
protected:
    int _sock_fd;
    sockaddr_in _self;
    std::mutex _connections_mutex;
    std::vector<sockaddr_in> _connections;
    std::atomic<bool> _running;
    std::atomic<std::chrono::time_point<std::chrono::steady_clock>> _watchdog;

    NetPort(const char *ip, in_port_t port);

    virtual ~NetPort();

    void feed_dog();

    bool dog_is_alive() const;

    void close_all();

    bool _accept_connection();

    void _clean_up_closed();

    virtual void _on_clean_up(Connection *c) = 0;

    virtual void _initial() = 0;

    virtual void _main_loop_work() = 0;

    virtual void _on_new_connection() = 0;

    virtual Connection *_new_connection(int new_conn_fd, SockAddrWrapper* other) = 0;

public:

    void listen_on_socket(int conn_count);
};
