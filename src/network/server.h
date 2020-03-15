#pragma once

#include <mutex>
#include <atomic>

#include "netport.h"
#include "array.h"
#include "packet.h"
#include "socket_addr.h"

class Server : public NetPort {
public:
    std::mutex _client_mutex;
    ObjectArray _clients;
    std::atomic<size_t> _passed_update;
    std::atomic<size_t> _expected_update;
    std::atomic<bool> _new_update;

    Server(const char *ip, in_port_t port);

    ~Server();

    void server_listen();

    void _cleanup_closed();

    void update_and_alert(SockAddrWrapper *saw);

    Packet* get_clients();

    void remove_client(SockAddrWrapper* client);

    bool new_client_update();

    void _on_clean_up(Connection *c) override;

    Connection* _new_connection(int new_conn_fd, SockAddrWrapper *other) override;
};
