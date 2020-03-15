#pragma once

#include <assert.h>
#include <thread>
#include <mutex>
#include <atomic>
#include <errno.h>
#include <unistd.h>
#include <assert.h>
#include <arpa/inet.h>
#include <vector>

#include "netport.h"
#include "socket_addr.h"
#include "array.h"
#include "packet.h"

class CtSConnection;

class Client : public NetPort {
private:
    sockaddr_in _server;
    CtSConnection *_server_connection;
    std::mutex _oclient_mutex;
    std::vector<sockaddr_in> _other_clients;
    std::atomic<bool> _client_update; 

    void _initial() override;

    void _main_loop_work() override;

public:
    Client(const char *client_ip, const char *server_ip, in_port_t port);

    ~Client();

    Packet *get_registration_packet();

    Connection* _new_connection(int new_conn_fd, SockAddrWrapper *other) override;
};
