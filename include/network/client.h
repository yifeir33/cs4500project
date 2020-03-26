#pragma once

#include <mutex>
#include <atomic>
#include <arpa/inet.h>
#include <vector>

#include "network/netport.h"
#include "network/packet.h"
#include "data/kvstore.h"

class CtSConnection;
class CtCConnection;

class Client : public NetPort {
private:
    sockaddr_in _server;
    std::unique_ptr<CtSConnection> _server_connection;
    std::mutex _oclient_mutex;
    std::vector<sockaddr_in> _other_clients;
    std::atomic<bool> _client_update; 

    Client(const char *client_ip, const char *server_ip, in_port_t server_port);

    Client(const Client&) = delete;
    Client& operator=(const Client&) = delete;
    Client(Client&&) = delete;

    void _initial() override;

    void _main_loop_work() override;

    void _on_clean_up(std::shared_ptr<Connection> c) override;

    void _on_new_connection() override;

    std::shared_ptr<Connection> _new_connection(int new_conn_fd, sockaddr_in other) override;

    // blocking
    std::shared_ptr<DataFrame> _get_value_helper(std::shared_ptr<CtCConnection> c, const std::string& key);

public:
    static bool init(const char *client_ip, const char *server_ip, in_port_t server_port);
    static std::weak_ptr<Client> get_instance();

    ~Client();

    std::unique_ptr<Packet> get_registration_packet();

    // this is blocking
    std::shared_ptr<DataFrame> get_value(const KVStore::Key& key);

    // friends
    friend class CtSConnection;
    friend class CtCConnection;
};
