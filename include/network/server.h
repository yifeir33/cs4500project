#pragma once

#include <mutex>
#include <atomic>

#include "network/netport.h"
#include "network/packet.h"

// singleton pattern -> also enforce that no client
// can exist either
class Server : public NetPort {
protected:
    std::mutex _client_mutex;
    std::vector<sockaddr_in> _clients;
    std::atomic<size_t> _passed_update;
    std::atomic<size_t> _expected_update;
    std::atomic<bool> _new_update;

    Server(const char *ip, in_port_t port);

    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;
    Server(Server&&) = delete;

    void _initial() override;

    void _main_loop_work() override;

    void _on_new_connection() override;

    void _on_clean_up(std::shared_ptr<Connection> c) override;

    std::shared_ptr<Connection> _new_connection(int new_conn_fd, sockaddr_in other) override;

public:
    static bool init(const char *ip, in_port_t port);
    static std::weak_ptr<Server> get_instance();

    ~Server();

    void update_and_alert(sockaddr_in saddr);

    std::unique_ptr<Packet> get_clients();

    void remove_client(sockaddr_in client);

    bool new_client_update();

    friend class ServerConnection;
};
