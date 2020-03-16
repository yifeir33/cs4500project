#pragma once

#include <mutex>
#include <atomic>

#include "netport.h"
#include "packet.h"

class Server : public NetPort {
public:
    std::mutex _client_mutex;
    std::vector<sockaddr_in> _clients;
    std::atomic<size_t> _passed_update;
    std::atomic<size_t> _expected_update;
    std::atomic<bool> _new_update;

    Server(const char *ip, in_port_t port);

    ~Server();

    void server_listen();

    void _cleanup_closed();

    void update_and_alert(sockaddr_in saddr);

    Packet* get_clients();

    void remove_client(sockaddr_in client);

    bool new_client_update();

    void _on_clean_up(Connection *c) override;

    std::unique_ptr<Connection> _new_connection(int new_conn_fd, sockaddr_in other) override;

    friend class ServerConnection;
};
