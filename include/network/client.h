#pragma once

#include <mutex>
#include <atomic>
#include <arpa/inet.h>
#include <vector>

#include "network/netport.h"
#include "network/packet.h"
#include "data/kvstore.h"
#include "data/distributed_store.h"

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
    std::vector<uint8_t> _operation_request_helper(std::shared_ptr<CtCConnection> c, std::string key, size_t opcode);

    // blocking
    std::shared_ptr<DataFrame> _get_value_helper(std::shared_ptr<CtCConnection> c, const std::string& key);

    void _connect_to_client(sockaddr_in client);

    void _check_client_updates();

public:
    static bool init(const char *client_ip, const char *server_ip, in_port_t server_port);
    static std::weak_ptr<Client> get_instance();

    ~Client();

    Packet get_registration_packet();

    void request_teardown() override;

    std::vector<sockaddr_in> get_full_client_list() const;

    bool push_remote(sockaddr_in node, std::string distributed_key,
                     std::vector<DistributedStore::MetaData> md, std::vector<std::string> keys,
                     std::vector<std::shared_ptr<DataFrame>> dfs);

    // blocking
    std::vector<uint8_t> operation_request(sockaddr_in node, std::string key, size_t opcode);

    // this is blocking
    std::shared_ptr<DataFrame> get_value(const KVStore::Key& key);

    // friends
    friend class CtSConnection;
    friend class CtCConnection;
};
