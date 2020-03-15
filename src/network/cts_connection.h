#pragma once

#include "connection.h"
#include "client.h"

class CtSConnection : public Connection {
protected:
    Client& _client;

    ParseResult _parse_data(Packet &packet) override;

public:

    CtSConnection(int fd, Client& c, sockaddr_in server);

    void run() override;

    void register_with_server();

    void deregister_and_shutdown();
};
