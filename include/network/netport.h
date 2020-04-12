#pragma once

#include <netinet/ip.h>
#include <chrono>
#include <vector>
#include <mutex>

#include "util/object.h"
#include "network/connection.h"

#define WATCHDOG_TIMEOUT  60 // seconds

class NetPort : public Object {
private:
    std::mutex _fd_mutex;
    int _sock_fd;

    NetPort(const NetPort&) = delete;
    NetPort& operator=(const NetPort&) = delete;
    NetPort(NetPort&&) = delete;

protected:
    static std::mutex instance_lock;
    static std::shared_ptr<NetPort> np_instance;

    sockaddr_in _self;
    std::mutex _connections_mutex;
    std::vector<std::shared_ptr<Connection>> _connections;
    std::atomic<bool> _running;
    std::atomic<std::chrono::time_point<std::chrono::steady_clock>> _watchdog;

    NetPort(const char *ip, in_port_t port);

    virtual ~NetPort();

    void feed_dog();

    bool dog_is_alive() const;

    void close_all();

    bool _accept_connection();

    void _clean_up_closed();

    virtual void _on_clean_up(std::shared_ptr<Connection> c) = 0;

    virtual void _initial() = 0;

    virtual void _main_loop_work() = 0;

    virtual void _on_new_connection() = 0;

    virtual std::shared_ptr<Connection> _new_connection(int new_conn_fd, sockaddr_in other) = 0;

public:
    void listen_on_socket(int conn_count);

    sockaddr_in get_self() const;

    virtual void request_teardown() = 0;

    size_t hash() const override;

    bool equals(const Object *other) const override;

    std::shared_ptr<Object> clone() const override;
};
